/* BGP-4 Finite State Machine   
   From RFC1771 [A Border Gateway Protocol 4 (BGP-4)]
   Copyright (C) 1996, 97, 98 Kunihiro Ishiguro

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#include <zebra.h>

#include "linklist.h"
#include "prefix.h"
#include "vty.h"
#include "sockunion.h"
#include "thread.h"
#include "log.h"
#include "stream.h"
#include "memory.h"
#include "plist.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_debug.h"
#include "bgpd/bgp_fsm.h"
#include "bgpd/bgp_packet.h"
#include "bgpd/bgp_network.h"
#include "bgpd/bgp_route.h"
#include "bgpd/bgp_dump.h"
#include "bgpd/bgp_open.h"
#ifdef HAVE_SNMP
#include "bgpd/bgp_snmp.h"
#endif /* HAVE_SNMP */

/* BGP FSM (finite state machine) has three types of functions.  Type
   one is thread functions.  Type two is event functions.  Type three
   is FSM functions.  Timer functions are set by bgp_timer_set
   function. */

/* BGP event function. */
int bgp_event (struct thread *);

/* BGP thread functions. */
static int bgp_start_timer (struct thread *);
static int bgp_connect_timer (struct thread *);
static int bgp_holdtime_timer (struct thread *);
static int bgp_keepalive_timer (struct thread *);

/* BGP FSM functions. */
static int bgp_start (struct peer *);

/* BGP start timer jitter. */
int
bgp_start_jitter (int time)
{
  return ((rand () % (time + 1)) - (time / 2));
}

/* Hook function called after bgp event is occered.  And vty's
   neighbor command invoke this function after making neighbor
   structure. */
void
bgp_timer_set (struct peer *peer)
{
  int jitter = 0;

  switch (peer->status)
    {
    case Idle:
      /* First entry point of peer's finite state machine.  In Idle
	 status start timer is on unless peer is shutdown or peer is
	 inactive.  All other timer must be turned off */
      if (CHECK_FLAG (peer->flags, PEER_FLAG_SHUTDOWN)
	  || CHECK_FLAG (peer->sflags, PEER_STATUS_PREFIX_OVERFLOW)
	  || ! peer_active (peer))
	{
	  BGP_TIMER_OFF (peer->t_start);
	}
      else
	{
	  jitter = bgp_start_jitter (peer->v_start);
	  BGP_TIMER_ON (peer->t_start, bgp_start_timer,
			peer->v_start + jitter);
	}
      BGP_TIMER_OFF (peer->t_connect);
      BGP_TIMER_OFF (peer->t_holdtime);
      BGP_TIMER_OFF (peer->t_keepalive);
      BGP_TIMER_OFF (peer->t_asorig);
      BGP_TIMER_OFF (peer->t_routeadv);
      break;

    case Connect:
      /* After start timer is expired, the peer moves to Connnect
         status.  Make sure start timer is off and connect timer is
         on. */
      BGP_TIMER_OFF (peer->t_start);
      BGP_TIMER_ON (peer->t_connect, bgp_connect_timer, peer->v_connect);
      BGP_TIMER_OFF (peer->t_holdtime);
      BGP_TIMER_OFF (peer->t_keepalive);
      BGP_TIMER_OFF (peer->t_asorig);
      BGP_TIMER_OFF (peer->t_routeadv);
      break;

    case Active:
      /* Active is waiting connection from remote peer.  And if
         connect timer is expired, change status to Connect. */
      BGP_TIMER_OFF (peer->t_start);
      /* If peer is passive mode, do not set connect timer. */
      if (CHECK_FLAG (peer->flags, PEER_FLAG_PASSIVE))
	{
	  BGP_TIMER_OFF (peer->t_connect);
	}
      else
	{
	  BGP_TIMER_ON (peer->t_connect, bgp_connect_timer, peer->v_connect);
	}
      BGP_TIMER_OFF (peer->t_holdtime);
      BGP_TIMER_OFF (peer->t_keepalive);
      BGP_TIMER_OFF (peer->t_asorig);
      BGP_TIMER_OFF (peer->t_routeadv);
      break;

    case OpenSent:
      /* OpenSent status. */
      BGP_TIMER_OFF (peer->t_start);
      BGP_TIMER_OFF (peer->t_connect);
      if (peer->v_holdtime != 0)
	{
	  BGP_TIMER_ON (peer->t_holdtime, bgp_holdtime_timer, 
			peer->v_holdtime);
	}
      else
	{
	  BGP_TIMER_OFF (peer->t_holdtime);
	}
      BGP_TIMER_OFF (peer->t_keepalive);
      BGP_TIMER_OFF (peer->t_asorig);
      BGP_TIMER_OFF (peer->t_routeadv);
      break;

    case OpenConfirm:
      /* OpenConfirm status. */
      BGP_TIMER_OFF (peer->t_start);
      BGP_TIMER_OFF (peer->t_connect);

      /* If the negotiated Hold Time value is zero, then the Hold Time
         timer and KeepAlive timers are not started. */
      if (peer->v_holdtime == 0)
	{
	  BGP_TIMER_OFF (peer->t_holdtime);
	  BGP_TIMER_OFF (peer->t_keepalive);
	}
      else
	{
	  BGP_TIMER_ON (peer->t_holdtime, bgp_holdtime_timer,
			peer->v_holdtime);
	  BGP_TIMER_ON (peer->t_keepalive, bgp_keepalive_timer, 
			peer->v_keepalive);
	}
      BGP_TIMER_OFF (peer->t_asorig);
      BGP_TIMER_OFF (peer->t_routeadv);
      break;

    case Established:
      /* In Established status start and connect timer is turned
         off. */
      BGP_TIMER_OFF (peer->t_start);
      BGP_TIMER_OFF (peer->t_connect);

      /* Same as OpenConfirm, if holdtime is zero then both holdtime
         and keepalive must be turned off. */
      if (peer->v_holdtime == 0)
	{
	  BGP_TIMER_OFF (peer->t_holdtime);
	  BGP_TIMER_OFF (peer->t_keepalive);
	}
      else
	{
	  BGP_TIMER_ON (peer->t_holdtime, bgp_holdtime_timer,
			peer->v_holdtime);
	  BGP_TIMER_ON (peer->t_keepalive, bgp_keepalive_timer,
			peer->v_keepalive);
	}
      BGP_TIMER_OFF (peer->t_asorig);
      break;
    }
}

/* BGP start timer.  This function set BGP_Start event to thread value
   and process event. */
static int
bgp_start_timer (struct thread *thread)
{
  struct peer *peer;

  peer = THREAD_ARG (thread);
  peer->t_start = NULL;

  if (BGP_DEBUG (fsm, FSM))
    zlog (peer->log, LOG_DEBUG,
	  "%s [FSM] Timer (start timer expire).", peer->host);

  THREAD_VAL (thread) = BGP_Start;
  bgp_event (thread);

  return 0;
}

/* BGP connect retry timer. */
static int
bgp_connect_timer (struct thread *thread)
{
  struct peer *peer;

  peer = THREAD_ARG (thread);
  peer->t_connect = NULL;

  if (BGP_DEBUG (fsm, FSM))
    zlog (peer->log, LOG_DEBUG, "%s [FSM] Timer (connect timer expire)",
	  peer->host);

  THREAD_VAL (thread) = ConnectRetry_timer_expired;
  bgp_event (thread);

  return 0;
}

/* BGP holdtime timer. */
static int
bgp_holdtime_timer (struct thread *thread)
{
  struct peer *peer;

  peer = THREAD_ARG (thread);
  peer->t_holdtime = NULL;

  if (BGP_DEBUG (fsm, FSM))
    zlog (peer->log, LOG_DEBUG,
	  "%s [FSM] Timer (holdtime timer expire)",
	  peer->host);

  THREAD_VAL (thread) = Hold_Timer_expired;
  bgp_event (thread);

  return 0;
}

/* BGP keepalive fire ! */
static int
bgp_keepalive_timer (struct thread *thread)
{
  struct peer *peer;

  peer = THREAD_ARG (thread);
  peer->t_keepalive = NULL;

  if (BGP_DEBUG (fsm, FSM))
    zlog (peer->log, LOG_DEBUG,
	  "%s [FSM] Timer (keepalive timer expire)",
	  peer->host);

  THREAD_VAL (thread) = KeepAlive_timer_expired;
  bgp_event (thread);

  return 0;
}

int
bgp_routeadv_timer (struct thread *thread)
{
  struct peer *peer;

  peer = THREAD_ARG (thread);
  peer->t_routeadv = NULL;

  if (BGP_DEBUG (fsm, FSM))
    zlog (peer->log, LOG_DEBUG,
	  "%s [FSM] Timer (routeadv timer expire)",
	  peer->host);

  peer->synctime = time (NULL);

  BGP_WRITE_ON (peer->t_write, bgp_write, peer->fd);

  BGP_TIMER_ON (peer->t_routeadv, bgp_routeadv_timer,
		peer->v_routeadv);

  return 0;
}

/* Reset bgp update timer */
static void
bgp_uptime_reset (struct peer *peer)
{
  peer->uptime = time (NULL);
}

/* BGP Peer Down Cause */
char *peer_down_str[] =
{
  "",
  "Router ID changed",
  "Remote AS changed",
  "Local AS change",
  "Cluster ID changed",
  "Confederation identifier changed",
  "Confederation peer changed",
  "RR client config change",
  "RS client config change",
  "Update source change",
  "Address family activated",
  "Admin. shutdown",
  "User reset",
  "BGP Notification received",
  "BGP Notification send",
  "Peer closed the session",
  "Neighbor deleted",
  "Peer-group add member",
  "Peer-group delete member",
  "Capability changed",
  "Passive config change",
  "Multihop config change"
};

/* Administrative BGP peer stop event. */
int
bgp_stop (struct peer *peer)
{
  int established = 0;
  afi_t afi;
  safi_t safi;
  char orf_name[BUFSIZ];

  /* Increment Dropped count. */
  if (peer->status == Established)
    {
      established = 1;
      peer->dropped++;
      bgp_fsm_change_status (peer, Idle);

      /* bgp log-neighbor-changes of neighbor Down */
      if (bgp_flag_check (peer->bgp, BGP_FLAG_LOG_NEIGHBOR_CHANGES))
	zlog_info ("%%ADJCHANGE: neighbor %s Down %s", peer->host,
		   peer_down_str [(int) peer->last_reset]);

      /* set last reset time */
      peer->resettime = time (NULL);

#ifdef HAVE_SNMP
      bgpTrapBackwardTransition (peer);
#endif /* HAVE_SNMP */
    }

  /* Reset uptime. */
  bgp_uptime_reset (peer);

  /* Need of clear of peer. */
  if (established)
    bgp_clear_route_all (peer);

  /* Stop read and write threads when exists. */
  BGP_READ_OFF (peer->t_read);
  BGP_WRITE_OFF (peer->t_write);

  /* Stop all timers. */
  BGP_TIMER_OFF (peer->t_start);
  BGP_TIMER_OFF (peer->t_connect);
  BGP_TIMER_OFF (peer->t_holdtime);
  BGP_TIMER_OFF (peer->t_keepalive);
  BGP_TIMER_OFF (peer->t_asorig);
  BGP_TIMER_OFF (peer->t_routeadv);

  /* Delete all existing events of the peer. */
  BGP_EVENT_DELETE (peer);

  /* Stream reset. */
  peer->packet_size = 0;

  /* Clear input and output buffer.  */
  if (peer->ibuf)
    stream_reset (peer->ibuf);
  if (peer->work)
    stream_reset (peer->work);
  stream_fifo_clean (peer->obuf);

  /* Close of file descriptor. */
  if (peer->fd >= 0)
    {
      close (peer->fd);
      peer->fd = -1;
    }

  /* Connection information. */
  if (peer->su_local)
    {
      XFREE (MTYPE_SOCKUNION, peer->su_local);
      peer->su_local = NULL;
    }

  if (peer->su_remote)
    {
      XFREE (MTYPE_SOCKUNION, peer->su_remote);
      peer->su_remote = NULL;
    }

  /* Clear remote router-id. */
  peer->remote_id.s_addr = 0;

  /* Reset all negotiated variables */
  peer->afc_nego[AFI_IP][SAFI_UNICAST] = 0;
  peer->afc_nego[AFI_IP][SAFI_MULTICAST] = 0;
  peer->afc_nego[AFI_IP][SAFI_MPLS_VPN] = 0;
  peer->afc_nego[AFI_IP6][SAFI_UNICAST] = 0;
  peer->afc_nego[AFI_IP6][SAFI_MULTICAST] = 0;
  peer->afc_adv[AFI_IP][SAFI_UNICAST] = 0;
  peer->afc_adv[AFI_IP][SAFI_MULTICAST] = 0;
  peer->afc_adv[AFI_IP][SAFI_MPLS_VPN] = 0;
  peer->afc_adv[AFI_IP6][SAFI_UNICAST] = 0;
  peer->afc_adv[AFI_IP6][SAFI_MULTICAST] = 0;
  peer->afc_recv[AFI_IP][SAFI_UNICAST] = 0;
  peer->afc_recv[AFI_IP][SAFI_MULTICAST] = 0;
  peer->afc_recv[AFI_IP][SAFI_MPLS_VPN] = 0;
  peer->afc_recv[AFI_IP6][SAFI_UNICAST] = 0;
  peer->afc_recv[AFI_IP6][SAFI_MULTICAST] = 0;

  /* Reset route refresh flag. */
  UNSET_FLAG (peer->cap, PEER_CAP_REFRESH_ADV);
  UNSET_FLAG (peer->cap, PEER_CAP_REFRESH_OLD_RCV);
  UNSET_FLAG (peer->cap, PEER_CAP_REFRESH_NEW_RCV);
  UNSET_FLAG (peer->cap, PEER_CAP_DYNAMIC_ADV);
  UNSET_FLAG (peer->cap, PEER_CAP_DYNAMIC_RCV);

  for (afi = AFI_IP ; afi < AFI_MAX ; afi++)
    for (safi = SAFI_UNICAST ; safi < SAFI_MAX ; safi++)
      {
	/* peer address family capability flags*/
	peer->af_cap[afi][safi] = 0;
	/* peer address family status flags*/
	peer->af_sflags[afi][safi] = 0;
	/* Received ORF prefix-filter */
	peer->orf_plist[afi][safi] = NULL;
        /* ORF received prefix-filter pnt */
        sprintf (orf_name, "%s.%d.%d", peer->host, afi, safi);
        prefix_bgp_orf_remove_all (orf_name);
      }

  /* Reset keepalive and holdtime */
  if (CHECK_FLAG (peer->config, PEER_CONFIG_TIMER))
    {
      peer->v_keepalive = peer->keepalive;
      peer->v_holdtime = peer->holdtime;
    }
  else
    {
      peer->v_keepalive = peer->bgp->default_keepalive;
      peer->v_holdtime = peer->bgp->default_holdtime;
    }

  peer->update_time = 0;

  /* Until we are sure that there is no problem about prefix count
     this should be commented out.*/
#if 0
  /* Reset prefix count */
  peer->pcount[AFI_IP][SAFI_UNICAST] = 0;
  peer->pcount[AFI_IP][SAFI_MULTICAST] = 0;
  peer->pcount[AFI_IP][SAFI_MPLS_VPN] = 0;
  peer->pcount[AFI_IP6][SAFI_UNICAST] = 0;
  peer->pcount[AFI_IP6][SAFI_MULTICAST] = 0;
#endif /* 0 */

  return 0;
}

/* BGP peer is stoped by the error. */
int
bgp_stop_with_error (struct peer *peer)
{
  /* Double start timer. */
  peer->v_start *= 2;

  /* Overflow check. */
  if (peer->v_start >= (60 * 2))
    peer->v_start = (60 * 2);

  bgp_stop (peer);

  return 0;
}

/* TCP connection open.  Next we send open message to remote peer. And
   add read thread for reading open message. */
int
bgp_connect_success (struct peer *peer)
{
  if (peer->fd < 0)
    {
      zlog_err ("bgp_connect_success peer's fd is negative value %d",
		peer->fd);
      return -1;
    }
  BGP_READ_ON (peer->t_read, bgp_read, peer->fd);

  /* bgp_getsockname (peer); */

  if (! CHECK_FLAG (peer->sflags, PEER_STATUS_ACCEPT_PEER))
    bgp_open_send (peer);

  return 0;
}

/* TCP connect fail */
int
bgp_connect_fail (struct peer *peer)
{
  bgp_stop (peer);
  return 0;
}

/* This function is the first starting point of all BGP connection. It
   try to connect to remote peer with non-blocking IO. */
int
bgp_start (struct peer *peer)
{
  int status;

  /* If the peer is passive mode, force to move to Active mode. */
  if (CHECK_FLAG (peer->flags, PEER_FLAG_PASSIVE))
    {
      BGP_EVENT_ADD (peer, TCP_connection_open_failed);
      return 0;
    }

  status = bgp_connect (peer);

  switch (status)
    {
    case connect_error:
      if (BGP_DEBUG (fsm, FSM))
	plog_info (peer->log, "%s [FSM] Connect error", peer->host);
      BGP_EVENT_ADD (peer, TCP_connection_open_failed);
      break;
    case connect_success:
      if (BGP_DEBUG (fsm, FSM))
	plog_info (peer->log, "%s [FSM] Connect immediately success",
		   peer->host);
      BGP_EVENT_ADD (peer, TCP_connection_open);
      break;
    case connect_in_progress:
      /* To check nonblocking connect, we wait until socket is
         readable or writable. */
      if (BGP_DEBUG (fsm, FSM))
	plog_info (peer->log, "%s [FSM] Non blocking connect waiting result",
		   peer->host);
      if (peer->fd < 0)
	{
	  zlog_err ("bgp_start peer's fd is negative value %d",
		    peer->fd);
	  return -1;
	}
      BGP_READ_ON (peer->t_read, bgp_read, peer->fd);
      BGP_WRITE_ON (peer->t_write, bgp_write, peer->fd);
      break;
    }
  return 0;
}

/* Connect retry timer is expired when the peer status is Connect. */
int
bgp_reconnect (struct peer *peer)
{
  bgp_stop (peer);
  bgp_start (peer);
  return 0;
}

int
bgp_fsm_open (struct peer *peer)
{
  /* Send keepalive and make keepalive timer */
  bgp_keepalive_send (peer);

  /* Reset holdtimer value. */
  BGP_TIMER_OFF (peer->t_holdtime);

  return 0;
}

/* Called after event occured, this function change status and reset
   read/write and timer thread. */
void
bgp_fsm_change_status (struct peer *peer, int status)
{
  bgp_dump_state (peer, peer->status, status);

  /* Preserve old status and change into new status. */
  peer->ostatus = peer->status;
  peer->status = status;
}

/* Keepalive send to peer. */
int
bgp_fsm_keepalive_expire (struct peer *peer)
{
  bgp_keepalive_send (peer);
  return 0;
}

/* Hold timer expire.  This is error of BGP connection. So cut the
   peer and change to Idle status. */
int
bgp_fsm_holdtime_expire (struct peer *peer)
{
  if (BGP_DEBUG (fsm, FSM))
    zlog (peer->log, LOG_DEBUG, "%s [FSM] Hold timer expire", peer->host);

  /* Send notify to remote peer. */
  bgp_notify_send (peer, BGP_NOTIFY_HOLD_ERR, 0);

  /* Sweep if it is temporary peer. */
  if (CHECK_FLAG (peer->sflags, PEER_STATUS_ACCEPT_PEER))
    {
      zlog_info ("%s [Event] Accepting BGP peer is deleted", peer->host);
      peer_delete (peer);
      return -1;
    }

  return 0;
}

/* Status goes to Established.  Send keepalive packet then make first
   update information. */
int
bgp_establish (struct peer *peer)
{
  struct bgp_notify *notify;
  afi_t afi;
  safi_t safi;

  /* Reset capability open status flag. */
  if (! CHECK_FLAG (peer->sflags, PEER_STATUS_CAPABILITY_OPEN))
    SET_FLAG (peer->sflags, PEER_STATUS_CAPABILITY_OPEN);

  /* Clear last notification data. */
  notify = &peer->notify;
  if (notify->data)
    XFREE (MTYPE_TMP, notify->data);
  memset (notify, 0, sizeof (struct bgp_notify));

  /* Clear start timer value to default. */
  peer->v_start = BGP_INIT_START_TIMER;

  /* Increment established count. */
  peer->established++;
  bgp_fsm_change_status (peer, Established);

  /* bgp log-neighbor-changes of neighbor Up */
  if (bgp_flag_check (peer->bgp, BGP_FLAG_LOG_NEIGHBOR_CHANGES))
    zlog_info ("%%ADJCHANGE: neighbor %s Up", peer->host);

#ifdef HAVE_SNMP
  bgpTrapEstablished (peer);
#endif /* HAVE_SNMP */

  /* Reset uptime, send keepalive, send current table. */
  bgp_uptime_reset (peer);

  /* Send route-refresh when ORF is enabled */
  for (afi = AFI_IP ; afi < AFI_MAX ; afi++)
    for (safi = SAFI_UNICAST ; safi < SAFI_MAX ; safi++)
      if (CHECK_FLAG (peer->af_cap[afi][safi], PEER_CAP_ORF_PREFIX_SM_ADV))
	{
	  if (CHECK_FLAG (peer->af_cap[afi][safi], PEER_CAP_ORF_PREFIX_RM_RCV))
	    bgp_route_refresh_send (peer, afi, safi, ORF_TYPE_PREFIX,
				    REFRESH_IMMEDIATE, 0);
	  else if (CHECK_FLAG (peer->af_cap[afi][safi], PEER_CAP_ORF_PREFIX_RM_OLD_RCV))
	    bgp_route_refresh_send (peer, afi, safi, ORF_TYPE_PREFIX_OLD,
				    REFRESH_IMMEDIATE, 0);
	}

  if (peer->v_keepalive)
    bgp_keepalive_send (peer);

  /* First update is deferred until ORF or ROUTE-REFRESH is received */
  for (afi = AFI_IP ; afi < AFI_MAX ; afi++)
    for (safi = SAFI_UNICAST ; safi < SAFI_MAX ; safi++)
      if (CHECK_FLAG (peer->af_cap[afi][safi], PEER_CAP_ORF_PREFIX_RM_ADV))
	if (CHECK_FLAG (peer->af_cap[afi][safi], PEER_CAP_ORF_PREFIX_SM_RCV)
	    || CHECK_FLAG (peer->af_cap[afi][safi], PEER_CAP_ORF_PREFIX_SM_OLD_RCV))
	  SET_FLAG (peer->af_sflags[afi][safi], PEER_STATUS_ORF_WAIT_REFRESH);

  bgp_announce_route_all (peer);

  BGP_TIMER_ON (peer->t_routeadv, bgp_routeadv_timer, 1);

  return 0;
}

/* Keepalive packet is received. */
int
bgp_fsm_keepalive (struct peer *peer)
{
  /* peer count update */
  peer->keepalive_in++;

  BGP_TIMER_OFF (peer->t_holdtime);
  return 0;
}

/* Update packet is received. */
int
bgp_fsm_update (struct peer *peer)
{
  BGP_TIMER_OFF (peer->t_holdtime);
  return 0;
}

/* This is empty event. */
int
bgp_ignore (struct peer *peer)
{
  if (BGP_DEBUG (fsm, FSM))
    zlog (peer->log, LOG_DEBUG, "%s [FSM] bgp_ignore called", peer->host);
  return 0;
}

/* Finite State Machine structure */
struct {
  int (*func) ();
  int next_state;
} FSM [BGP_STATUS_MAX - 1][BGP_EVENTS_MAX - 1] = 
{
  {
    /* Idle state: In Idle state, all events other than BGP_Start is
       ignored.  With BGP_Start event, finite state machine calls
       bgp_start(). */
    {bgp_start,  Connect},	/* BGP_Start                    */
    {bgp_stop,   Idle},		/* BGP_Stop                     */
    {bgp_stop,   Idle},		/* TCP_connection_open          */
    {bgp_stop,   Idle},		/* TCP_connection_closed        */
    {bgp_ignore, Idle},		/* TCP_connection_open_failed   */
    {bgp_stop,   Idle},		/* TCP_fatal_error              */
    {bgp_ignore, Idle},		/* ConnectRetry_timer_expired   */
    {bgp_ignore, Idle},		/* Hold_Timer_expired           */
    {bgp_ignore, Idle},		/* KeepAlive_timer_expired      */
    {bgp_ignore, Idle},		/* Receive_OPEN_message         */
    {bgp_ignore, Idle},		/* Receive_KEEPALIVE_message    */
    {bgp_ignore, Idle},		/* Receive_UPDATE_message       */
    {bgp_ignore, Idle},		/* Receive_NOTIFICATION_message */
  },
  {
    /* Connect */
    {bgp_ignore,  Connect},	/* BGP_Start                    */
    {bgp_stop,    Idle},	/* BGP_Stop                     */
    {bgp_connect_success, OpenSent}, /* TCP_connection_open          */
    {bgp_stop, Idle},		/* TCP_connection_closed        */
    {bgp_connect_fail, Active}, /* TCP_connection_open_failed   */
    {bgp_connect_fail, Idle},	/* TCP_fatal_error              */
    {bgp_reconnect, Connect},	/* ConnectRetry_timer_expired   */
    {bgp_ignore,  Idle},	/* Hold_Timer_expired           */
    {bgp_ignore,  Idle},	/* KeepAlive_timer_expired      */
    {bgp_ignore,  Idle},	/* Receive_OPEN_message         */
    {bgp_ignore,  Idle},	/* Receive_KEEPALIVE_message    */
    {bgp_ignore,  Idle},	/* Receive_UPDATE_message       */
    {bgp_stop,    Idle},	/* Receive_NOTIFICATION_message */
  },
  {
    /* Active, */
    {bgp_ignore,  Active},	/* BGP_Start                    */
    {bgp_stop,    Idle},	/* BGP_Stop                     */
    {bgp_connect_success, OpenSent}, /* TCP_connection_open          */
    {bgp_stop,    Idle},	/* TCP_connection_closed        */
    {bgp_ignore,  Active},	/* TCP_connection_open_failed   */
    {bgp_ignore,  Idle},	/* TCP_fatal_error              */
    {bgp_start,   Connect},	/* ConnectRetry_timer_expired   */
    {bgp_ignore,  Idle},	/* Hold_Timer_expired           */
    {bgp_ignore,  Idle},	/* KeepAlive_timer_expired      */
    {bgp_ignore,  Idle},	/* Receive_OPEN_message         */
    {bgp_ignore,  Idle},	/* Receive_KEEPALIVE_message    */
    {bgp_ignore,  Idle},	/* Receive_UPDATE_message       */
    {bgp_stop_with_error, Idle}, /* Receive_NOTIFICATION_message */
  },
  {
    /* OpenSent, */
    {bgp_ignore,  OpenSent},	/* BGP_Start                    */
    {bgp_stop,    Idle},	/* BGP_Stop                     */
    {bgp_stop,    Idle},	/* TCP_connection_open          */
    {bgp_stop,    Active},	/* TCP_connection_closed        */
    {bgp_ignore,  Idle},	/* TCP_connection_open_failed   */
    {bgp_stop,    Idle},	/* TCP_fatal_error              */
    {bgp_ignore,  Idle},	/* ConnectRetry_timer_expired   */
    {bgp_fsm_holdtime_expire, Idle},	/* Hold_Timer_expired           */
    {bgp_ignore,  Idle},	/* KeepAlive_timer_expired      */
    {bgp_fsm_open,    OpenConfirm},	/* Receive_OPEN_message         */
    {bgp_ignore,  Idle},	/* Receive_KEEPALIVE_message    */
    {bgp_ignore,  Idle},	/* Receive_UPDATE_message       */
    {bgp_stop_with_error, Idle}, /* Receive_NOTIFICATION_message */
  },
  {
    /* OpenConfirm, */
    {bgp_ignore,  OpenConfirm},	/* BGP_Start                    */
    {bgp_stop,    Idle},	/* BGP_Stop                     */
    {bgp_stop,    Idle},	/* TCP_connection_open          */
    {bgp_stop,    Idle},	/* TCP_connection_closed        */
    {bgp_stop,    Idle},	/* TCP_connection_open_failed   */
    {bgp_stop,    Idle},	/* TCP_fatal_error              */
    {bgp_ignore,  Idle},	/* ConnectRetry_timer_expired   */
    {bgp_fsm_holdtime_expire, Idle},	/* Hold_Timer_expired           */
    {bgp_ignore,  OpenConfirm},	/* KeepAlive_timer_expired      */
    {bgp_ignore,  Idle},	/* Receive_OPEN_message         */
    {bgp_establish, Established}, /* Receive_KEEPALIVE_message    */
    {bgp_ignore,  Idle},	/* Receive_UPDATE_message       */
    {bgp_stop_with_error, Idle}, /* Receive_NOTIFICATION_message */
  },
  {
    /* Established, */
    {bgp_ignore,  Established},	/* BGP_Start                    */
    {bgp_stop,    Idle},	/* BGP_Stop                     */
    {bgp_stop,    Idle},	/* TCP_connection_open          */
    {bgp_stop,    Idle},	/* TCP_connection_closed        */
    {bgp_ignore,  Idle},	/* TCP_connection_open_failed   */
    {bgp_stop,    Idle},	/* TCP_fatal_error              */
    {bgp_ignore,  Idle},	/* ConnectRetry_timer_expired   */
    {bgp_fsm_holdtime_expire, Idle}, /* Hold_Timer_expired           */
    {bgp_fsm_keepalive_expire, Established}, /* KeepAlive_timer_expired      */
    {bgp_stop, Idle},		/* Receive_OPEN_message         */
    {bgp_fsm_keepalive, Established}, /* Receive_KEEPALIVE_message    */
    {bgp_fsm_update,   Established}, /* Receive_UPDATE_message       */
    {bgp_stop_with_error, Idle}, /* Receive_NOTIFICATION_message */
  },
};

static char *bgp_event_str[] =
{
  NULL,
  "BGP_Start",
  "BGP_Stop",
  "TCP_connection_open",
  "TCP_connection_closed",
  "TCP_connection_open_failed",
  "TCP_fatal_error",
  "ConnectRetry_timer_expired",
  "Hold_Timer_expired",
  "KeepAlive_timer_expired",
  "Receive_OPEN_message",
  "Receive_KEEPALIVE_message",
  "Receive_UPDATE_message",
  "Receive_NOTIFICATION_message"
};

/* Execute event process. */
int
bgp_event (struct thread *thread)
{
  int ret;
  int event;
  int next;
  struct peer *peer;

  peer = THREAD_ARG (thread);
  event = THREAD_VAL (thread);

  /* Logging this event. */
  next = FSM [peer->status -1][event - 1].next_state;

  if (BGP_DEBUG (fsm, FSM))
    plog_info (peer->log, "%s [FSM] %s (%s->%s)", peer->host, 
	       bgp_event_str[event],
	       LOOKUP (bgp_status_msg, peer->status),
	       LOOKUP (bgp_status_msg, next));
  if (BGP_DEBUG (normal, NORMAL)
      && strcmp (LOOKUP (bgp_status_msg, peer->status), LOOKUP (bgp_status_msg, next)))
    zlog_info ("%s went from %s to %s",
	       peer->host,
	       LOOKUP (bgp_status_msg, peer->status),
	       LOOKUP (bgp_status_msg, next));

  /* Call function. */
  ret = (*(FSM [peer->status - 1][event - 1].func))(peer);

  /* When function do not want proceed next job return -1. */
  if (ret < 0)
    return ret;
    
  /* If status is changed. */
  if (next != peer->status)
    bgp_fsm_change_status (peer, next);

  /* Make sure timer is set. */
  bgp_timer_set (peer);

  return 0;
}

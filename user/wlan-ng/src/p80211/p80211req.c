/* src/p80211/p80211req.c
*
* Request/Indication/MacMgmt interface handling functions
*
* Copyright (C) 1999 AbsoluteValue Systems, Inc.  All Rights Reserved.
* --------------------------------------------------------------------
*
* linux-wlan
*
*   The contents of this file are subject to the Mozilla Public
*   License Version 1.1 (the "License"); you may not use this file
*   except in compliance with the License. You may obtain a copy of
*   the License at http://www.mozilla.org/MPL/
*
*   Software distributed under the License is distributed on an "AS
*   IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*   implied. See the License for the specific language governing
*   rights and limitations under the License.
*
*   Alternatively, the contents of this file may be used under the
*   terms of the GNU Public License version 2 (the "GPL"), in which
*   case the provisions of the GPL are applicable instead of the
*   above.  If you wish to allow the use of your version of this file
*   only under the terms of the GPL and not to allow others to use
*   your version of this file under the MPL, indicate your decision
*   by deleting the provisions above and replace them with the notice
*   and other provisions required by the GPL.  If you do not delete
*   the provisions above, a recipient may use your version of this
*   file under either the MPL or the GPL.
*
* --------------------------------------------------------------------
*
* Inquiries regarding the linux-wlan Open Source project can be
* made directly to:
*
* AbsoluteValue Systems Inc.
* info@linux-wlan.com
* http://www.linux-wlan.com
*
* --------------------------------------------------------------------
*
* Portions of the development of this software were funded by 
* Intersil Corporation as part of PRISM(R) chipset product development.
*
* --------------------------------------------------------------------
*
* This file contains the functions, types, and macros to support the
* MLME request interface that's implemented via the device ioctls.
* 
* --------------------------------------------------------------------
*/

/*================================================================*/
/* System Includes */

#define __NO_VERSION__		/* prevent the static definition */

#include <linux/config.h>
#include <linux/version.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/wireless.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/sock.h>
#include <linux/netlink.h>

#include <wlan/wlan_compat.h>

/*================================================================*/
/* Project Includes */

#include <wlan/version.h>
#include <wlan/p80211types.h>
#include <wlan/p80211hdr.h>
#include <wlan/p80211mgmt.h>
#include <wlan/p80211conv.h>
#include <wlan/p80211msg.h>
#include <wlan/p80211netdev.h>
#include <wlan/p80211ioctl.h>
#include <wlan/p80211metadef.h>
#include <wlan/p80211metastruct.h>

/*================================================================*/
/* Local Constants */

/* Maximum amount of time we'll wait for a request to complete */
#define P80211REQ_MAXTIME	3*HZ	/* 3 seconds */

/*================================================================*/
/* Local Macros */

/*================================================================*/
/* Local Types */

/*================================================================*/
/* Local Static Definitions */

/* Netlink interface(s) */
static struct sock	*nl_indicate = NULL;

/*================================================================*/
/* Local Function Declarations */

static void p80211req_handlemsg( wlandevice_t *wlandev, p80211msg_t *msg);
static int p80211req_mibset_mibget(wlandevice_t *wlandev, p80211msg_dot11req_mibget_t *mib_msg, int isget);
#ifdef CONFIG_NETLINK
static void p80211ind_rx(struct sock *sk, int len);
#endif

/*================================================================*/
/* Function Definitions */


/*----------------------------------------------------------------
* p80211req_dorequest
*
* Handles an MLME reqest/confirm message.
* 
* Arguments:
*	wlandev		WLAN device struct
*	msgbuf		Buffer containing a request message
*
* Returns: 
*	0 on success, an errno otherwise
*	
* Call context:
*	Potentially blocks the caller, so it's a good idea to 
*	not call this function from an interrupt context.
----------------------------------------------------------------*/
int p80211req_dorequest( wlandevice_t *wlandev, UINT8 *msgbuf)
{
	int		result = 0;
	p80211msg_t	*msg = (p80211msg_t*)msgbuf;

	DBFENTER;

	/* Check to make sure the MSD is running */
	if ( 
	!((wlandev->msdstate == WLAN_MSD_HWPRESENT &&
	msg->msgcode == DIDmsg_lnxreq_ifstate) ||
	wlandev->msdstate == WLAN_MSD_RUNNING || 
	wlandev->msdstate == WLAN_MSD_FWLOAD) ) {
		return -ENODEV;
	}

	/* Check Permissions */
	if (!capable(CAP_NET_ADMIN) && 
	    (msg->msgcode != DIDmsg_dot11req_mibget)) {
		WLAN_LOG_ERROR("%s: only dot11req_mibget allowed for non-root.\n", wlandev->name);
		return -EPERM;
	}

	/* Check for busy status */
	if ( test_and_set_bit(1, &(wlandev->request_pending))) {
		return -EBUSY;
	}

	/* Allow p80211 to look at msg and handle if desired. */
	/* So far, all p80211 msgs are immediate, no waitq/timer necessary */
	/* This may change. */
	p80211req_handlemsg(wlandev, msg);

	/* Pass it down to wlandev via wlandev->mlmerequest */
	if ( wlandev->mlmerequest != NULL ) 
		wlandev->mlmerequest(wlandev, msg);

	clear_bit( 1, &(wlandev->request_pending));
	DBFEXIT;
	return result;	/* if result==0, msg->status still may contain an err */
}

/*----------------------------------------------------------------
* p80211req_handlemsg
*
* p80211 message handler.  Primarily looks for messages that
* belong to p80211 and then dispatches the appropriate response.
* TODO: we don't do anything yet.  Once the linuxMIB is better
*	defined we'll need a get/set handler.  
* 
* Arguments:
*	wlandev		WLAN device struct
*	msg		message structure
*
* Returns: 
*	nothing (any results are set in the status field of the msg)
*	
* Call context:
*	Process thread
----------------------------------------------------------------*/
void p80211req_handlemsg( wlandevice_t *wlandev, p80211msg_t *msg)
{
        DBFENTER;
        
	switch (msg->msgcode) {

	case DIDmsg_lnxreq_hostwep: {
		p80211msg_lnxreq_hostwep_t *req = (p80211msg_lnxreq_hostwep_t*) msg;
		wlandev->hostwep &= ~(HOSTWEP_DECRYPT|HOSTWEP_ENCRYPT);
		if (req->decrypt.data == P80211ENUM_truth_true)
			wlandev->hostwep |= HOSTWEP_DECRYPT;
		if (req->encrypt.data == P80211ENUM_truth_true)
			wlandev->hostwep |= HOSTWEP_ENCRYPT;

		break;
	}
	case DIDmsg_dot11req_mibget:
	case DIDmsg_dot11req_mibset: {
		int isget = (msg->msgcode == DIDmsg_dot11req_mibget);
		p80211msg_dot11req_mibget_t  *mib_msg = (p80211msg_dot11req_mibget_t *) msg;
		p80211req_mibset_mibget (wlandev, mib_msg, isget);
	}
	default:
		// XXX do nothing!
		;
	} /* switch msg->msgcode */

	DBFEXIT;

	return;
}

int p80211req_mibset_mibget(wlandevice_t *wlandev, p80211msg_dot11req_mibget_t *mib_msg, int isget)
{
	p80211itemd_t   *mibitem = (p80211itemd_t *) mib_msg->mibattribute.data;
	p80211pstrd_t  *pstr = (p80211pstrd_t*) mibitem->data;
	UINT8 *key = mibitem->data + sizeof(p80211pstrd_t);

	DBFENTER;

	switch (mibitem->did) {
	case DIDmib_dot11smt_p80211Table_p80211_ifstate: {
		UINT32 *data = (UINT32 *) mibitem->data;
		if (isget)
			switch (wlandev->msdstate) {
			case WLAN_MSD_HWPRESENT:
				*data = P80211ENUM_ifstate_disable;
				break;
			case WLAN_MSD_FWLOAD:
				*data = P80211ENUM_ifstate_fwload;
				break;
			case WLAN_MSD_RUNNING:
				*data = P80211ENUM_ifstate_enable;
				break;
			default:
				*data = P80211ENUM_ifstate_enable;
			}
		break;
	}		
	case DIDmib_dot11phy_dot11PhyOperationTable_dot11ShortPreambleEnabled: {
		UINT32 *data = (UINT32 *) mibitem->data;

		if (isget)
			*data = wlandev->shortpreamble;
		else
			wlandev->shortpreamble = *data;
		break;	
	}
	case DIDmib_dot11smt_dot11WEPDefaultKeysTable_dot11WEPDefaultKey0: {
		if (!isget)
			wep_change_key(wlandev, 0, key, pstr->len);
		break;
	}
	case DIDmib_dot11smt_dot11WEPDefaultKeysTable_dot11WEPDefaultKey1: {
		if (!isget)
			wep_change_key(wlandev, 1, key, pstr->len);
		break;
	}
	case DIDmib_dot11smt_dot11WEPDefaultKeysTable_dot11WEPDefaultKey2: {
		if (!isget)
			wep_change_key(wlandev, 2, key, pstr->len);
		break;
	}
	case DIDmib_dot11smt_dot11WEPDefaultKeysTable_dot11WEPDefaultKey3: {
		if (!isget)
			wep_change_key(wlandev, 3, key, pstr->len);
		break;
	}
	case DIDmib_dot11smt_dot11PrivacyTable_dot11WEPDefaultKeyID: {
		UINT32 *data = (UINT32 *) mibitem->data;
		
		if (isget) {
			*data = wlandev->hostwep & HOSTWEP_DEFAULTKEY_MASK;
			} else {
				wlandev->hostwep &= ~(HOSTWEP_DEFAULTKEY_MASK);
				
				wlandev->hostwep |= (*data & HOSTWEP_DEFAULTKEY_MASK);
			}
		break;
	}
	case DIDmib_dot11smt_dot11PrivacyTable_dot11PrivacyInvoked: {
		UINT32 *data = (UINT32 *) mibitem->data;
		
		if (isget) {
			if (wlandev->hostwep & HOSTWEP_PRIVACYINVOKED)
				*data = P80211ENUM_truth_true;
			else
				*data = P80211ENUM_truth_false;
		} else {
				wlandev->hostwep &= ~(HOSTWEP_PRIVACYINVOKED);
				if (*data == P80211ENUM_truth_true)
					wlandev->hostwep |= HOSTWEP_PRIVACYINVOKED;
		}
		break;
	}
	case DIDmib_dot11smt_dot11PrivacyTable_dot11ExcludeUnencrypted: {
		UINT32 *data = (UINT32 *) mibitem->data;

		if (isget) {
			if (wlandev->hostwep & HOSTWEP_EXCLUDEUNENCRYPTED)
				*data = P80211ENUM_truth_true;
			else
				*data = P80211ENUM_truth_false;
		} else {			
			wlandev->hostwep &= ~(HOSTWEP_EXCLUDEUNENCRYPTED);
			if (*data == P80211ENUM_truth_true)
				wlandev->hostwep |= HOSTWEP_EXCLUDEUNENCRYPTED;
		}
		break;
	}
	default:
		// XXXX do nothing!
		;
	}

	DBFEXIT;
	return 0;
}

#ifdef CONFIG_NETLINK
/*----------------------------------------------------------------
* p80211indicate_init
*
* Called during the p80211 startup to set up the netlink interfaces
* for sniffing and indication messages.
* 
* Arguments:
*	none
*
* Returns: 
*	nothing
*	
* Call context:
*	Any
----------------------------------------------------------------*/
void p80211indicate_init(void)
{
	DBFENTER;
	nl_indicate = 
		netlink_kernel_create( P80211_NL_SOCK_IND, &p80211ind_rx);
	if ( nl_indicate == NULL ) {
		WLAN_LOG_DEBUG(2,"Failed to create indicate netlink i/f.\n");
	}
	DBFEXIT;
	return;
}


/*----------------------------------------------------------------
* p80211indicate_shutdown
*
* Called during the p80211 unload to get rid of our netlink 
* interface.
* 
* Arguments:
*	none
*
* Returns: 
*	nothing
*	
* Call context:
*	Any
----------------------------------------------------------------*/
void p80211indicate_shutdown(void)
{
	struct sock	*nl;
	DBFENTER;
	nl = nl_indicate;
	nl_indicate = NULL;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71) )
	if ( nl != NULL && nl->socket != NULL) {
		sock_release(nl->socket);
	}
#else
	if ( nl != NULL && nl->sk_socket != NULL) {
		sock_release(nl->sk_socket);
	}
#endif

	DBFEXIT;
	return;
}


/*----------------------------------------------------------------
* p80211ind_rx
*
* Called by netlink when a message comes down.  We currently don't
* support message from userland via netlink, so this is a empty
* function.
* 
* Arguments:
*	sk		netlink sock
*	len		msg len
*
* Returns: 
*	nothing
*	
* Call context:
*	Any
----------------------------------------------------------------*/
void p80211ind_rx(struct sock *sk, int len)
{
	return;
}


/*----------------------------------------------------------------
* p80211ind_mlme
*
* Called by the MSD to deliver an mlme type indication message.
* 
* Arguments:
*	wlandev		WLAN device struct
*	skb		skb containing message to deliver
*
* Returns: 
*	nothing
*	
* Call context:
*	Any
----------------------------------------------------------------*/
void p80211ind_mlme( wlandevice_t *wlandev, struct sk_buff *skb)
{
	DBFENTER;
	if ( nl_indicate != NULL ) {

/* TODO: Look at queuing mlme indications when requests are pending. */

		netlink_broadcast(nl_indicate, 
			skb, 0, P80211_NL_MCAST_GRP_MLME, GFP_ATOMIC);
	} else {
		WLAN_LOG_DEBUG(2,"Can't send indicate msg, no netlink i/f\n");
	}
	DBFEXIT;
	return;
}

/*----------------------------------------------------------------
* p80211ind_distribution
*
* Called by the MSD to deliver a distribution system type 
* indication message.
* 
* Arguments:
*	wlandev		WLAN device struct
*	skb		skb containing message to deliver
*
* Returns: 
*	nothing
*	
* Call context:
*	Any
----------------------------------------------------------------*/
void p80211ind_distribution( wlandevice_t *wlandev, struct sk_buff *skb)
{
	DBFENTER;
	if ( nl_indicate != NULL ) {
/*
		skb = alloc_skb(len, GFP_ATOMIC);
		skb_put(skb, len);
		memcpy(skb->data, msg, len);
*/
		netlink_broadcast(nl_indicate, 
			skb, 0, P80211_NL_MCAST_GRP_DIST, GFP_ATOMIC);
	} else {
		WLAN_LOG_DEBUG(2,"Can't send indicate msg, no netlink i/f\n");
	}
	DBFEXIT;
	return;
}
#endif /* CONFIG_NETLINK */

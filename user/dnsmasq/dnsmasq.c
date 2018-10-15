/* dnsmasq is Copyright (c) 2000 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

/* See RFC1035 for details of the protocol this code talks. */

/* Author's email: simon@thekelleys.org.uk */

#include "dnsmasq.h"

static int sighup, sigusr1, sigusr2;

static void sig_handler(int sig)
{
  if (sig == SIGHUP)
    sighup = 1;
  else if (sig == SIGUSR1)
    sigusr1 = 1;
  else if (sig == SIGUSR2)
    sigusr2 = 1;
}

int main (int argc, char **argv)
{
  char *int_err_string;
  int cachesize = CACHESIZ;
  int port = NAMESERVER_PORT;
  int query_port = 0;
  unsigned long local_ttl = 0;
  int logged_lease = 0, first_loop = 1;
  unsigned int options;
  char *runfile = RUNFILE;
  time_t resolv_changed = 0;
  time_t now, last = 0;
  time_t lease_file_change = 0;
  ino_t lease_file_inode = (ino_t)0;
  struct irec *iface, *interfaces = NULL;
  char *mxname = NULL;
  char *mxtarget = NULL;
  char *lease_file = NULL;
  char *addn_hosts = NULL;
  char *domain_suffix = NULL;
  char *username = CHUSER;
  char *groupname = CHGRP;
  struct iname *if_names = NULL;
  struct iname *if_addrs = NULL;
  struct iname *if_except = NULL;
  struct iname *if_tmp;
  struct server *serv_addrs = NULL;
  char *dnamebuff, *packet;
  struct server *servers, *last_server;
  struct resolvc default_resolv = { NULL, 1, 0, RESOLVFILE };
  struct resolvc *resolv = &default_resolv;
  struct bogus_addr *bogus_addr = NULL;
  struct serverfd *serverfdp, *sfds = NULL;

  sighup = 1; /* init cache the first time through */
  sigusr1 = 0; /* but don't dump */
  sigusr2 = 0; /* or rescan interfaces */
  signal(SIGUSR1, sig_handler);
  signal(SIGUSR2, sig_handler);
  signal(SIGHUP, sig_handler);

  /* These get allocated here to avoid overflowing the small stack
     on embedded systems. dnamebuff is big enough to hold one
     maximal sixed domain name and gets passed into all the processing
     code. We manage to get away with one buffer. */
  dnamebuff = safe_malloc(MAXDNAME);
  /* Size: we check after adding each record, so there must be 
     memory for the largest packet, and the largest record */
  packet = safe_malloc(PACKETSZ+MAXDNAME+RRFIXEDSZ);
  
  options = read_opts(argc, argv, dnamebuff, &resolv, &mxname, &mxtarget, &lease_file,
		      &username, &groupname, &domain_suffix, &runfile, 
		      &if_names, &if_addrs, &if_except, &bogus_addr, 
		      &serv_addrs, &cachesize, &port, &query_port, &local_ttl, &addn_hosts);
  
  int_err_string = enumerate_interfaces(&interfaces, if_names, if_addrs, if_except, port);
  
  if (int_err_string)
    die(int_err_string, NULL);
  
  for (if_tmp = if_names; if_tmp; if_tmp = if_tmp->next)
    if (if_tmp->name && !if_tmp->found)
      die("unknown interface %s", if_tmp->name);
  
  for (if_tmp = if_addrs; if_tmp; if_tmp = if_tmp->next)
    if (!if_tmp->found)
      {
	char addrbuff[ADDRSTRLEN];
#ifdef HAVE_IPV6
	if (if_tmp->addr.sa.sa_family == AF_INET)
	  inet_ntop(AF_INET, &if_tmp->addr.in.sin_addr,
		    addrbuff, ADDRSTRLEN);
	  else
	    inet_ntop(AF_INET6, &if_tmp->addr.in6.sin6_addr,
		      addrbuff, ADDRSTRLEN);
#else
	strcpy(addrbuff, inet_ntoa(if_tmp->addr.in.sin_addr));
#endif
	die("no interface with address %s", addrbuff);
      }
      
  forward_init(1);

  cache_init(cachesize, options & OPT_LOG);
  
  setbuf(stdout, NULL);

  if (!(options & OPT_DEBUG))
    {
      FILE *pidfile;
      struct passwd *ent_pw;
      int i;
        
      /* The following code "daemonizes" the process. 
	 See Stevens section 12.4 */

#ifndef NO_FORK
      if (fork() != 0 )
	exit(0);
      
      setsid();
      
      if (fork() != 0)
	exit(0);
#endif
      
      chdir("/");
      umask(022); /* make pidfile 0644 */
      
      /* write pidfile _after_ forking ! */
      if (runfile && (pidfile = fopen(runfile, "w")))
      	{
	  fprintf(pidfile, "%d\n", (int) getpid());
	  fclose(pidfile);
	}
      
      umask(0);

      for (i=0; i<64; i++)
	{
	  for (iface = interfaces; iface; iface = iface->next)
	    if (iface->fd == i)
	      break;
	  if (iface)
	    continue;
	  
	  close(i);
	}

      /* Change uid and gid for security */
      if (username && (ent_pw = getpwnam(username)))
	{
	  gid_t dummy;
	  struct group *gp;
	  /* remove all supplimentary groups */
	  setgroups(0, &dummy);
	  /* change group for /etc/ppp/resolv.conf 
	     otherwise get the group for "nobody" */
	  if ((groupname && (gp = getgrnam(groupname))) || 
	      (gp = getgrgid(ent_pw->pw_gid)))
	    setgid(gp->gr_gid); 
	  /* finally drop root */
	  setuid(ent_pw->pw_uid);
	}
    }

  openlog("dnsmasq", 
	  DNSMASQ_LOG_OPT(options & OPT_DEBUG), 
	  DNSMASQ_LOG_FAC(options & OPT_DEBUG));
  
  if (cachesize)
    syslog(LOG_INFO, "started, version %s cachesize %d", VERSION, cachesize);
  else
    syslog(LOG_INFO, "started, version %s cache disabled", VERSION);
  
  if (options & OPT_LOCALMX)
    syslog(LOG_INFO, "serving MX record for local hosts target %s", mxtarget);
  else if (mxname)
    syslog(LOG_INFO, "serving MX record for mailhost %s target %s", 
	   mxname, mxtarget);
  
  if (getuid() == 0 || geteuid() == 0)
    syslog(LOG_WARNING, "failed to drop root privs");
  
  servers = last_server = check_servers(serv_addrs, interfaces, &sfds);
  
  while (1)
    {
      int ready, maxfd = 0;
      fd_set rset;
      HEADER *header;
      struct stat statbuf;
   
      if (first_loop)
	/* do init stuff only first time round. */
	{
	  first_loop = 0;
	  ready = 0;
	}
      else
	{
	  FD_ZERO(&rset);

	  for (serverfdp = sfds; serverfdp; serverfdp = serverfdp->next)
	    {
	      FD_SET(serverfdp->fd, &rset);
	      if (serverfdp->fd > maxfd)
		maxfd = serverfdp->fd;
	    }

	  for (iface = interfaces; iface; iface = iface->next)
	    {
	      FD_SET(iface->fd, &rset);
	      if (iface->fd > maxfd)
		maxfd = iface->fd;
	    }
	  
	  ready = select(maxfd+1, &rset, NULL, NULL, NULL);
	  
	  if (ready == -1)
	    {
	      if (errno == EINTR)
		ready = 0; /* do signal handlers */
	      else
		continue;
	    }
	}
      
      if (sighup)
	{
	  signal(SIGHUP, SIG_IGN);
	  cache_reload(options, dnamebuff, domain_suffix, addn_hosts);
	  if (resolv && (options & OPT_NO_POLL))
	    servers = last_server = 
	      check_servers(reload_servers(resolv->name, dnamebuff, servers, query_port), 
			    interfaces, &sfds);
	  sighup = 0;
	  signal(SIGHUP, sig_handler);
	}
      
      if (sigusr1)
	{
	  signal(SIGUSR1, SIG_IGN);
	  dump_cache(options & (OPT_DEBUG | OPT_LOG), cachesize);
	  sigusr1 = 0;
	  signal(SIGUSR1, sig_handler);
	}
      
      if (sigusr2)
	{
	  signal(SIGUSR2, SIG_IGN);
	  if (getuid() != 0 && port <= 1024)
	    syslog(LOG_ERR, "cannot re-scan interfaces unless --user=root");
	  else
	   {
	     syslog(LOG_INFO, "rescanning network interfaces");
	     int_err_string = enumerate_interfaces(&interfaces, if_names, if_addrs, if_except, port);
	     if (int_err_string)
	       syslog(LOG_ERR, int_err_string, strerror(errno));
	   }
	  sigusr2 = 0;
	  signal(SIGUSR2, sig_handler);
	  /* may be new file descriptors now, so redo select() */
	  ready = 0;
	}
      
      now = time(NULL);

      /* Check for changes to resolv files and DHCP leases file once per second max. */
      if (last == 0 || now > last)
	{
	  last = now;
	  if (!(options & OPT_NO_POLL))
	    {
	      struct resolvc *res = resolv, *latest = NULL;
	      time_t last_change = 0;
	      /* There may be more than one possible file. 
		 Go through and find the one which changed _last_.
		 Warn of any which can't be read. */
	      while (res)
		{
		  if (stat(res->name, &statbuf) == -1)
		    {
		      if (!res->logged)
			syslog(LOG_WARNING, "failed to access %s: %m", res->name);
		      res->logged = 1;
		    }
		  else
		    {
		      res->logged = 0;
		      if (statbuf.st_mtime > last_change)
			{
			  last_change = statbuf.st_mtime;
			  latest = res;
			}
		    }
		  res = res->next;
		}
	  
	      if (latest && last_change > resolv_changed)
		{
		  resolv_changed = last_change;
		  servers = last_server = 
		    check_servers(reload_servers(latest->name, dnamebuff, servers, query_port),
				  interfaces, &sfds);
		}
	    }

	  if (lease_file)
	    {
syslog(LOG_WARNING, "checking lease file %s", lease_file);
	      if (stat(lease_file, &statbuf) == -1)
		{
		  if (!logged_lease)
		    syslog(LOG_WARNING, "failed to access %s: %m", lease_file);
		  logged_lease = 1;
		}
	      else
		{ 
		  logged_lease = 0;
		  if ((statbuf.st_mtime != lease_file_change) ||
		      (statbuf.st_ino != lease_file_inode))
		    {
		      lease_file_change = statbuf.st_mtime;
		      lease_file_inode = statbuf.st_ino;
		      load_dhcp(lease_file, domain_suffix, now, dnamebuff);
		    }
		}
	    }
	}
		
      if (ready == 0)
	continue; /* no sockets ready */
      
       for (serverfdp = sfds; serverfdp; serverfdp = serverfdp->next)
	 if (FD_ISSET(serverfdp->fd, &rset))
	   last_server = reply_query(serverfdp->fd, options, packet, now, 
				     dnamebuff, last_server, bogus_addr);
      
      for (iface = interfaces; iface; iface = iface->next)
	{
	  if (FD_ISSET(iface->fd, &rset))
	    {
	      /* request packet, deal with query */
	      union mysockaddr udpaddr;
	      socklen_t udplen = sizeof(udpaddr);
	      int m, n = recvfrom(iface->fd, packet, PACKETSZ, 0, &udpaddr.sa, &udplen); 
	      udpaddr.sa.sa_family = iface->addr.sa.sa_family;
#ifdef HAVE_IPV6
	      if (udpaddr.sa.sa_family == AF_INET6)
		udpaddr.in6.sin6_flowinfo = htonl(0);
#endif	      
	      header = (HEADER *)packet;
	      if (n >= (int)sizeof(HEADER) && !header->qr)
		{
		  if (extract_request(header, (unsigned int)n, dnamebuff))
		    {
		      if (udpaddr.sa.sa_family == AF_INET) 
			log_query(F_QUERY | F_IPV4 | F_FORWARD, dnamebuff, 
				  (struct all_addr *)&udpaddr.in.sin_addr);
#ifdef HAVE_IPV6
		      else
			log_query(F_QUERY | F_IPV6 | F_FORWARD, dnamebuff, 
				  (struct all_addr *)&udpaddr.in6.sin6_addr);
#endif
		    }
		  
		  m = answer_request (header, ((char *) header) + PACKETSZ, (unsigned int)n, 
				      mxname, mxtarget, options, now, local_ttl, dnamebuff);
		  if (m >= 1)
		    {
		      /* answered from cache, send reply */
		      sendto(iface->fd, (char *)header, m, 0, 
			     &udpaddr.sa, sa_len(&udpaddr));
		    }
		  else 
		    {
		      /* cannot answer from cache, send on to real nameserver */
		      last_server = forward_query(iface->fd, &udpaddr, header, n, 
						  options, dnamebuff, servers, 
						  last_server, now, local_ttl);
		    }
		}
	      
	    }
	}
    }
  
  return 0;
}







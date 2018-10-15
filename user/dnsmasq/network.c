/* dnsmasq is Copyright (c) 2000 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

/* Author's email: simon@thekelleys.org.uk */

#include "dnsmasq.h"

static char  *add_iface(struct irec **list, unsigned int flags, 
			char *name, union mysockaddr *addr, 
			struct iname *names, struct iname *addrs, struct iname *except)
{
  struct irec *iface;
  int fd, opt;
  
  /* we may need to check the whitelist */
  if (names)
    { 
      struct iname *tmp;
      for (tmp = names; tmp; tmp = tmp->next)
	if (tmp->name && (strcmp(tmp->name, name) == 0))
	  {
	    tmp->found = 1;
	    break;
	  }
      if (!(flags & IFF_LOOPBACK) && !tmp) 
	/* not on whitelist and not loopback */
	return NULL;
    }
  
  if (addrs)
    { 
      struct iname *tmp;
      for (tmp = addrs; tmp; tmp = tmp->next)
	if (sockaddr_isequal(&tmp->addr, addr))
	  {
	    tmp->found = 1;
	    break;
	  }
      
      if (!tmp) 
	/* not on whitelist */
	return NULL;
    }
  
  /* check blacklist */
  if (except)
    {
      struct iname *tmp;
      for (tmp = except; tmp; tmp = tmp->next)
	if (tmp->name && strcmp(tmp->name, name) == 0)
	  return NULL;
    }

  /* check whether the interface IP has been added already 
     it is possible to have multiple interfaces with the same address
     and we may be re-scanning. */
  for (iface = *list; iface; iface = iface->next) 
    if (sockaddr_isequal(&iface->addr, addr))
      break;
  if (iface)
    {
      iface->valid = 1;
      return NULL;
    }

  fd = socket(addr->sa.sa_family, SOCK_DGRAM, 0);
  if (fd == -1)
    return "failed to create socket: %s";

  /* Set SO_REUSEADDR on the socket, this allows is to bind 
     specific addresses even if BIND is running and has bound *:53 */
  opt = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  
  if (bind(fd, &addr->sa, sa_len(addr)))
    {
      int errsave = errno;
      close(fd);
      errno = errsave;
      return "failed to bind socket: %s";
    }

  /* If OK, add it to the head of the list */
  if (!(iface = malloc(sizeof(struct irec))))
    {
      close(fd);
      return "cannot allocate interface";
    }

  iface->fd = fd;
  iface->addr = *addr;
  iface->next = *list;
  iface->valid = 1;
  *list = iface;
  
  return NULL;
}

/* get all interfaces in system and for each one allowed add it to the chain 
   at interfacep. May be called more that once: interfaces which still exist
   are left on the chain, those which have gone have sockets close()ed an are
   unlinked. Return value is NULL if OK, an error string and the value of errno
   on error. */
char *enumerate_interfaces(struct irec **interfacep,
			   struct iname *names,
			   struct iname *addrs,
			   struct iname *except,
			   int port)
{
  /* this code is adapted from Stevens, page 434. It finally
     destroyed my faith in the C/unix API */
  int len = 100 * sizeof(struct ifreq);
  int errsave, lastlen = 0;
  struct irec *iface, *prev;
  char *buf, *ptr, *err = NULL;
  struct ifconf ifc;
  int fd = socket(PF_INET, SOCK_DGRAM, 0);
  
  if (fd == -1)
    return "cannot create socket to enumerate interfaces: %s";
  
  /* make all interfaces as old. Any left that way after the scan are reaped. */
  for (iface = *interfacep; iface; iface = iface->next)
    iface->valid = 0;

  while (1)
    {
      if (!(buf = malloc(len)))
	{
	  err = "cannot allocate buffer";
	  goto end;
	}
      ifc.ifc_len = len;
      ifc.ifc_buf = buf;
      if (ioctl(fd, SIOCGIFCONF, &ifc) < 0)
	{
	  if (errno != EINVAL || lastlen != 0)
	    {
	      err = "ioctl error while enumerating interfaces: %s";
	      goto end;
	    }
	}
      else
	{
	  if (ifc.ifc_len == lastlen)
	    break; /* got a big enough buffer now */
	  lastlen = ifc.ifc_len;
	}
      len += 10*sizeof(struct ifreq);
      free(buf);
    }

  for (ptr = buf; ptr < buf + ifc.ifc_len; )
    {
      struct ifreq *ifr = (struct ifreq *) ptr;
      union mysockaddr addr;
#ifdef HAVE_SOCKADDR_SA_LEN
      ptr += ifr->ifr_addr.sa_len + IF_NAMESIZE;
#else
      ptr += sizeof(struct ifreq);
#endif

      /* copy address since getting flags overwrites */
      if (ifr->ifr_addr.sa_family == AF_INET)
	{
	  addr.in = *((struct sockaddr_in *) &ifr->ifr_addr);
	  addr.in.sin_port = htons(port);
	  
	  if (ioctl(fd, SIOCGIFFLAGS, ifr) < 0)
	    {
	      err = "ioctl error getting interface flags: %m";
	      goto end;
	    }
	  
	  if ((err = add_iface(interfacep, ifr->ifr_flags, ifr->ifr_name, &addr, names, addrs, except)))
	    goto end;
	}
#ifdef HAVE_IPV6
      else if (ifr->ifr_addr.sa_family == AF_INET6)
	{
#ifdef HAVE_BROKEN_SOCKADDR_IN6
	  addr.in6 = *((struct my_sockaddr_in6 *) &ifr->ifr_addr);
#else
	  addr.in6 = *((struct sockaddr_in6 *) &ifr->ifr_addr);
#endif
	  addr.in6.sin6_port = htons(port);
	  addr.in6.sin6_flowinfo = htonl(0);
	  
	  if (ioctl(fd, SIOCGIFFLAGS, ifr) < 0)
	    {
	      err = "ioctl error getting interface flags: %m";
	      goto end;
	    }
	  
	  if ((err = add_iface(interfacep, ifr->ifr_flags, ifr->ifr_name, &addr, names, addrs, except)))
	    goto end;
	}
#endif /* IPV6 */
      
    }

 end:
  errsave = errno; /* since errno gets overwritten by close */
  if (buf)
    free(buf);
  close(fd);
  if (err)
    { 
      errno = errsave;
      return err;
    }

#if defined(HAVE_LINUX_IPV6_PROC) && defined(HAVE_IPV6)
  /* IPv6 addresses don't seem to work with SIOCGIFCONF. Barf */
  /* This code snarfed from net-tools 1.60 and certainly linux specific, though
     it shouldn't break on other Unices, and their SIOGIFCONF might work. */
  {
    FILE *f = fopen(IP6INTERFACES, "r");
    
    if (f)
      {
	union mysockaddr addr;
	unsigned int plen, scope, flags, if_idx;
	char devname[20], addrstring[32];
	
	while (fscanf(f, "%32s %02x %02x %02x %02x %20s\n",
		      addrstring, &if_idx, &plen, &scope, &flags, devname) != EOF) 
	  {
	    int i;
	    unsigned char *addr6p = (unsigned char *) &addr.in6.sin6_addr;
	    memset(&addr, 0, sizeof(addr));
	    addr.sa.sa_family = AF_INET6;
	    for (i=0; i<16; i++)
	      {
		unsigned int byte;
		sscanf(addrstring+i+i, "%02x", &byte);
		addr6p[i] = byte;
	      }
#ifdef HAVE_SOCKADDR_SA_LEN 
	    /* For completeness - should never be defined on Linux. */
	    addr.in6.sin6_len = sizeof(struct sockaddr_in6);
#endif
	    addr.in6.sin6_port = htons(port);
	    addr.in6.sin6_flowinfo = htonl(0);
	    addr.in6.sin6_scope_id = htonl(scope);
	    
	    if ((err = add_iface(interfacep, flags, devname, &addr, names, addrs, except)))
	      {
		errsave = errno;
		fclose(f);
		errno = errsave;
		return err;
	      }
	  }
	
	fclose(f);
      }
  }
#endif /* LINUX */

  /* now remove interfaces which were not found on this scan */
  for(prev = NULL, iface = *interfacep; iface; )
    {
      if (iface->valid)
	{
	  prev = iface;
	  iface = iface->next;
	}
      else
	{
	  struct irec *tmp = iface;
	  close(iface->fd);
	  /* remove pending queries from this interface */
	  reap_forward(iface->fd); 
	  /* unlink */
	  if (prev)
	    prev->next = iface->next;
	  else
	    *interfacep = iface->next;
	  iface = iface->next;
	  free(tmp);
	}
    }
 
  return NULL; /* no error */
}

static struct serverfd *allocate_sfd(union mysockaddr *addr, struct serverfd **sfds)
{
  struct serverfd *sfd;
  
  /* may have a suitable one already */
  for (sfd = *sfds; sfd; sfd = sfd->next )
    if (sockaddr_isequal(&sfd->source_addr, addr))
      return sfd;
  
  /* need to make a new one. */
  errno = ENOMEM; /* in case malloc fails. */
  if (!(sfd = malloc(sizeof(struct serverfd))))
    return NULL;
  
  if ((sfd->fd = socket(addr->sa.sa_family, SOCK_DGRAM, 0)) == -1)
    {
      free(sfd);
      return NULL;
    }
  
  if (bind(sfd->fd, (struct sockaddr *)addr, sa_len(addr)) == -1)
    {
      int errsave = errno; /* save error from bind. */
      close(sfd->fd);
      free(sfd);
      errno = errsave;
      return NULL;
    }
  
  sfd->source_addr = *addr;
  sfd->next = *sfds;
  *sfds = sfd;
  
  return sfd;
}

struct server *check_servers(struct server *new, struct irec *interfaces, struct serverfd **sfds)
{
  char addrbuff[ADDRSTRLEN];
  struct irec *iface;
  struct server *tmp, *ret = NULL;
  int port = 0;

  /* forward table rules reference servers, so have to blow them away */
  forward_init(0);
  
  for (;new; new = tmp)
    {
      tmp = new->next;
      
      if (!(new->flags & (SERV_LITERAL_ADDRESS | SERV_NO_ADDR)))
	{
#ifdef HAVE_IPV6
	  if (new->addr.sa.sa_family == AF_INET)
	    {
	      inet_ntop(AF_INET, &new->addr.in.sin_addr, addrbuff, ADDRSTRLEN);
	      port = ntohs(new->addr.in.sin_port);
	    }
	  else if (new->addr.sa.sa_family == AF_INET6)
	    {
	      inet_ntop(AF_INET6, &new->addr.in6.sin6_addr, addrbuff, ADDRSTRLEN);
	      port = ntohs(new->addr.in6.sin6_port);
	    }
#else
	  strcpy(addrbuff, inet_ntoa(new->addr.in.sin_addr));
	  port = ntohs(new->addr.in.sin_port); 
#endif
	  for (iface = interfaces; iface; iface = iface->next)
	    if (sockaddr_isequal(&new->addr, &iface->addr))
	      break;
	  if (iface)
	    {
	      syslog(LOG_WARNING, "ignoring nameserver %s - local interface", addrbuff);
	      free(new);
	      continue;
	    }
	  
	  /* Do we need a socket set? */
	  if (!new->sfd && !(new->sfd = allocate_sfd(&new->source_addr, sfds)))
	    {
	      syslog(LOG_WARNING, 
		     "ignoring nameserver %s - cannot make/bind socket: %m", addrbuff);
	      free(new);
	      continue;
	    }
	}
      
      /* reverse order - gets it right. */
      new->next = ret;
      ret = new;
      
      if (new->flags & (SERV_HAS_DOMAIN | SERV_FOR_NODOTS))
	{
	  char *s1, *s2;
	  if (new->flags & SERV_HAS_DOMAIN)
	    s1 = "domain", s2 = new->domain;
	  else
	    s1 = "unqualified", s2 = "domains";
	  
	  if (new->flags & SERV_NO_ADDR)
	    syslog(LOG_INFO, "using local addresses only for %s %s", s1, s2);
	  else if (!(new->flags & SERV_LITERAL_ADDRESS))
	    syslog(LOG_INFO, "using nameserver %s#%d for %s %s", addrbuff, port, s1, s2);
	}
      else
	syslog(LOG_INFO, "using nameserver %s#%d", addrbuff, port); 
    }
  
 return ret;
}
  
struct server *reload_servers(char *fname, char *buff, struct server *serv, int query_port)
{
  FILE *f;
  char *line;
  struct server *old_servers = NULL;
  struct server *new_servers = NULL;

  /* move old servers to free list - we can reuse the memory 
     and not risk malloc if there are the same or fewer new servers. 
     Servers which were specced on the command line go to the new list. */
  while (serv)
    {
      struct server *tmp = serv->next;
      if (serv->flags & SERV_FROM_RESOLV)
	{
	  serv->next = old_servers;
	  old_servers = serv;
	}
      else
	{
	  serv->next = new_servers;
	  new_servers = serv;
	}
      serv = tmp;
    }

  /* buff happens to be NAXDNAME long... */
  f = fopen(fname, "r");
  if (!f)
    {
      syslog(LOG_ERR, "failed to read %s: %m", fname);
    }
  else
    {
      syslog(LOG_INFO, "reading %s", fname);
      while ((line = fgets(buff, MAXDNAME, f)))
	{
	  union  mysockaddr addr, source_addr;
	  char *token = strtok(line, " \t\n\r");
	  struct server *serv;
	  
	  if (!token || strcmp(token, "nameserver") != 0)
	    continue;
	  if (!(token = strtok(NULL, " \t\n")))
	    continue;
	  
#ifdef HAVE_IPV6
          if (inet_pton(AF_INET, token, &addr.in.sin_addr))
#else
          if ((addr.in.sin_addr.s_addr = inet_addr(token)) != (in_addr_t) -1)
#endif
	    {
#ifdef HAVE_SOCKADDR_SA_LEN
	      source_addr.in.sin_len = addr.in.sin_len = sizeof(struct sockaddr_in);
#endif
	      source_addr.in.sin_family = addr.in.sin_family = AF_INET;
	      addr.in.sin_port = htons(NAMESERVER_PORT);
	      source_addr.in.sin_addr.s_addr = INADDR_ANY;
	      source_addr.in.sin_port = htons(query_port);
	    }
#ifdef HAVE_IPV6
	  else if (inet_pton(AF_INET6, token, &addr.in6.sin6_addr))
	    {
#ifdef HAVE_SOCKADDR_SA_LEN
	      source_addr.in6.sin6_len = addr.in6.sin6_len = sizeof(struct sockaddr_in6);
#endif
	      source_addr.in6.sin6_family = addr.in6.sin6_family = AF_INET6;
	      addr.in6.sin6_port = htons(NAMESERVER_PORT);
	      source_addr.in6.sin6_flowinfo = addr.in6.sin6_flowinfo = htonl(0);
	      source_addr.in6.sin6_addr= in6addr_any;
	      source_addr.in6.sin6_port = htons(query_port);
	    }
#endif /* IPV6 */
	  else
	    continue;
	  
	  if (old_servers)
	    {
	      serv = old_servers;
	      old_servers = old_servers->next;
	    }
	  else if (!(serv = malloc(sizeof (struct server))))
	    continue;
	  
	  /* this list is reverse ordered: 
	     it gets reversed again in check_servers */
	  serv->next = new_servers;
	  new_servers = serv;
	  serv->addr = addr;
	  serv->source_addr = source_addr;
	  serv->domain = NULL;
	  serv->sfd = NULL;
	  serv->flags = SERV_FROM_RESOLV;
	}
  
      fclose(f);
    }

  /* Free any memory not used. */
  while(old_servers)
    {
      struct server *tmp = old_servers->next;
      free(old_servers);
      old_servers = tmp;
    }

  return new_servers;
}








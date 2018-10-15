/* dnsmasq is Copyright (c) 2000 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include "dnsmasq.h"

static struct crec *cache_head, *cache_tail, **hash_table;
static struct crec *dhcp_inuse, *dhcp_spare, *new_chain;
static int cache_inserted, cache_live_freed, insert_error;
static union bigname *big_free;
static int bignames_left, log_queries, cache_size, hash_size;
static char *addn_file;

static void cache_free(struct crec *crecp);
static void cache_unlink(struct crec *crecp);
static void cache_link(struct crec *crecp);

void cache_init(int size, int logq)
{
  struct crec *crecp;
  int i;

  log_queries = logq;
  cache_head = cache_tail = NULL;
  dhcp_inuse = dhcp_spare = NULL;
  new_chain = NULL;
  cache_size = size;
  big_free = NULL;
  bignames_left = size/10;
  addn_file = NULL;

  cache_inserted = cache_live_freed = 0;

  if (cache_size > 0)
    {
      crecp = safe_malloc(size*sizeof(struct crec));
      
      for (i=0; i<size; i++, crecp++)
	{
	  cache_link(crecp);
	  crecp->flags = 0;
	}
    }
  
  /* hash_size is a power of two. */
  for (hash_size = 64; hash_size < cache_size/10; hash_size = hash_size << 1);
  hash_table = safe_malloc(hash_size*sizeof(struct crec *));
  for(i=0; i < hash_size; i++)
    hash_table[i] = NULL;
}

static struct crec **hash_bucket(unsigned char *name)
{
  unsigned int c, val = 0;
  
  /* don't use tolower and friends here - they may be messed up by LOCALE */
  while((c = *name++))
    if (c >= 'A' && c <= 'Z')
      val += c + 'a' - 'A';
    else
      val += c;
  
  /* hash_size is a power of two */
  return hash_table + (val & (hash_size - 1));
}

static void cache_hash(struct crec *crecp)
{
  struct crec **bucket = hash_bucket(cache_get_name(crecp));
  crecp->hash_next = *bucket;
  *bucket = crecp;
}
 
static void cache_free(struct crec *crecp)
{
  crecp->flags &= ~F_FORWARD;
  crecp->flags &= ~F_REVERSE;

  if (cache_tail)
    cache_tail->next = crecp;
  else
    cache_head = crecp;
  crecp->prev = cache_tail;
  crecp->next = NULL;
  cache_tail = crecp;
  
  /* retrieve big name for further use. */
  if (crecp->flags & F_BIGNAME)
    {
      crecp->name.bname->next = big_free;
      big_free = crecp->name.bname;
      crecp->flags &= ~F_BIGNAME;
    }
}    

/* insert a new cache entry at the head of the list (youngest entry) */
static void cache_link(struct crec *crecp)
{
  if (cache_head) /* check needed for init code */
    cache_head->prev = crecp;
  crecp->next = cache_head;
  crecp->prev = NULL;
  cache_head = crecp;
  if (!cache_tail)
    cache_tail = crecp;
}

/* remove an arbitrary cache entry for promotion */ 
static void cache_unlink (struct crec *crecp)
{
  if (crecp->prev)
    crecp->prev->next = crecp->next;
  else
    cache_head = crecp->next;

  if (crecp->next)
    crecp->next->prev = crecp->prev;
  else
    cache_tail = crecp->prev;
}

char *cache_get_name(struct crec *crecp)
{
  /* F_BIGNAME set on DHCP entries means non-standard sized cache struct,
     NOT a pointer to a bigname struct. */
  return ((crecp->flags & (F_BIGNAME | F_DHCP)) == F_BIGNAME) ? 
    crecp->name.bname->name : crecp->name.sname;
}

static void cache_scan_free(char *name, struct all_addr *addr, time_t now, unsigned short flags)
{
  /* Scan and remove old entries.
     If (flags & F_FORWARD) then remove any forward entries for name and any expired
     entries but only in the same hash bucket as name.
     If (flags & F_REVERSE) then remove any reverse entries for addr and any expired
     entries in the whole cache.
     If (flags == 0) remove any expired entries in the whole cache. */
  
#define F_CACHESTATUS (F_HOSTS | F_DHCP | F_FORWARD | F_REVERSE | F_IPV4 | F_IPV6)
  struct crec *crecp, **up;
  flags &= (F_FORWARD | F_REVERSE | F_IPV6 | F_IPV4);

  if (flags & F_FORWARD)
    {
      for (up = hash_bucket(name), crecp = *up; crecp; crecp = crecp->hash_next)
	if ((crecp->ttd < now && !(crecp->flags & F_IMMORTAL)) ||
	    ((flags == (crecp->flags & F_CACHESTATUS)) && hostname_isequal(cache_get_name(crecp), name)))
	  {
	    *up = crecp->hash_next;
	    if (!(crecp->flags & (F_HOSTS | F_DHCP)))
	      { 
		cache_unlink(crecp);
		cache_free(crecp);
	      }
	  }
	else
	  up = &crecp->hash_next;
    }
  else
    {
      int i;
#ifdef HAVE_IPV6
      int addrlen = (flags & F_IPV6) ? IN6ADDRSZ : INADDRSZ;
#else
      int addrlen = INADDRSZ;
#endif 
      for (i = 0; i < hash_size; i++)
	for (crecp = hash_table[i], up = &hash_table[i]; crecp; crecp = crecp->hash_next)
	  if ((crecp->ttd < now && !(crecp->flags & F_IMMORTAL)) ||
	      ((flags == (crecp->flags & F_CACHESTATUS)) && memcmp(&crecp->addr, addr, addrlen) == 0))
	    {
	      *up = crecp->hash_next;
	      if (!(crecp->flags & (F_HOSTS | F_DHCP)))
		{ 
		  cache_unlink(crecp);
		  cache_free(crecp);
		}
	    }
	  else
	    up = &crecp->hash_next;
    }
}

/* Note: The normal calling sequence is
   cache_start_insert
   cache_insert * n
   cache_end_insert

   but an abort can cause the cache_end_insert to be missed 
   in which can the next cache_start_insert cleans things up. */

void cache_start_insert(void)
{
  /* Free any entries which didn't get committed during the last
     insert due to error.
  */
  while (new_chain)
    {
      struct crec *tmp = new_chain->next;
      cache_free(new_chain);
      new_chain = tmp;
    }
  new_chain = NULL;
  insert_error = 0;
}
 
void cache_insert(char *name, struct all_addr *addr, 
		  time_t now,  unsigned long ttl, unsigned short flags)
{
#ifdef HAVE_IPV6
  int addrlen = (flags & F_IPV6) ? IN6ADDRSZ : INADDRSZ;
#else
  int addrlen = INADDRSZ;
#endif
  struct crec *new;
  union bigname *big_name = NULL;
  int freed_all = flags & F_REVERSE;

  log_query(flags | F_UPSTREAM, name, addr);

  /* name is needed as workspace by log_query in this case */
  if ((flags & F_NEG) && (flags & F_REVERSE))
    name = NULL;

  /* CONFIG bit no needed except for logging */
  flags &= ~F_CONFIG;

  /* if previous insertion failed give up now. */
  if (insert_error)
    return;

  /* First remove any expired entries and entries for the name/address we
     are currently inserting. */
  cache_scan_free(name, addr, now, flags);
  
  /* Now get a cache entry from the end of the LRU list */
  while (1) {
    if (!(new = cache_tail)) /* no entries left - cache is too small, bail */
      {
	insert_error = 1;
	return;
      }
    
    /* End of LRU list is still in use: if we didn't scan all the hash
       chains for expired entries do that now. If we already tried that
       then it's time to start spilling things. */
    
    if (new->flags & (F_FORWARD | F_REVERSE))
      { 
	if (freed_all)
	  {
	    cache_scan_free(cache_get_name(new), &new->addr, now, new->flags);
	    cache_live_freed++;
	  }
	else
	  {
	    cache_scan_free(NULL, NULL, now, 0);
	    freed_all = 1;
	  }
	continue;
      }
 
    /* Check if we need to and can allocate extra memory for a long name.
       If that fails, give up now. */
    if (name && (strlen(name) > SMALLDNAME-1))
      {
	if (big_free)
	  { 
	    big_name = big_free;
	    big_free = big_free->next;
	  }
	else if (!bignames_left ||
		 !(big_name = (union bigname *)malloc(sizeof(union bigname))))
	  {
	    insert_error = 1;
	    return;
	  }
	else
	  bignames_left--;
	
      }

    /* Got the rest: finally grab entry. */
    cache_unlink(new);
    break;
  }
  
  new->flags = flags;
  if (big_name)
    {
      new->name.bname = big_name;
      new->flags |= F_BIGNAME;
    }
  if (name)
    strcpy(cache_get_name(new), name);
  else
    *cache_get_name(new) = 0;
  if (addr)
    memcpy(&new->addr, addr, addrlen);
  new->ttd = ttl + now;
  new->next = new_chain;
  new_chain = new;
}

/* after end of insertion, commit the new entries */
void cache_end_insert(void)
{
  if (insert_error)
    return;
  
  while (new_chain)
    { 
      struct crec *tmp = new_chain->next;
      cache_hash(new_chain);
      cache_link(new_chain);
      new_chain = tmp;
      cache_inserted++;
    }
  new_chain = NULL;
}

struct crec *cache_find_by_name(struct crec *crecp, char *name, time_t now, unsigned short prot)
{
  struct crec *ans;

  if (crecp) /* iterating */
    ans = crecp->next;
  else
    {
      /* first search, look for relevant entries and push to top of list
	 also free anything which has expired */
      struct crec *next, **up, **insert = NULL, **chainp = &ans;
         
      for (up = hash_bucket(name), crecp = *up; crecp; crecp = next)
	{
	  next = crecp->hash_next;
	  
	  if ((crecp->flags & F_IMMORTAL) || crecp->ttd > now)
	    {
	      if ((crecp->flags & F_FORWARD) && 
		  (crecp->flags & prot) &&
		  hostname_isequal(cache_get_name(crecp), name))
		{
		  if (crecp->flags & (F_HOSTS | F_DHCP))
		    {
		      *chainp = crecp;
		      chainp = &crecp->next;
		    }
		  else
		    {
		      cache_unlink(crecp);
		      cache_link(crecp);
		    }
	      	      
		  /* move all but the first entry up the hash chain
		     this implements round-robin */
		  if (!insert)
		    {
		      insert = up; 
		      up = &crecp->hash_next; 
		    }
		  else
		    {
		      *up = crecp->hash_next;
		      crecp->hash_next = *insert;
		      *insert = crecp;
		      insert = &crecp->hash_next;
		    }
		}
	      else
		/* case : not expired, incorrect entry. */
		up = &crecp->hash_next; 
	    }
	  else
	    {
	      /* expired entry, free it */
	      *up = crecp->hash_next;
	      if (!(crecp->flags & (F_HOSTS | F_DHCP)))
		{ 
		  cache_unlink(crecp);
		  cache_free(crecp);
		}
	    }
	}
	  
      *chainp = cache_head;
    }

  if (ans && 
      (ans->flags & F_FORWARD) &&
      (ans->flags & prot) &&
      hostname_isequal(cache_get_name(ans), name))
    return ans;
  
  return NULL;
}

struct crec *cache_find_by_addr(struct crec *crecp, struct all_addr *addr, 
				time_t now, unsigned short prot)
{
  struct crec *ans;
#ifdef HAVE_IPV6
  int addrlen = (prot == F_IPV6) ? IN6ADDRSZ : INADDRSZ;
#else
  int addrlen = INADDRSZ;
#endif
  
  if (crecp) /* iterating */
    ans = crecp->next;
  else
    {  
      /* first search, look for relevant entries and push to top of list
	 also free anything which has expired */
       int i;
       struct crec **up, **chainp = &ans;
       
       for(i=0; i<hash_size; i++)
	 for (crecp = hash_table[i], up = &hash_table[i]; crecp; crecp = crecp->hash_next)
	   if ((crecp->flags & F_IMMORTAL) || crecp->ttd > now)
	     {      
	       if ((crecp->flags & F_REVERSE) && 
		   (crecp->flags & prot) &&
		   memcmp(&crecp->addr, addr, addrlen) == 0)
		 {	    
		   if (crecp->flags & (F_HOSTS | F_DHCP))
		     {
		       *chainp = crecp;
		       chainp = &crecp->next;
		     }
		   else
		     {
		       cache_unlink(crecp);
		       cache_link(crecp);
		     }
		 }
	       up = &crecp->hash_next;
	     }
	   else
	     {
	       *up = crecp->hash_next;
	       if (!(crecp->flags & (F_HOSTS | F_DHCP)))
		 {
		   cache_unlink(crecp);
		   cache_free(crecp);
		 }
	     }
       
       *chainp = cache_head;
    }
  
  if (ans && 
      (ans->flags & F_REVERSE) &&
      (ans->flags & prot) &&
      memcmp(&ans->addr, addr, addrlen) == 0)
    return ans;
  
  return NULL;
}

static void add_hosts_entry(struct crec *cache, struct all_addr *addr, int addrlen, unsigned short flags)
{
  struct crec *lookup = cache_find_by_name(NULL, cache->name.sname, 0, flags & (F_IPV4 | F_IPV6));

  /* Remove duplicates in hosts files. */
  if (lookup  && (lookup->flags & F_HOSTS) &&
      memcmp(&lookup->addr, addr, addrlen) == 0)
    free(cache);
  else
    {
      cache->flags = flags;
      memcpy(&cache->addr, addr, addrlen);
      cache_hash(cache);

    }
}

static void read_hostsfile(char *filename, int opts, char *buff, char *domain_suffix, unsigned short addn_flag)
{  
  FILE *f = fopen(filename, "r");
  char *line;
  int count = 0, lineno = 0;
  
  if (!f)
    {
      syslog(LOG_ERR, "failed to load names from %s: %m", filename);
      return;
    }
    
  while ((line = fgets(buff, MAXDNAME, f)))
    {
      struct all_addr addr;
      char *token = strtok(line, " \t\n\r");
      int addrlen;
      unsigned short flags;
          
      lineno++;

      if (!token || (*token == '#')) 
	continue;

#ifdef HAVE_IPV6      
      if (inet_pton(AF_INET, token, &addr) == 1)
	{
	  flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV4;
	  addrlen = INADDRSZ;
	}
      else if (inet_pton(AF_INET6, token, &addr) == 1)
	{
	  flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV6;
	  addrlen = IN6ADDRSZ;
	}
#else 
     if ((addr.addr.addr4.s_addr = inet_addr(token)) != (in_addr_t) -1)
        {
          flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV4;
          addrlen = INADDRSZ;
	}
#endif
      else
	continue;

     while ((token = strtok(NULL, " \t\n\r")) && (*token != '#'))
       {
	 struct crec *cache;
	 if (canonicalise(token))
	   {
	     count++;
	     /* If set, add a version of the name with a default domain appended */
	     if ((opts & OPT_EXPAND) && domain_suffix && !strchr(token, '.') && 
		 (cache = malloc(sizeof(struct crec) + 
				 strlen(token)+2+strlen(domain_suffix)-SMALLDNAME)))
	       {
		 strcpy(cache->name.sname, token);
		 strcat(cache->name.sname, ".");
		 strcat(cache->name.sname, domain_suffix);
		 add_hosts_entry(cache, &addr, addrlen, flags | addn_flag);
		 /* Only first name is cannonical and used for reverse lookups */
		 flags &=  ~F_REVERSE;
	       }
	     if ((cache = malloc(sizeof(struct crec) + strlen(token)+1-SMALLDNAME)))
	       {
		 strcpy(cache->name.sname, token);
		 add_hosts_entry(cache, &addr, addrlen, flags | addn_flag);
		 /* Clear this here in case not done above. */
		 flags &=  ~F_REVERSE;
	       }
	   }
	 else
	   syslog(LOG_ERR, "bad name at %s line %d", filename, lineno); 
       }
    }
  
  fclose(f);

  syslog(LOG_INFO, "read %s - %d addresses", filename, count);
}
	    
void cache_reload(int opts, char *buff, char *domain_suffix, char *addn_hosts)
{
  struct crec *cache, **up, *tmp;
  int i;

  for (i=0; i<hash_size; i++)
    for (cache = hash_table[i], up = &hash_table[i]; cache; cache = tmp)
      {
	tmp = cache->hash_next;
	if (cache->flags & F_HOSTS)
	  {
	    *up = cache->hash_next;
	    free(cache);
	  }
	else if (!(cache->flags & F_DHCP))
	  {
	    *up = cache->hash_next;
	    if (cache->flags & F_BIGNAME)
	      {
		cache->name.bname->next = big_free;
		big_free = cache->name.bname;
	      }
	    cache->flags = 0;
	  }
	else
	  up = &cache->hash_next;
      }
  
  if ((opts & OPT_NO_HOSTS) && !addn_hosts)
    {
      if (cache_size > 0)
	syslog(LOG_INFO, "cleared cache");
      return;
    }

  if (!(opts & OPT_NO_HOSTS))
    read_hostsfile(HOSTSFILE, opts, buff, domain_suffix, 0);
  if (addn_hosts)
    {
      read_hostsfile(addn_hosts, opts, buff, domain_suffix, F_ADDN);
      addn_file = addn_hosts;
    }
} 

void cache_unhash_dhcp(void)
{
  struct crec *tmp, *cache, **up;
  int i;

  for (i=0; i<hash_size; i++)
    for (cache = hash_table[i], up = &hash_table[i]; cache; cache = cache->hash_next)
      if (cache->flags & F_DHCP)
	*up = cache->hash_next;
      else
	up = &cache->hash_next;

  /* prev field links all dhcp entries */
  for (cache = dhcp_inuse; cache; cache = tmp)
    {
      tmp = cache->prev;
      /* we don't reuse large structs - they get freed and reallocated. */
      if (cache->flags & F_BIGNAME)
	free(cache);
      else
	{
	  cache->prev = dhcp_spare;
	  dhcp_spare = cache;
	}
    }

  dhcp_inuse = NULL;
}

void cache_add_dhcp_entry(char *host_name, struct all_addr *host_address, time_t ttd, unsigned short flags) 
{
  struct crec *crec;
  
  if ((crec = cache_find_by_name(NULL, host_name, 0, F_IPV4)))
    {
      if (crec->flags & F_HOSTS)
	syslog(LOG_WARNING, "Ignoring DHCP lease for %s because it clashes with an /etc/hosts entry.", host_name);
      else if (!(crec->flags & F_DHCP))
	{
	  if (crec->flags & F_NEG)
	    {
	      /* name may have been searched for before being allocated to DHCP and 
		 therefore got a negative cache entry. If so delete it and continue. */
	      cache_scan_free(host_name, NULL, 0, F_IPV4 | F_FORWARD);
	      goto newrec;
	    }
	  else
	    syslog(LOG_WARNING, "Ignoring DHCP lease for %s because it clashes with a cached name.", cache_get_name(crec));
	}
      else if (ttd > crec->ttd || ttd == (time_t)-1)
	{
	  /* simply a later entry in the leases file which supercedes and earlier one. */
	  memcpy(&crec->addr, host_address, INADDRSZ);
	  if (ttd == (time_t)-1)
	    crec->flags |= F_IMMORTAL;
	  else
	    crec->ttd = ttd;
	}
    }
  else if ((crec = cache_find_by_addr(NULL, host_address, 0, F_IPV4)) && (crec->flags & F_NEG))
    {
      cache_scan_free(NULL, host_address, 0, F_IPV4 | F_REVERSE);
      goto newrec;
    }
  else
    { /* can't find it in cache. */
    newrec:
      crec = dhcp_spare;
      if ((strlen (host_name) < SMALLDNAME-1) && crec) 
	/* old entries to reuse (will not fit large names) */
        dhcp_spare = dhcp_spare->prev;
      else /* need new one */
	{
	  if ((strlen(host_name) < SMALLDNAME-1))
	    crec = malloc(sizeof(struct crec));
	  else
	    {
	      flags |= F_BIGNAME;
	      crec = malloc(sizeof(struct crec) +
			    strlen(host_name)+1-SMALLDNAME);
	    }
	}
      
      if (crec) /* malloc may fail */
	{
	  crec->flags = F_DHCP | F_FORWARD | F_IPV4 | flags;
	  if (ttd == (time_t)-1)
	    crec->flags |= F_IMMORTAL;
	  else
	    crec->ttd = ttd;
	  memcpy(&crec->addr, host_address, INADDRSZ);
	  strcpy(cache_get_name(crec), host_name);
	  crec->prev = dhcp_inuse;
	  dhcp_inuse = crec;
	  cache_hash(crec);
	}
    }
}



void dump_cache(int debug, int cache_size)
{
  syslog(LOG_INFO, "Cache size %d, %d/%d cache insertions re-used unexpired cache entries.", 
	 cache_size, cache_live_freed, cache_inserted); 
  
  if (debug)
    {
      struct crec *cache ;
      char addrbuff[ADDRSTRLEN];
      int i;
      syslog(LOG_DEBUG, "Host                                     Address                        Flags     Expires\n");
    
      for (i=0; i<hash_size; i++)
	for (cache = hash_table[i]; cache; cache = cache->hash_next)
	  {
	    if ((cache->flags & F_NEG) && (cache->flags & F_FORWARD))
	      addrbuff[0] = 0;
#ifdef HAVE_IPV6
	    else if (cache->flags & F_IPV4)
	      inet_ntop(AF_INET, &cache->addr, addrbuff, ADDRSTRLEN);
	    else if (cache->flags & F_IPV6)
	      inet_ntop(AF_INET6, &cache->addr, addrbuff, ADDRSTRLEN);
#else
            else 
	      strcpy(addrbuff, inet_ntoa(cache->addr.addr.addr4));
#endif
	    syslog(LOG_DEBUG, "%-40.40s %-30.30s %s%s%s%s%s%s%s%s%s%s  %s",
		   cache_get_name(cache), addrbuff,
		   cache->flags & F_IPV4 ? "4" : "",
		   cache->flags & F_IPV6 ? "6" : "",
		   cache->flags & F_FORWARD ? "F" : " ",
		   cache->flags & F_REVERSE ? "R" : " ",
		   cache->flags & F_IMMORTAL ? "I" : " ",
		   cache->flags & F_DHCP ? "D" : " ",
		   cache->flags & F_NEG ? "N" : " ",
		   cache->flags & F_NXDOMAIN ? "X" : " ",
		   cache->flags & F_HOSTS ? "H" : " ",
		   cache->flags & F_ADDN ? "A" : " ",
		   cache->flags & F_IMMORTAL ? "\n" : ctime(&(cache->ttd))) ;
	  }
      
    }
}


void log_query(unsigned short flags, char *name, struct all_addr *addr)
{
  char *source;
  char *verb = "is";
  char addrbuff[ADDRSTRLEN];
  
  if (!log_queries)
    return;
  
  if (flags & F_NEG)
    {
      if (flags & F_REVERSE)
#ifdef HAVE_IPV6
	inet_ntop(flags & F_IPV4 ? AF_INET : AF_INET6,
		  addr, name, MAXDNAME);
#else
        strcpy(name, inet_ntoa(addr->addr.addr4));  
#endif
	      
      if (flags & F_NXDOMAIN)
	strcpy(addrbuff, "<NXDOMAIN>-");
      else
	strcpy(addrbuff, "<NODATA>-");
      
      if (flags & F_IPV4)
	strcat(addrbuff, "IPv4");
      else
	strcat(addrbuff, "IPv6");
    }
  else
#ifdef HAVE_IPV6
    inet_ntop(flags & F_IPV4 ? AF_INET : AF_INET6,
	      addr, addrbuff, ADDRSTRLEN);
#else
    strcpy(addrbuff, inet_ntoa(addr->addr.addr4));  
#endif
  
  if (flags & F_DHCP)
    source = "DHCP";
  else if (flags & F_HOSTS)
    {
      if (flags & F_ADDN)
	source = addn_file;
      else
	source = HOSTSFILE;
    }
   else if (flags & F_CONFIG)
    source = "config";
  else if (flags & F_UPSTREAM)
    source = "reply";
  else if (flags & F_SERVER)
    {
      source = "forwarded";
      verb = "to";
    }
  else if (flags & F_QUERY)
    {
      source = "query";
      verb = "from";
    }
  else
    source = "cached";
  
  if ((flags & F_FORWARD) | (flags & F_NEG))
    syslog(LOG_DEBUG, "%s %s %s %s", source, name, verb, addrbuff);
  else if (flags & F_REVERSE)
    syslog(LOG_DEBUG, "%s %s is %s", source, addrbuff, name);
}


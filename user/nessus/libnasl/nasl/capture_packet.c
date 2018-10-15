/* Nessus Attack Scripting Language 
 *
 * Copyright (C) 2002 - 2004 Tenable Network Security
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <includes.h>
#include <pcap.h>

#include "capture_packet.h"
#include "nasl_raw.h"



extern int islocalhost(struct in_addr *);




/*
 * Set up the pcap filter, and select the correct interface.
 *
 * The filter will be changed only if this is necessary
 * 
 */
 
int init_capture_device(struct in_addr src, struct in_addr dst, char * filter)

{
 int ret = -1;
 char * interface = NULL;
 char * a_dst, *a_src;
 char errbuf[PCAP_ERRBUF_SIZE];
 int free_filter = 0;
 
 a_src = estrdup(inet_ntoa(src));
 a_dst = estrdup(inet_ntoa(dst));
 
 if((filter == NULL) || (filter[0]=='\0') || (filter[0]=='0'))
 {
  filter = emalloc(256);
  free_filter = 1;
  if(islocalhost(&src) == 0)
  	snprintf(filter, 256, "ip and (src host %s and dst host %s)",
 		  a_src, a_dst);
		  
 }
 else {
 	if(islocalhost(&src) == 0)filter = estrdup(filter);
	else filter = emalloc(1);
	free_filter = 1;
 	}
 
 efree(&a_dst);
 efree(&a_src);

 if((interface = routethrough(&src, &dst))||
    (interface = pcap_lookupdev(errbuf)))   
    ret = bpf_open_live(interface, filter);
     
 
 if(free_filter != 0)efree(&filter);

 return ret;   
}

struct ip * capture_next_packet(int bpf, int timeout, int * sz)
{
  int len;
  int dl_len;
  char * packet = NULL;
  char * ret = NULL;
  struct timeval past, now, then;
  struct timezone tz;
 
  if(bpf < 0)
   return NULL;
   
  dl_len =  get_datalink_size(bpf_datalink(bpf));
  bzero(&past, sizeof(past));
  bzero(&now, sizeof(now));
  gettimeofday(&then, &tz);
  for(;;)
  {
   bcopy(&then, &past, sizeof(then));
   packet = (char*)bpf_next(bpf, &len);
   if(packet != NULL)
     break;
   gettimeofday(&now, &tz);
  
   if(now.tv_usec < past.tv_usec)
   	{
	past.tv_sec ++;
	now.tv_usec += 1000000;
	}
	
   if(timeout > 0)
   {
    if((now.tv_sec - past.tv_sec) >= timeout)
    	break;
    }
   else break;
   }	
  
  
  if(packet != NULL)
  {
   struct ip * ip;
   ip = (struct ip *)(packet + dl_len);
#ifdef BSD_BYTE_ORDERING
   ip->ip_len = ntohs(ip->ip_len);
   ip->ip_off = ntohs(ip->ip_off);
#endif   
   ip->ip_id = ntohs(ip->ip_id);
   ret = emalloc(len - dl_len);
   bcopy(ip, ret, len -  dl_len);
   if(sz != NULL)*sz = len - dl_len;
  }
 return((struct ip*)ret);
}

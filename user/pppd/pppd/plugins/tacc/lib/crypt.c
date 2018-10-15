/*
 * Copyright 1997-2000 Pawel Krawczyk <kravietz@ceti.pl>
 *
 * See http://www.ceti.com.pl/~kravietz/progs/tacacs.html
 * for details.
 *
 * crypt.c  TACACS+ encryption related functions
 *
 */

#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "tacplus.h"
#include "libtac.h"
#include "xalloc.h"
#include "md5.h"

/* Produce MD5 pseudo-random pad for TACACS+ encryption.
   Use data from packet header and secret, which
   should be a global variable */
u_char *_tac_md5_pad(int len, HDR *hdr)  {
 int n, i, bufsize;
 int bp=0; /* buffer pointer */
 int pp=0; /* pad pointer */
 u_char *pad;
 u_char *buf;
 MD5_CTX mdcontext;

 /* make pseudo pad */
 n=(int)(len/16)+1;  /* number of MD5 runs */
 bufsize=sizeof(hdr->session_id) + strlen(tac_secret) + sizeof(hdr->version)
         + sizeof(hdr->seq_no) + MD5_LEN + 10;
 buf= (u_char *) xcalloc(1, bufsize);
 pad= (u_char *) xcalloc(n, MD5_LEN);

 for(i=0; i<n; i++) {
 /* MD5_1 = MD5{session_id, secret, version, seq_no}
    MD5_2 = MD5{session_id, secret, version, seq_no, MD5_1} */

  /* place session_id, key, version and seq_no in buffer */
  bp=0;
  bcopy(&hdr->session_id, buf, sizeof(session_id));
  bp+=sizeof(session_id);
  bcopy(tac_secret, buf+bp, strlen(tac_secret));
  bp+=strlen(tac_secret);
  bcopy(&hdr->version, buf+bp, sizeof(hdr->version));
  bp+=sizeof(hdr->version);
  bcopy(&hdr->seq_no, buf+bp, sizeof(hdr->seq_no));
  bp+=sizeof(hdr->seq_no);

  /* append previous pad if this is not the first run */
  if(i) {
    bcopy(pad+((i-1)*MD5_LEN), buf+bp, MD5_LEN);
    bp+=MD5_LEN;
  }
  
  MD5_Init(&mdcontext);
  MD5_Update(&mdcontext, buf, bp);
  /* this is because MD5 implementation has changed between
   * pppd versions 2.2.0g and 2.3.4
   */
#if 1
  MD5_Final(pad+pp, &mdcontext); /* correct for pppd-2.3.4 */
#else
  MD5_Final(&mdcontext); /* correct for pppd-2.2.0g */
  bcopy(&mdcontext.digest, pad+pp, MD5_LEN);
#endif
   
  pp+=MD5_LEN;
 }

 free(buf);
 return(pad);
 
} /* _tac_md5_pad */

/* Perform encryption/decryption on buffer. This means simply XORing
   each byte from buffer with according byte from pseudo-random
   pad. */
void _tac_crypt(u_char *buf, HDR *th, int length) {
 int i;
 u_char *pad;
 
 /* null operation if no encryption requested */
 if(th->encryption == TAC_PLUS_ENCRYPTED) {
 
  pad=_tac_md5_pad(length, th);
 
  for(i=0; i<length; i++) {
   *(buf+i) ^= pad[i];
  }
  
  free(pad);
 
 } else {
  syslog(LOG_WARNING, "%s: using no TACACS+ encryption", __FUNCTION__);
 }
} /* _tac_crypt */

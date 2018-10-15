#ifndef _cygwin_asm_socket_h
#define _cygwin_asm_socket_h

#include_next <asm/socket.h>

/* This is a lame cop-out, but cygwin's SIOCGIFCONF doesn't define
   IFF_POINTOPOINT, so this should never happen anyway. */
#ifndef SIOCGIFDSTADDR
# define SIOCGIFDSTADDR SIOCGIFADDR
#endif

#endif

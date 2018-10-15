#ifndef ___INCLUDES_H__
#define ___INCLUDES_H__
/*
 * Nessus system includes 
 */
#ifdef _CYGWIN_
#undef _WIN32
#endif


#ifdef _WIN32
#include "config.w32"
#else
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef linux
/* avoid symbol clash with librpcsvc.a(xmount.o) */
#define xdr_fhstatus xDr_fHsTaTuS
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif

#if defined (STDC_HEADERS) && defined (HAVE_STRING_H)
# include <string.h>
#else                                                     
# ifndef HAVE_STRCHR                                                     
#  define strchr index                                                   
#  define strrchr rindex                                                
# endif /* not defined HAVE_STRCHR */
  char *strchr (), *strrchr ();                 
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))                      
#  define memmove(d, s, n) bcopy ((s), (d), (n))                        
# endif  /* not HAVE_MEMCPY */
#endif  /* STDC_HEADERS && HAVE_STRING_H */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */


#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if HAVE_SYS_WAIT_H                                                     
#include <sys/wait.h>                                                
#endif                                                                

#ifndef WEXITSTATUS
#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)               
#endif                                                                

#ifndef WIFEXITED                                                        
#define WIFEXITED(stat_val) (((stat_val) & 255) == 0)                 
#endif    

#ifdef TIME_WITH_SYS_TIME                                           
# include <sys/time.h>                                                
# include <time.h>                                               
#else                
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h> 
# else                                                                
#  include <time.h>                                                  
# endif                                                                
#endif   

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_DIRENT_H                                                     
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)                 
#else                                                                  
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen                         
# ifdef HAVE_SYS_NDIR_H                                                     
#  include <sys/ndir.h>                                                  
# endif                                                             
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>                                                
# endif                                                               
# ifdef HAVE_NDIR_H                                                         
#  include <ndir.h>                                         
# endif            
#endif   

#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif

#ifdef HAVE_SSL
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif

/*
 * Threads management
 *
 * Nessus is currently able to deal with 
 * 3 kinds of threads API :
 *
 * - the POSIX Threads
 * - the WindowsNT Threads
 * - fork() used as Threads
 *
 */

#ifdef NESSUSNT
#define USE_NT_THREADS
#else
#ifndef USE_PTHREADS
#define USE_FORK_THREADS
#endif
#endif

#ifdef USE_PTHREADS
# ifdef HAVE_PTHREAD_H
#  include <pthread.h>
# else
#  error "Your system is lacking pthread support"
# endif
#endif

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif


#ifdef HAVE_LINUX_MSG_H
#include <linux/msg.h>
#else
#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#ifdef HAVE_SYS_MSG_H
#include <sys/msg.h>
#endif
#endif


#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif


#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#ifndef MSGMAX
#define MSGMAX 4000 	/* This is awkwardly arbitrary */
#endif

#ifdef BSD_BYTE_ORDERING
# define FIX(n) (n)
# define UNFIX(n) (n)
#else
# define FIX(n) htons(n)
# define UNFIX(n) ntohs(n)
#endif

#include <pcap.h>

#include <ntcompat.h>
#include <libnessus.h>
#include <nessus-devel.h>

#endif /* not defined(___INCLUDES_H) */



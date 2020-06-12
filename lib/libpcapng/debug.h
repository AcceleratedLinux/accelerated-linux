#ifndef __DEBUG_H__
#define __DEBUG_H__

#if defined(LIBPCAPNG_DEBUG)
#define ERROR(...)  fprintf(stderr, __VA_ARGS__)
#else
#define ERROR(...)  do { } while (0)
#endif

#endif /* __DEBUG_H__ */

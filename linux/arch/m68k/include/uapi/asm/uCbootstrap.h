/* uCbootstrap.h:  Bootloader system call interface
 *
 * Copyright (C) 2001-2008 Arcturus Networks Inc.
 *               <www.ArcturusNetworks.com>   
 * Copyright (C) 1999 Rt-Control Inc.
 *
 * Release notes:
 * 7 Nov 2001 by Michael Durrant <mdurrant@uClinux.org>
 *            - moved old uCbootloader.h file from the 
 *              uCdimm platform directory into linux/asm-m68knommu/
 *            - combined the bootstd.h and flash.h file from uCsimm
 *              and uCdimm into a common uCbootloader.h file.
 */

#ifndef __UCBOOTSTRAP_H__
#define __UCBOOTSTRAP_H__

#define NR_BSC 23            /* last used bootloader system call */

#define __BN_reset        0  /* reset and start the bootloader */
#define __BN_test         1  /* tests the system call interface */
#define __BN_exec         2  /* executes a bootloader image */
#define __BN_exit         3  /* terminates a bootloader image */
#define __BN_program      4  /* program FLASH from a chain */
#define __BN_erase        5  /* erase sector(s) of FLASH */
#define __BN_open         6
#define __BN_write        7
#define __BN_read         8
#define __BN_close        9
#define __BN_mmap         10 /* map a file descriptor into memory */
#define __BN_munmap       11 /* remove a file to memory mapping */
#define __BN_gethwaddr    12 /* get the hardware address of my interfaces */
#define __BN_getserialnum 13 /* get the serial number of this board */
#define __BN_getbenv      14 /* get a bootloader envvar */
#define __BN_setbenv      15 /* get a bootloader envvar */
#define __BN_setpmask     16 /* set the protection mask */
#define __BN_readbenv     17 /* read environment variables */
#define __BN_flash_chattr_range		18
#define __BN_flash_erase_range		19
#define __BN_flash_write_range		20
#define __BN_ramload			21 /* load kernel into RAM and exec */
#define __BN_program2      22  /* program second FLASH from a chain */

#define __BN_test0         0x100  /* tests the system call interface */
#define __BN_test1         0x101  /* tests the system call interface */
#define __BN_test2         0x102  /* tests the system call interface */
#define __BN_test3         0x103  /* tests the system call interface */
#define __BN_test4         0x104  /* tests the system call interface */
#define __BN_test5         0x105  /* tests the system call interface */


/* Calling conventions compatible to (uC)linux/68k
 * We use simmilar macros to call into the bootloader as for uClinux
 */

#define __bsc_return(type, res) \
do { \
   if ((unsigned long)(res) >= (unsigned long)(-64)) { \
      /* let errno be a function, preserve res in %d0 */ \
      int __err = -(res); \
      errno = __err; \
      res = -1; \
   } \
   return (type)(res); \
} while (0)

#define _bsc0(type,name) \
type name(void) \
{ \
  long __res;						\
  __asm__ __volatile__ ("movel  %1, %%d0\n\t"           \
                        "trap   #2\n\t"                 \
                        "movel  %%d0, %0"               \
                        : "=g" (__res)                  \
                        : "i" (__BN_##name)             \
                        : "cc", "%d0");                 \
   __bsc_return(type,__res); \
}

#define _bsc1(type,name,atype,a) \
type name(atype a) \
{ \
  long __res;						\
  __asm__ __volatile__ ("movel  %2, %%d1\n\t"           \
                        "movel  %1, %%d0\n\t"           \
                        "trap   #2\n\t"                 \
                        "movel  %%d0, %0"               \
                        : "=g" (__res)                  \
                        : "i" (__BN_##name),            \
                          "g" ((long)a)                 \
                        : "cc", "%d0", "%d1");          \
   __bsc_return(type,__res); \
}

#define _bsc2(type,name,atype,a,btype,b) \
type name(atype a, btype b) \
{ \
  long __res;						\
  __asm__ __volatile__ ("movel  %3, %%d2\n\t"           \
                        "movel  %2, %%d1\n\t"           \
                        "movel  %1, %%d0\n\t"           \
                        "trap   #2\n\t"                 \
                        "movel  %%d0, %0"               \
                        : "=g" (__res)                  \
                        : "i" (__BN_##name),            \
                          "a" ((long)a),                \
                          "g" ((long)b)                 \
                        : "cc", "%d0", "%d1", "%d2");   \
   __bsc_return(type,__res); \
}

#define _bsc3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a, btype b, ctype c) \
{ \
  long __res;						\
  __asm__ __volatile__ ("movel  %4, %%d3\n\t"           \
                        "movel  %3, %%d2\n\t"           \
                        "movel  %2, %%d1\n\t"           \
                        "movel  %1, %%d0\n\t"           \
                        "trap   #2\n\t"                 \
                        "movel  %%d0, %0"               \
                        : "=g" (__res)                  \
                        : "i" (__BN_##name),            \
                          "a" ((long)a),                \
                          "a" ((long)b),                \
                          "g" ((long)c)                 \
                        : "cc", "%d0", "%d1", "%d2", 	\
			   "%d3");                      \
   __bsc_return(type,__res); \
}

#define _bsc4(type,name,atype,a,btype,b,ctype,c,dtype,d) \
type name(atype a, btype b, ctype c, dtype d) \
{ \
  long __res;						\
  __asm__ __volatile__ ("movel  %5, %%d4\n\t"           \
                        "movel  %4, %%d3\n\t"           \
                        "movel  %3, %%d2\n\t"           \
                        "movel  %2, %%d1\n\t"           \
                        "movel  %1, %%d0\n\t"           \
                        "trap   #2\n\t"                 \
                        "movel  %%d0, %0"               \
                        : "=g" (__res)                  \
                        : "i" (__BN_##name),            \
                          "a" ((long)a),                \
                          "a" ((long)b),                \
                          "a" ((long)c),                \
                          "g" ((long)d)                 \
                        : "cc", "%d0", "%d1", "%d2", 	\
			   "%d3", "%d4");               \
   __bsc_return(type,__res); \
}

#define _bsc5(type,name,atype,a,btype,b,ctype,c,dtype,d,etype,e) \
type name(atype a, btype b, ctype c, dtype d, etype e)  \
{ \
  long __res;						\
  __asm__ __volatile__ ("movel  %6, %%d5\n\t"           \
                        "movel  %5, %%d4\n\t"           \
                        "movel  %4, %%d3\n\t"           \
                        "movel  %3, %%d2\n\t"           \
                        "movel  %2, %%d1\n\t"           \
                        "movel  %1, %%d0\n\t"           \
                        "trap   #2\n\t"                 \
                        "movel  %%d0, %0"               \
                        : "=g" (__res)                  \
                        : "i" (__BN_##name),            \
                          "a" ((long)a),                \
                          "a" ((long)b),                \
                          "a" ((long)c),                \
                          "a" ((long)d),                \
                          "g" ((long)e)                 \
                        : "cc", "%d0", "%d1", "%d2", 	\
			   "%d3", "%d4", "%d5");        \
   __bsc_return(type,__res); \
}

/* Command codes for the flash_command routine */
#define FLASH_SELECT    0       /* no command; just perform the mapping */
#define FLASH_RESET     1       /* reset to read mode */
#define FLASH_READ      1       /* reset to read mode, by any other name */
#define FLASH_AUTOSEL   2       /* autoselect (fake Vid on pin 9) */
#define FLASH_PROG      3       /* program a word */
#define FLASH_CERASE    4       /* chip erase */
#define FLASH_SERASE    5       /* sector erase */
#define FLASH_ESUSPEND  6       /* suspend sector erase */
#define FLASH_ERESUME   7       /* resume sector erase */
#define FLASH_UB        8       /* go into unlock bypass mode */
#define FLASH_UBPROG    9       /* program a word using unlock bypass */
#define FLASH_UBRESET   10      /* reset to read mode from unlock bypass mode */
#define FLASH_LASTCMD   10      /* used for parameter checking */

/* Return codes from flash_status */
#define STATUS_READY    0       /* ready for action */
#define STATUS_BUSY     1       /* operation in progress */
#define STATUS_ERSUSP   2       /* erase suspended */
#define STATUS_TIMEOUT  3       /* operation timed out */
#define STATUS_ERROR    4       /* unclassified but unhappy status */

/* manufacturer ID */
#define AMDPART   	0x01
#define TOSPART		0x98
#define SSTPART		0xbf
#define ATPART		0x1f

/* A list of 4 AMD device ID's - add others as needed */
#define ID_AM29DL800T   0x224A
#define ID_AM29DL800B   0x22CB
#define ID_AM29LV800T   0x22DA
#define ID_AM29LV800B   0x225B
#define ID_AM29LV160B   0x2249
#define ID_AM29LV160T   0x22C4
#define ID_TC58FVT160	0x00C2
#define ID_TC58FVB160	0x0043
#define ID_TC58FVB160X2	0x00430043
#define ID_AM29LV400B   0x22BA
#define ID_AT49BV1614   0x00C0
#define ID_SST39VF160   0x2782
/* definition for arm7tdni */
#define ID_AT27LV1024	0x0000
#define ID_AT29LV1024	0x0001

#define SECTOR_DIRTY   0x01
#define SECTOR_ERASED  0x02
#define SECTOR_PROTECT 0x04

#define PGM_ERASE_FIRST 0x0001
#define PGM_RESET_AFTER 0x0002
#define PGM_EXEC_AFTER  0x0004
#define PGM_HALT_AFTER  0x0008
#define PGM_DEBUG       0x0010

/* pmask values: */
#define FEF_BOOT_READ   0x0002	/* Read permission for bootloader */
#define FEF_BOOT_WRITE  0x0004	/* Write permission for bootloader */
#define FEF_SUPER_READ  0x0020	/* Read permission for supervisor */
#define FEF_SUPER_WRITE 0x0040	/* Write permission for supervisor */
#define FEF_USER_READ   0x0200	/* Read permission for user */
#define FEF_USER_WRITE  0x0400	/* Write permission for user */

/* an mnode points at 4k pages of data through an offset table. */
#ifndef _memnode_struct_
#define _memnode_struct_

typedef struct _memnode {
  int len;
  int *offset;
} mnode_t;
#endif

/*
 * uCbootloader header specification
 */

#if !defined (_UCHEADER_H_)
#define _UCHEADER_H_
#define _UCHEADER_VERSION_ 0.2

typedef struct {
        unsigned char magic[8];     /* magic number "uCimage\0" */
        int           header_size;  /* after which data begins */
        int           data_size;    /* size of image in bytes */
        char          datecode[12]; /* output of 'date -I': "yyyy-mm-dd" */
        unsigned char md5sum[16];   /* binary md5sum of data */
        char          name[128];    /* filename or ID */
        int           bit32sum;     /* 32 bit checksum for data fields */
        char          partition;    /* partition number */
        char          padding[79];  /* pad to 256 bytes */
} uCimage_header;

#define UCHEADER_MAGIC "uCimage\0" /* including one terminating zero */
#endif /* !defined (_UCHEADER_H_) */

#endif /* __UCBOOTSTRAP_H__ */

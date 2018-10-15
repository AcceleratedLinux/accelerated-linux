/* ---------------------------------------------------------------------------
          Copyright (c) 2006-2008 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    target.h - Target platform functions header

    Author      Date        Description
    THa         12/10/06    Created file.
   ---------------------------------------------------------------------------
*/


#ifndef __TARGET_H
#define __TARGET_H


#define HardwareDisableInterruptSync( pHardware )  \
    HardwareDisableInterrupt( pHardware )


#if defined(_WIN32) || defined(DEF_LINUX)


/* -------------------------------------------------------------------------- *
 *                               WIN32 OS                                     *
 * -------------------------------------------------------------------------- */

#ifdef _WIN32

#define FAR
#ifdef UNDER_CE
#define NEWLINE    "\r\n"
#else
#define NEWLINE    "\n"
#endif

typedef unsigned char   BYTE;
typedef unsigned char   UINT8;

#ifdef NDIS_MINIPORT_DRIVER
#include <ndis.h>


#undef HardwareDisableInterruptSync
void HardwareDisableInterruptSync ( void* pHardware );


#define DBG_PRINT  DbgPrint


#define CPU_TO_LE32( x )  ( x )
#define LE32_TO_CPU( x )  ( x )


#define HW_READ_BYTE( phwi, addr, data )                                    \
    *( data ) = *(( volatile UCHAR* )(( phwi )->m_pVirtualMemory + addr ))

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    *(( volatile UCHAR* )(( phwi )->m_pVirtualMemory + addr )) = ( UCHAR )( data )

#define HW_READ_WORD( phwi, addr, data )                                    \
    *( data ) = *(( volatile USHORT* )(( phwi )->m_pVirtualMemory + addr ))

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    *(( volatile USHORT* )(( phwi )->m_pVirtualMemory + addr )) = ( USHORT )( data )

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *( data ) = *(( volatile UINT* )(( phwi )->m_pVirtualMemory + addr ))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    *(( volatile UINT* )(( phwi )->m_pVirtualMemory + addr )) = ( UINT )( data )

BOOLEAN PciReadConfig ( PVOID, int, UINT* );
BOOLEAN PciReadConfigByte ( PVOID, int, PUCHAR );
BOOLEAN PciReadConfigWord ( PVOID, int, PUSHORT );
BOOLEAN PciWriteConfig ( PVOID, int, UINT );
BOOLEAN PciWriteConfigByte ( PVOID, int, UCHAR );
BOOLEAN PciWriteConfigWord ( PVOID, int, USHORT );

#define HW_PCI_READ_BYTE( phwi, addr, data )                                \
    PciReadConfigByte(( phwi )->m_pPciCfg, addr, data )

#define HW_PCI_READ_WORD( phwi, addr, data )                                \
    PciReadConfigWord(( phwi )->m_pPciCfg, addr, data )

#define HW_PCI_READ_DWORD( phwi, addr, data )                               \
    PciReadConfig(( phwi )->m_pPciCfg, addr, data )

#define HW_PCI_WRITE_BYTE( phwi, addr, data )                               \
    PciWriteConfigByte(( phwi )->m_pPciCfg, addr, data )

#define HW_PCI_WRITE_WORD( phwi, addr, data )                               \
    PciWriteConfigWord(( phwi )->m_pPciCfg, addr, data )

#define HW_PCI_WRITE_DWORD( phwi, addr, data )                              \
    PciWriteConfig(( phwi )->m_pPciCfg, addr, data )


#define MOVE_MEM( dest, src, len )                                          \
    NdisMoveMemory( dest, src, len )

#else  /* #ifdef NDIS_MINIPORT_DRIVER */
#include <conio.h>
#include <stdio.h>
#include <string.h>


typedef unsigned char   UCHAR;
typedef unsigned char*  PUCHAR;
typedef unsigned short  USHORT;
typedef unsigned short* PUSHORT;
typedef unsigned long   ULONG;
typedef unsigned long*  PULONG;
typedef unsigned long   ULONGLONG;
typedef unsigned long*  PULONGLONG;

typedef int             BOOLEAN;
typedef int*            PBOOLEAN;


#define FALSE  0
#define TRUE   1


#define DBG_PRINT  printf


#define CPU_TO_LE32( x )  ( x )
#define LE32_TO_CPU( x )  ( x )


#define HW_READ_BUFFER( phwi, addr, data, len )                             \
    HardwareReadBuffer( phwi, addr, ( PULONG ) data, ( len )  )
#if (0)
    HardwareReadBuffer( phwi, addr, ( PUSHORT ) data, ( len ) >> 1 )
#endif

#define HW_WRITE_BUFFER( phwi, addr, data, len )                            \
    HardwareWriteBuffer( phwi, addr, ( PULONG ) data, ( len ) )
#if (0)
    HardwareWriteBuffer( phwi, addr, ( PUSHORT ) data, ( len ) >> 1 )
#endif

#define HW_READ_BYTE( phwi, addr, data )                                    \
    *data = _inp(( USHORT )(( phwi )->m_ulVIoAddr + addr ))

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    _outp(( USHORT )(( phwi )->m_ulVIoAddr + addr ), data )

#define HW_READ_WORD( phwi, addr, data )                                    \
    *data = _inpw(( USHORT )(( phwi )->m_ulVIoAddr + addr ))

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    _outpw(( USHORT )(( phwi )->m_ulVIoAddr + addr ), ( USHORT )( data ))

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *data = _inpd(( USHORT )(( phwi )->m_ulVIoAddr + addr ))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    _outpd(( USHORT )(( phwi )->m_ulVIoAddr + addr ), data )

#define MOVE_MEM( dest, src, len )                                          \
    memcpy( dest, src, len )

#endif /* #ifdef NDIS_MINIPORT_DRIVER */
#endif /* #ifdef _WIN32 */

#define MEM_READ_BYTE( phwi, addr, data )                                   \
    *data = *(( PUCHAR )(( phwi )->m_pVirtualMemory + addr ))

#define MEM_WRITE_BYTE( phwi, addr, data )                                  \
    *(( PUCHAR )(( phwi )->m_pVirtualMemory + addr )) = ( UCHAR )( data )

#define MEM_READ_WORD( phwi, addr, data )                                   \
    *data = *(( PUSHORT )(( phwi )->m_pVirtualMemory + addr ))

#define MEM_WRITE_WORD( phwi, addr, data )                                  \
    *(( PUSHORT )(( phwi )->m_pVirtualMemory + addr )) = ( USHORT )( data )

#define MEM_READ_DWORD( phwi, addr, data )                                  \
    *data = *(( PULONG )(( phwi )->m_pVirtualMemory + addr ))

#define MEM_WRITE_DWORD( phwi, addr, data )                                 \
    *(( PULONG )(( phwi )->m_pVirtualMemory + addr )) = ( ULONG )( data )


/* -------------------------------------------------------------------------- *
 *                             LINUX OS                                       *
 * -------------------------------------------------------------------------- */

#ifdef DEF_LINUX
#include <linux/mm.h>
#include <linux/pci.h>
#include <asm/io.h>
#include <mach/platform.h>


typedef unsigned char   BYTE;
typedef unsigned char   UCHAR;
typedef unsigned char   UINT8;
typedef unsigned char*  PUCHAR;
typedef unsigned short  WORD;
typedef unsigned short  USHORT;
typedef unsigned short* PUSHORT;
typedef unsigned int    UINT;
typedef unsigned int*   PUINT;
typedef unsigned int    UINT32;
typedef unsigned int    DWORD;
typedef unsigned long   ULONG;
typedef unsigned long*  PULONG;
typedef void*           PVOID;

typedef int             BOOLEAN;
typedef int*            PBOOLEAN;

#if 0
typedef unsigned long long  ULONGLONG;
typedef unsigned long long* PULONGLONG;
#else
typedef unsigned long  ULONGLONG;
typedef unsigned long* PULONGLONG;
#endif


#define MIO_DWORD( x )  *(( volatile unsigned int* )( x ))
#define MIO_WORD( x )   *(( volatile unsigned short* )( x ))
#define MIO_BYTE( x )   *(( volatile unsigned char* )( x ))


#define FAR

#define FALSE  0
#define TRUE   1


#define NEWLINE    "\n"
#define DBG_PRINT  printk


#define CPU_TO_LE32( x )  cpu_to_le32( x )
#define CPU_TO_LE16( x )  cpu_to_le16( x )
#define LE32_TO_CPU( x )  le32_to_cpu( x )
#define LE16_TO_CPU( x )  le16_to_cpu( x )


#define HW_READ_BYTE( phwi, addr, data )                                    \
    *data = readb(( void* )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    writeb( data, ( void* )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_READ_WORD( phwi, addr, data )                                    \
    *data = LE16_TO_CPU( readw(( void* )(( phwi )->m_pVirtualMemory +       \
        ( addr ))))

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    writew( CPU_TO_LE16(( USHORT )( data )),                                \
        ( void* )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *data = LE32_TO_CPU( readl(( void* )(( phwi )->m_pVirtualMemory +       \
        ( addr ))))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    writel( CPU_TO_LE32( data ),                                            \
        ( void* )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_PCI_READ_BYTE( phwi, addr, data )                                \
    pci_read_config_byte(( struct pci_dev* )( phwi )->m_pPciCfg,            \
        addr, data )

#define HW_PCI_READ_WORD( phwi, addr, data )                                \
    pci_read_config_word(( struct pci_dev* )( phwi )->m_pPciCfg,            \
        addr, data )

#define HW_PCI_READ_DWORD( phwi, addr, data )                               \
    pci_read_config_dword(( struct pci_dev* )( phwi )->m_pPciCfg,           \
        addr, data )

#define HW_PCI_WRITE_BYTE( phwi, addr, data )                               \
    pci_write_config_byte(( struct pci_dev* )( phwi )->m_pPciCfg,           \
        addr, data )

#define HW_PCI_WRITE_WORD( phwi, addr, data )                               \
    pci_write_config_word(( struct pci_dev* )( phwi )->m_pPciCfg,           \
        addr, data )

#define HW_PCI_WRITE_DWORD( phwi, addr, data )                              \
    pci_write_config_dword(( struct pci_dev* )( phwi )->m_pPciCfg,          \
        addr, data )

#define MOVE_MEM( dest, src, len )                                          \
    memcpy( dest, src, len )

#endif /* #ifdef DEF_LINUX */

#endif /* #if defined(_WIN32) || defined(DEF_LINUX) */


/* -------------------------------------------------------------------------- *
 *                             QC TEST                                        *
 * -------------------------------------------------------------------------- */

#ifdef KS_ARM

#define cpu_to_le16(x)  ((unsigned short)(x))
#define cpu_to_le32(x)  ((unsigned long)(x))
#define le16_to_cpu(x)  ((unsigned short)(x))
#define le32_to_cpu(x)  ((unsigned long)(x))


/* asm/io.h */
#define __arch_getb(a)			(*(volatile unsigned char *)(a))
#define __arch_getw(a)			(*(volatile unsigned short *)(a))
#define __arch_getl(a)			(*(volatile unsigned int  *)(a))

#define __arch_putb(v,a)		(*(volatile unsigned char *)(a) = (v))
#define __arch_putw(v,a)		(*(volatile unsigned short *)(a) = (v))
#define __arch_putl(v,a)		(*(volatile unsigned int  *)(a) = (v))


#define __raw_writeb(v,a)		__arch_putb(v,a)
#define __raw_writew(v,a)		__arch_putw(v,a)
#define __raw_writel(v,a)		__arch_putl(v,a)

#define __raw_readb(a)			__arch_getb(a)
#define __raw_readw(a)			__arch_getw(a)
#define __raw_readl(a)			__arch_getl(a)


/* asm/arch/io.h */
#define __io(a)				(a)
#define __mem_pci(a)			((unsigned long)(a))


#ifdef __io
#define outb(v,p)			__raw_writeb(v,__io(p))
#define outw(v,p)			__raw_writew(cpu_to_le16(v),__io(p))
#define outl(v,p)			__raw_writel(cpu_to_le32(v),__io(p))

#define inb(p)	({ unsigned int __v = __raw_readb(__io(p)); __v; })
#define inw(p)	({ unsigned int __v = le16_to_cpu(__raw_readw(__io(p))); __v; })
#define inl(p)	({ unsigned int __v = le32_to_cpu(__raw_readl(__io(p))); __v; })

#define outsb(p,d,l)			__raw_writesb(__io(p),d,l)
#define outsw(p,d,l)			__raw_writesw(__io(p),d,l)
#define outsl(p,d,l)			__raw_writesl(__io(p),d,l)

#define insb(p,d,l)			__raw_readsb(__io(p),d,l)
#define insw(p,d,l)			__raw_readsw(__io(p),d,l)
#define insl(p,d,l)			__raw_readsl(__io(p),d,l)
#endif


/*
 * If this architecture has PCI memory IO, then define the read/write
 * macros.  These should only be used with the cookie passed from
 * ioremap.
 */
#ifdef __mem_pci

#define readb(c) ({ unsigned int __v = __raw_readb(__mem_pci(c)); __v; })
#define readw(c) ({ unsigned int __v = le16_to_cpu(__raw_readw(__mem_pci(c))); __v; })
#define readl(c) ({ unsigned int __v = le32_to_cpu(__raw_readl(__mem_pci(c))); __v; })

#define writeb(v,c)		__raw_writeb(v,__mem_pci(c))
#define writew(v,c)		__raw_writew(cpu_to_le16(v),__mem_pci(c))
#define writel(v,c)		__raw_writel(cpu_to_le32(v),__mem_pci(c))

#endif	/* __mem_pci */


#define HZ  100


extern volatile unsigned long jiffies;


#include "platform.h"


typedef unsigned char   BYTE;
typedef unsigned char   UCHAR;
typedef unsigned char   UINT8;
typedef unsigned char*  PUCHAR;
typedef unsigned short  WORD;
typedef unsigned short  USHORT;
typedef unsigned short* PUSHORT;
typedef unsigned int    DWORD;
typedef unsigned int    UINT32;
typedef unsigned long   ULONG;
typedef unsigned long*  PULONG;
typedef void*           PVOID;

typedef int             BOOLEAN;
typedef int*            PBOOLEAN;

typedef unsigned long long  ULONGLONG;
typedef unsigned long long* PULONGLONG;


#define FAR

#ifndef NULL
#define NULL  (( void* ) 0 )
#endif

#define FALSE  0
#define TRUE   1


#define NEWLINE    "\r\n"
#define DBG_PRINT  printf

void flush ( void );


#define CPU_TO_LE32( x )  cpu_to_le32( x )
#define LE32_TO_CPU( x )  le32_to_cpu( x )


void pci_read_config_byte ( PCIDevice_t* pdev, int offset, UCHAR* pbData );
void pci_read_config_word ( PCIDevice_t* pdev, int offset, USHORT* pwData );
void pci_read_config_dword ( PCIDevice_t* pdev, int offset, ULONG* pdwData );
void pci_write_config_byte ( PCIDevice_t* pdev, int offset, UCHAR bData );
void pci_write_config_word ( PCIDevice_t* pdev, int offset, USHORT wData );
void pci_write_config_dword ( PCIDevice_t* pdev, int offset, ULONG dwData );


#define HW_READ_BYTE( phwi, addr, data )                                    \
    *data = readb(( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_WRITE_BYTE( phwi, addr, data )                                   \
    writeb( data, ( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_READ_WORD( phwi, addr, data )                                    \
    *data = readw(( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_WRITE_WORD( phwi, addr, data )                                   \
    writew(( USHORT )( data ), ( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_READ_DWORD( phwi, addr, data )                                   \
    *data = readl(( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_WRITE_DWORD( phwi, addr, data )                                  \
    writel( data, ( unsigned )(( phwi )->m_pVirtualMemory + ( addr )))

#define HW_PCI_READ_BYTE( phwi, addr, data )                                \
    pci_read_config_byte(( PCIDevice_t* )( phwi )->m_pPciCfg,               \
        addr, data )

#define HW_PCI_READ_WORD( phwi, addr, data )                                \
    pci_read_config_word(( PCIDevice_t* )( phwi )->m_pPciCfg,               \
        addr, data )

#define HW_PCI_READ_DWORD( phwi, addr, data )                               \
    pci_read_config_dword(( PCIDevice_t* )( phwi )->m_pPciCfg,              \
        addr, data )

#define HW_PCI_WRITE_BYTE( phwi, addr, data )                               \
    pci_write_config_byte(( PCIDevice_t* )( phwi )->m_pPciCfg,              \
        addr, data )

#define HW_PCI_WRITE_WORD( phwi, addr, data )                               \
    pci_write_config_word(( PCIDevice_t* )( phwi )->m_pPciCfg,              \
        addr, data )

#define HW_PCI_WRITE_DWORD( phwi, addr, data )                              \
    pci_write_config_dword(( PCIDevice_t* )( phwi )->m_pPciCfg,             \
        addr, data )

#define MOVE_MEM( dest, src, len )                                          \
    memcpy( dest, src, len )


#include "util.h"
#endif


#ifndef ASSERT
#ifdef DBG
#define ASSERT( f )  \
    if ( !( f ) ) {  \
        DBG_PRINT( "assert %s at %d in %s\n", #f, __LINE__, __FILE__ );  \
    }

#else
#define ASSERT( f )
#endif
#endif


void DelayMicrosec (
    UINT microsec );

void DelayMillisec (
    UINT millisec );

void PrintMacAddress (
    PUCHAR bAddr );

void PrintIpAddress (
    UINT32 IpAddr );

void PrintPacketData (
    UCHAR  *data,
    int    len,
    UINT32 port,
    UINT32 flag );

#endif

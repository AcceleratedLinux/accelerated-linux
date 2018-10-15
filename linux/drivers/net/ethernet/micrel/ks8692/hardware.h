/* ---------------------------------------------------------------------------
          Copyright (c) 2006-2008 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    hardware.h - Target independent hardware header

    Author  Date      Version  Description
    THa     12/10/06  0.0.1    Created file.
    THa     08/29/07           Update for Linux 2.6.21.
    THa     11/01/07           Change number of receive descriptors to 64 for
                               better UDP receive throughput.
    THa     11/27/07           Correct huge frame support.
    THa     11/30/07           Correct MIB counter reporting.
    THa     05/30/08           Update for rev. B KSZ8692.
    THa     07/17/08           Add Access Control List (ACL) support.
    THa     07/28/08           Use UINT instead of ULONG.
                               Workaround transmit failure problem.
   ---------------------------------------------------------------------------
*/


#ifndef __HARDWARE_H
#define __HARDWARE_H

#include "target.h"

#ifdef CONFIG_CENTAUR2
#define CONFIG_PEGASUS
#endif

#ifdef CONFIG_PEGASUS
#define KS_READ_REG  KS8692_READ_REG
#define KS_WRITE_REG  KS8692_WRITE_REG

#else
#define KS_IO_BASE  KS8695_IO_BASE
#ifndef KS_VIO_BASE
#define KS_VIO_BASE  IO_ADDRESS( KS_IO_BASE )
#endif

#define KS8695_READ_REG( x )  __raw_readl( KS_VIO_BASE + ( x ))
#define KS8695_WRITE_REG( x, d )  __raw_writel( d, KS_VIO_BASE + ( x ))

#define KS_WAN_DMA_TX  KS8695_WAN_DMA_TX
#define KS_LAN_DMA_TX  KS8695_LAN_DMA_TX
#define KS_INT_ENABLE  KS8695_INT_ENABLE
#define KS_INT_STATUS  KS8695_INT_STATUS

#define KS_READ_REG  KS8695_READ_REG
#define KS_WRITE_REG  KS8695_WRITE_REG
#endif


#if 0
#define DBG
#endif
#ifdef DBG
#ifndef UNDER_CE
#define DEBUG_COUNTER
#define DEBUG_TIMEOUT
#endif

#if 0
#define DEBUG_INTERRUPT
#endif
#if 0
#define DEBUG_MEM
#endif
#if 1
#define DEBUG_DATA_HEADER
#endif
#if 0
#define DEBUG_RX
#endif
#if 0
#define DEBUG_RX_DATA
#if 0
#define DEBUG_RX_ALL
#undef DEBUG_DATA_HEADER
#endif
#endif
#if 0
#define DEBUG_TX
#endif
#if 0
#define DEBUG_TX_DATA
#endif
#if 0
#define DEBUG_RX_DESC
#endif
#if 0
#define DEBUG_RX_DESC_CHECK
#endif
#if 0
#define DEBUG_TX_DESC
#endif
#if 0
#define DEBUG_TX_DESC_CHECK
#endif
#endif

#if 0
#define INLINE
#endif

/* Enable to receive larger than 2000-byte frames. */
#if 0
#define RCV_HUGE_FRAME
#endif

/* Enable to receive larger than 2600-byte frames. */
#if 1
#define RCV_JUMBO_FRAME
#endif

#define SET_DEFAULT_LED  LED_SPEED_DUPLEX_ACT
/* Enable to change default LED mode. */
#if 0
#undef SET_DEFAULT_LED
#define SET_DEFAULT_LED  LED_SPEED_DUPLEX_LINK_ACT
#endif
#if 0
#undef SET_DEFAULT_LED
#define SET_DEFAULT_LED  LED_DUPLEX_10_1000
#endif

/* Enable to receive and check bad frames. */
#if 0
#define CHECK_RCV_ERRORS
#endif

/* Enable for better transmit performance. */
#if 1
#define SKIP_TX_INT
#define MAX_TX_HELD_SIZE  52000
#endif

#if 1
#define ALIGN_IP_HEADER
#endif
#ifdef ALIGN_IP_HEADER
#define SKB_RESERVED  2
#else
#define SKB_RESERVED  0
#if 0
#define USE_MULTIPLE_RX_DESC
#endif
#endif

/* THa  2008/07/24
   Enable to avoid transmit failure when switching from Gigabit to 100 Mbit.
*/
#if 1
#define AVOID_TX_FAILURE
#endif

#if 0
#define VERIFY_CSUM_GEN
#if 1
#define CSUM_VAL  0xEFBE
#else
#define CSUM_VAL  0
#endif
#if 1
#define VERIFY_NO_CSUM_GEN
#endif
#if 1
#define VERIFY_NO_IP_CSUM_GEN
#endif
#endif

#if 0
#define VERIFY_NO_CSUM_FRAGMENT
#endif

#ifndef VERIFY_NO_IP_CSUM_GEN
#if 0
#define VERIFY_IP_CSUM_GEN_BUG
#endif
#endif

#if 0
/* Verify hardware transmit loopback is working. */
#define VERIFY_LOOPBACK
#endif

#if 0
/* Verify multicast hash table filtering is working. */
#define VERIFY_MULTICAST
#endif

#if 0
#define TEST_MINIMUM_TX
#endif

#if 0
#define TEST_RX_LOOPBACK
#endif

/* -------------------------------------------------------------------------- */

#define MAC_ADDRESS_LENGTH  6

#define BASE_IO_RANGE  0x100

/* -------------------------------------------------------------------------- */

#ifdef NDIS_MINIPORT_DRIVER
#if defined( NDIS50_MINIPORT )  ||  defined( NDIS51_MINIPORT )
#include <pshpack1.h>

#else
#include <packon.h>
#endif
#endif


#if 0
#define LAN_MAX_OFFSET          0
#else
#define LAN_MAX_OFFSET          4
#endif

#define MAX_ETHERNET_BODY_SIZE  1500
#define ETHERNET_HEADER_SIZE    14

#define MAXIMUM_ETHERNET_PACKET_SIZE  \
    ( MAX_ETHERNET_BODY_SIZE + ETHERNET_HEADER_SIZE )

#define REGULAR_RX_BUF_SIZE     2000
#define WAN_MAX_RX_BUF_SIZE     2047
#define JUMBO_RX_BUF_SIZE       9216
#define JUMBO_MAX_RX_BUF_SIZE   ( 9500 - LAN_MAX_OFFSET )

#ifdef RCV_JUMBO_FRAME
#define LAN_MAX_RX_BUF_SIZE     JUMBO_MAX_RX_BUF_SIZE
#define LAN_REG_RX_BUF_SIZE     JUMBO_RX_BUF_SIZE
#else
#define LAN_MAX_RX_BUF_SIZE     ( 2600 - LAN_MAX_OFFSET )
#define LAN_REG_RX_BUF_SIZE     REGULAR_RX_BUF_SIZE
#endif

#ifdef CONFIG_KSZ8692VA
#define WAN_MAX_BUF_SIZE        LAN_MAX_RX_BUF_SIZE

/* RX descriptor length is masked at 0x400. */
#define LAN_MAX_BUF_SIZE        2044

#ifdef RCV_HUGE_FRAME
#define WAN_RX_BUF_SIZE         WAN_MAX_BUF_SIZE
/* LAN port cannot handle receiving more than 2047-byte frames. */
#define LAN_RX_BUF_SIZE         REGULAR_RX_BUF_SIZE
#else
#define WAN_RX_BUF_SIZE         LAN_REG_RX_BUF_SIZE
#define LAN_RX_BUF_SIZE         REGULAR_RX_BUF_SIZE
#endif
#define WAN_TX_BUF_SIZE         ( WAN_MAX_BUF_SIZE - 4 )
#define LAN_TX_BUF_SIZE         ( WAN_MAX_RX_BUF_SIZE - 4 )

#elif defined( CONFIG_KSZ8692VB )
#define WAN_MAX_BUF_SIZE        WAN_MAX_RX_BUF_SIZE
#define LAN_MAX_BUF_SIZE        LAN_MAX_RX_BUF_SIZE

#ifdef RCV_HUGE_FRAME
/* WAN port cannot handle receiving more than 2047-byte frames. */
#define WAN_RX_BUF_SIZE         REGULAR_RX_BUF_SIZE
#define LAN_RX_BUF_SIZE         LAN_MAX_BUF_SIZE
#else
#define WAN_RX_BUF_SIZE         REGULAR_RX_BUF_SIZE
#define LAN_RX_BUF_SIZE         LAN_REG_RX_BUF_SIZE
#endif
#define WAN_TX_BUF_SIZE         ( WAN_MAX_RX_BUF_SIZE - 4 )
#define LAN_TX_BUF_SIZE         ( LAN_MAX_BUF_SIZE - 4 )
#endif

#define NDIS_MAX_LOOKAHEAD      ( RX_BUF_SIZE - ETHERNET_HEADER_SIZE )

#define MAX_MULTICAST_LIST      32

#define HW_MULTICAST_SIZE       8


#define MAC_ADDR_ORDER( i )  ( MAC_ADDRESS_LENGTH - 1 - ( i ))


typedef enum
{
    MediaStateConnected,
    MediaStateDisconnected
} MEDIA_STATE;


typedef enum
{
    OID_COUNTER_UNKOWN,

    OID_COUNTER_FIRST,
    OID_COUNTER_DIRECTED_BYTES_XMIT = OID_COUNTER_FIRST, /* total bytes transmitted  */
    OID_COUNTER_DIRECTED_FRAMES_XMIT,    /* total packets transmitted */

    OID_COUNTER_BROADCAST_BYTES_XMIT,
    OID_COUNTER_BROADCAST_FRAME_XMIT,

    OID_COUNTER_DIRECTED_BYTES_RCV,      /* total bytes received   */
    OID_COUNTER_DIRECTED_FRAMES_RCV,     /* total packets received */
    OID_COUNTER_BROADCAST_BYTES_RCV,
    OID_COUNTER_BROADCAST_FRAMES_RCV,    /* total broadcast packets received (RXSR: RXBF)                */
    OID_COUNTER_MULTICAST_FRAMES_RCV,    /* total multicast packets received (RXSR: RXMF) or (RDSE0: MF) */
    OID_COUNTER_UNICAST_FRAMES_RCV,      /* total unicast packets received   (RXSR: RXUF)                */

    OID_COUNTER_XMIT_ERROR,              /* total transmit errors */
    OID_COUNTER_XMIT_LATE_COLLISION,     /* transmit Late Collision (TXSR: TXLC) */
    OID_COUNTER_XMIT_MORE_COLLISIONS,    /* transmit Maximum Collision (TXSR: TXMC) */
    OID_COUNTER_XMIT_UNDERRUN,           /* transmit Underrun (TXSR: TXUR) */
    OID_COUNTER_XMIT_ALLOC_FAIL,         /* transmit fail because no enought memory in the Tx Packet Memory */
    OID_COUNTER_XMIT_DROPPED,            /* transmit packet drop because no buffer in the host memory */
    OID_COUNTER_XMIT_INT_UNDERRUN,       /* transmit underrun from interrupt status (ISR: TXUIS) */
    OID_COUNTER_XMIT_INT_STOP,           /* transmit DMA MAC process stop from interrupt status (ISR: TXPSIE) */
    OID_COUNTER_XMIT_INT,                /* transmit Tx interrupt status (ISR: TXIE) */

    OID_COUNTER_RCV_ERROR,               /* total receive errors */
    OID_COUNTER_RCV_ERROR_CRC,           /* receive packet with CRC error (RXSR: RXCE) or (RDSE0: CE) */
    OID_COUNTER_RCV_ERROR_MII,           /* receive MII error (RXSR: RXMR) or (RDSE0: RE) */
    OID_COUNTER_RCV_ERROR_TOOLONG,       /* receive frame too long error (RXSR: RXTL) or (RDSE0: TL)  */
    OID_COUNTER_RCV_ERROR_RUNT,          /* receive Runt frame error (RXSR: RXRF) or (RDSE0: RF)  */
    OID_COUNTER_RCV_INVALID_FRAME,       /* receive invalid frame (RXSR: RXFV) */
    OID_COUNTER_RCV_ERROR_IP,            /* receive frame with IP checksum error  (RDSE0: IPE) */
    OID_COUNTER_RCV_ERROR_TCP,           /* receive frame with TCP checksum error (RDSE0: TCPE) */
    OID_COUNTER_RCV_ERROR_UDP,           /* receive frame with UDP checksum error (RDSE0: UDPE) */
    OID_COUNTER_RCV_NO_BUFFER,           /* receive failed on memory allocation for the incoming frames from interrupt status (ISR: RXOIS). */
    OID_COUNTER_RCV_DROPPED,             /* receive packet drop because no buffer in the host memory */
    OID_COUNTER_RCV_INT_ERROR,           /* receive error from interrupt status (ISR: RXEFIE) */
    OID_COUNTER_RCV_INT_STOP,            /* receive DMA MAC process stop from interrupt status (ISR: RXPSIE) */
    OID_COUNTER_RCV_INT,                 /* receive Rx interrupt status (ISR: RXIE) */

    OID_COUNTER_XMIT_OK,
    OID_COUNTER_RCV_OK,

    OID_COUNTER_RCV_ERROR_LEN,

    OID_COUNTER_LAST
} EOidCounter;


enum
{
    COUNT_BAD_FIRST,
    COUNT_BAD_ALLOC = COUNT_BAD_FIRST,
    COUNT_BAD_CMD,
    COUNT_BAD_CMD_BUSY,
    COUNT_BAD_CMD_INITIALIZE,
    COUNT_BAD_CMD_MEM_ALLOC,
    COUNT_BAD_CMD_RESET,
    COUNT_BAD_CMD_WRONG_CHIP,
    COUNT_BAD_COPY_DOWN,
    COUNT_BAD_RCV_FRAME,
    COUNT_BAD_RCV_PACKET,
    COUNT_BAD_SEND,
    COUNT_BAD_SEND_DIFF,
    COUNT_BAD_SEND_PACKET,
    COUNT_BAD_SEND_ZERO,
    COUNT_BAD_XFER_ZERO,
    COUNT_BAD_LAST
};


enum
{
    COUNT_GOOD_FIRST,
    COUNT_GOOD_CMD_RESET = COUNT_GOOD_FIRST,
    COUNT_GOOD_CMD_RESET_MMU,
    COUNT_GOOD_COPY_DOWN_ODD,
    COUNT_GOOD_INT,
    COUNT_GOOD_INT_LOOP,
    COUNT_GOOD_INT_ALLOC,
    COUNT_GOOD_INT_RX,
    COUNT_GOOD_INT_RX_EARLY,
    COUNT_GOOD_INT_RX_OVERRUN,
    COUNT_GOOD_INT_TX,
    COUNT_GOOD_INT_TX_EMPTY,
    COUNT_GOOD_NEXT_PACKET,
    COUNT_GOOD_NO_NEXT_PACKET,
    COUNT_GOOD_RCV_COMPLETE,
    COUNT_GOOD_RCV_DISCARD,
    COUNT_GOOD_RCV_NOT_DISCARD,
    COUNT_GOOD_RCV_CNT_1,
    COUNT_GOOD_RCV_CNT_2,
    COUNT_GOOD_RCV_CNT_3,
    COUNT_GOOD_SEND_PACKET,
    COUNT_GOOD_SEND_QUEUE,
    COUNT_GOOD_SEND_ZERO,
    COUNT_GOOD_XFER_ZERO,
    COUNT_GOOD_LAST
};


enum
{
    WAIT_DELAY_FIRST,
    WAIT_DELAY_PHY_RESET = WAIT_DELAY_FIRST,
    WAIT_DELAY_AUTO_NEG,
    WAIT_DELAY_MEM_ALLOC,
    WAIT_DELAY_CMD_BUSY,
    WAIT_DELAY_LAST
};


#if 0
#define CHECK_OVERRUN
#endif
#ifdef DBG
#if 1
#define DEBUG_OVERRUN
#endif
#endif


#define DESC_ALIGNMENT              16
#define BUFFER_ALIGNMENT            8


#define NUM_OF_RX_DESC  64
#define NUM_OF_TX_DESC  64


typedef struct
{
    UINT32 wFrameLen     : 14;

#ifdef CONFIG_KSZ8692VA
    UINT32 ulReserved1   : 1;

#else
    UINT32 fHighPriority : 1;
#endif
    UINT32 fFrameType    : 1;
    UINT32 fErrCRC       : 1;
    UINT32 fErrRunt      : 1;
    UINT32 fErrTooLong   : 1;
    UINT32 fErrPHY       : 1;
    UINT32 bAligned      : 2;

#ifdef CONFIG_KSZ8692VA
    UINT32 fHighPriority : 1;
    UINT32 ulReserved2   : 1;
    UINT32 fMulticast    : 1;
    UINT32 fError        : 1;
    UINT32 fCsumErrUDP   : 1;
    UINT32 fCsumErrTCP   : 1;
    UINT32 fCsumErrIP    : 1;

#else
    UINT32 fCsumCause    : 2;
    UINT32 fCsumNotDone  : 1;
    UINT32 fCsumError    : 3;
    UINT32 fError        : 1;
#endif
    UINT32 fLastDesc     : 1;
    UINT32 fFirstDesc    : 1;
    UINT32 fHWOwned      : 1;
} TDescRxStat;

typedef struct
{
    UINT32 ulReserved1   : 31;
    UINT32 fHWOwned      : 1;
} TDescTxStat;

typedef struct
{
    UINT32 wBufSize      : 14;
    UINT32 ulReserved3   : 11;
    UINT32 fEndOfRing    : 1;
    UINT32 ulReserved4   : 6;
} TDescRxBuf;

typedef struct
{
    UINT32 wBufSize      : 14;
    UINT32 ulReserved3   : 6;

#ifdef CONFIG_KSZ8692VA
    UINT32 ulReserved4   : 5;

#else
    UINT32 ulReserved4   : 4;
    UINT32 fCsumGenICMP  : 1;
#endif
    UINT32 fEndOfRing    : 1;
    UINT32 fCsumGenUDP   : 1;
    UINT32 fCsumGenTCP   : 1;
    UINT32 fCsumGenIP    : 1;
    UINT32 fLastSeg      : 1;
    UINT32 fFirstSeg     : 1;
    UINT32 fInterrupt    : 1;
} TDescTxBuf;

typedef union
{
    TDescRxStat rx;
    TDescTxStat tx;
    UINT32      ulData;
} TDescStat;

typedef union
{
    TDescRxBuf rx;
    TDescTxBuf tx;
    UINT32     ulData;
} TDescBuf;

typedef struct
{
    TDescStat Control;
    TDescBuf  BufSize;
    UINT32    ulBufAddr;
    UINT32    ulNextPtr;
} THw_Desc, *PTHw_Desc;


typedef struct
{
    TDescStat Control;
    TDescBuf  BufSize;

    /* Current buffers size value in hardware descriptor. */
    UINT32    ulBufSize;
} TSw_Desc, *PTSw_Desc;


typedef struct _Desc
{
    /* Hardware descriptor pointer to uncached physical memory. */
    PTHw_Desc     phw;

    /* Cached memory to hold hardware descriptor values for manipulation. */
    TSw_Desc      sw;

    /* Operating system dependent data structure to hold physical memory buffer
       allocation information.
    */
    PVOID         pReserved;

#ifdef CHECK_OVERRUN
    PTHw_Desc     pCheck;
#endif
} TDesc, *PTDesc;


typedef struct
{
    /* First descriptor in the ring. */
    PTDesc    pRing;

    /* Current descriptor being manipulated. */
    PTDesc    pCurrent;

    /* First hardware descriptor in the ring. */
    PTHw_Desc phwRing;

    /* The physical address of the first descriptor of the ring. */
    UINT32    ulRing;

    int       nSize;

    /* Number of descriptors allocated. */
    int       cnAlloc;

    /* Number of descriptors available for use. */
    int       cnAvail;

    /* Index for last descriptor released to hardware .*/
    int       iLast;

    /* Index for next descriptor available for use. */
    int       iNext;

    /* Mask for index wrapping. */
    int       iMax;
} TDescInfo, *PTDescInfo;


#ifdef CONFIG_KSZ8692VB
#define TOTAL_ACL_NUM  32

typedef struct {
    UINT       data;
    UINT       mask;
    UCHAR      mac[ 6 ];
    UCHAR      offset;
    UCHAR      protocol;
    USHORT     port;
    USHORT     reserved;
    UCHAR      ip4_addr[ 4 ];
    UCHAR      ip4_mask[ 4 ];
    UCHAR      ip6_addr[ 16 ];
    UCHAR      ip6_mask[ 16 ];
    UINT       acl      : 8;
    UINT       mode     : 8;
    UINT       enable   : 1;
    UINT       filter   : 1;
    UINT       priority : 1;
    UINT       changed  : 1;
} TAclInfo, *PTAclInfo;
#endif


#define RX_MIB_COUNTER_NUM      20

#ifdef CONFIG_KSZ8692VA
#define WAN_RX_MIB_COUNTER_NUM  22
#define LAN_RX_MIB_COUNTER_NUM  20

#define PORT_COUNTER_NUM        ( WAN_RX_MIB_COUNTER_NUM + TX_MIB_COUNTER_NUM )

#elif defined( CONFIG_KSZ8692VB )
#define WAN_RX_MIB_COUNTER_NUM  21
#define LAN_RX_MIB_COUNTER_NUM  22

#define PORT_COUNTER_NUM        ( LAN_RX_MIB_COUNTER_NUM + TX_MIB_COUNTER_NUM )
#endif
#define TX_MIB_COUNTER_NUM      11

#define WAN_PORT_COUNTER_NUM    ( WAN_RX_MIB_COUNTER_NUM + TX_MIB_COUNTER_NUM )
#define LAN_PORT_COUNTER_NUM    ( LAN_RX_MIB_COUNTER_NUM + TX_MIB_COUNTER_NUM )

#define TOTAL_PORT_COUNTER_NUM  ( PORT_COUNTER_NUM + 2 )

#define MIB_COUNTER_RX                   0x00
#define MIB_COUNTER_RX_UNDERSIZE         0x01
#define MIB_COUNTER_RX_FRAGMENT          0x02
#define MIB_COUNTER_RX_OVERSIZE          0x03
#define MIB_COUNTER_RX_JABBER            0x04
#define MIB_COUNTER_RX_SYMBOL_ERR        0x05
#define MIB_COUNTER_RX_CRC_ERR           0x06
#define MIB_COUNTER_RX_ALIGNMENT_ERR     0x07
#define MIB_COUNTER_RX_CTRL_8808         0x08
#define MIB_COUNTER_RX_PAUSE             0x09
#define MIB_COUNTER_RX_BROADCAST         0x0A
#define MIB_COUNTER_RX_MULTICAST         0x0B
#define MIB_COUNTER_RX_UNICAST           0x0C
#define MIB_COUNTER_RX_OCTET_64          0x0D
#define MIB_COUNTER_RX_OCTET_65_127      0x0E
#define MIB_COUNTER_RX_OCTET_128_255     0x0F
#define MIB_COUNTER_RX_OCTET_256_511     0x10
#define MIB_COUNTER_RX_OCTET_512_1023    0x11
#define MIB_COUNTER_RX_OCTET_1024_1521   0x12
#define MIB_COUNTER_RX_OCTET_1522_2000   0x13

#define MIB_COUNTER_RX_OCTET_2001_2047   0x14

#define MIB_COUNTER_RX_OCTET_2001_9216   0x14
#define MIB_COUNTER_RX_OCTET_9217_9500   0x15

#define MIB_COUNTER_TX                   ( pHardware->m_nTxCounterStart )

#define MIB_COUNTER_TX_LATE_COLLISION    ( MIB_COUNTER_TX + 1 )
#define MIB_COUNTER_TX_PAUSE             ( MIB_COUNTER_TX + 2 )
#define MIB_COUNTER_TX_BROADCAST         ( MIB_COUNTER_TX + 3 )
#define MIB_COUNTER_TX_MULTICAST         ( MIB_COUNTER_TX + 4 )
#define MIB_COUNTER_TX_UNICAST           ( MIB_COUNTER_TX + 5 )
#define MIB_COUNTER_TX_DEFERRED          ( MIB_COUNTER_TX + 6 )
#define MIB_COUNTER_TX_TOTAL_COLLISION   ( MIB_COUNTER_TX + 7 )
#define MIB_COUNTER_TX_EXCESS_COLLISION  ( MIB_COUNTER_TX + 8 )
#define MIB_COUNTER_TX_SINGLE_COLLISION  ( MIB_COUNTER_TX + 9 )
#define MIB_COUNTER_TX_MULTI_COLLISION   ( MIB_COUNTER_TX + 10 )


struct hw_fn;

typedef struct
{
    struct hw_fn*           m_hwfn;

    UCHAR                   m_bPermanentAddress[ MAC_ADDRESS_LENGTH ];

    UCHAR                   m_bOverrideAddress[ MAC_ADDRESS_LENGTH ];

    /* PHY status info. */
    UINT                    m_ulHardwareState;
    UINT32                  m_ulTransmitRate;
    UINT                    m_ulDuplex;

    USHORT                  m_wAdvertised;
    USHORT                  m_wLinkPartner;

    /* hardware resources */
    PUCHAR                  m_pVirtualMemory;

    UINT32                  m_dwTransmitConfig;
    UINT32                  m_dwReceiveConfig;
    UINT32                  m_ulInterruptMask;
    UINT32                  m_ulInterruptSet;
    UINT                    m_InterruptBlocked;
    UINT                    m_dwRegCounter;
    int                     m_nRegCounter;
    int                     m_nTxCounterStart;
    UCHAR                   m_nShift;
    UCHAR                   m_nPhyShift;
    UCHAR                   m_bReceiveStop;
    UCHAR                   m_bLinkIntWorking;

    UCHAR                   m_bEnabled;
    UCHAR                   m_bPromiscuous;
    UCHAR                   m_bAllMulticast;

    /* List of multicast addresses in use. */
    UCHAR                   m_bMulticastListSize;
    UCHAR                   m_bMulticastList[ MAX_MULTICAST_LIST ]
        [ MAC_ADDRESS_LENGTH ];

    /* member variables used for receiving */
    int                     m_nPacketLen;
    PUCHAR                  m_bLookahead;

    USHORT                  m_wPhyAddr;

    UCHAR                   m_bMacOverrideAddr;

    UCHAR                   m_bCurrentCounter;
    ULONGLONG               m_cnMIB[ TOTAL_PORT_COUNTER_NUM ];

    UCHAR                   m_bAcquire;

    /* hardware configurations read from the registry */
    /* 2: Full duplex; 1: half duplex */
    UCHAR                   m_bDuplex;
    /* 10: 10BT; 100: 100BT */
    USHORT                  m_wSpeed;

    UCHAR                   m_bMulticastBits[ HW_MULTICAST_SIZE ];

    TDescInfo               m_RxDescInfo;
    TDescInfo               m_TxDescInfo;

#ifdef SKIP_TX_INT
    int                     m_TxIntCnt;
    int                     m_TxIntMask;
    int                     m_TxSize;
#endif

#ifdef CONFIG_KSZ8692VB
    TAclInfo                AclInfo[ TOTAL_ACL_NUM ];

    UCHAR                   m_bBroadcastCounter;
    UCHAR                   m_b802_1p_mapping;
    UCHAR                   m_fBlock    : 1;
    UCHAR                   m_fBroadcast: 1;
    UCHAR                   m_fDiffServ : 1;
    UCHAR                   m_f802_1p   : 1;
    UCHAR                   m_bReserved;
    UINT                    m_dwDiffServ0;
    UINT                    m_dwDiffServ1;
#endif

#ifdef DEBUG_OVERRUN
    UINT                    m_ulDropped;
    UINT                    m_ulReceived;
#endif
    void*                   m_pDevice;

    /* member variables used for sending commands, mostly for debug purpose */
    int                     m_nWaitDelay[ WAIT_DELAY_LAST ];

    /* member variables for statistics */
    ULONGLONG               m_cnCounter[ OID_COUNTER_LAST ];  /* Driver statistics counter */
    UINT                    m_nBad[ COUNT_BAD_LAST ];
    UINT                    m_nGood[ COUNT_GOOD_LAST ];

} HARDWARE, *PHARDWARE;


struct hw_fn {
    void ( *fnHardwareGetLinkSpeed )( PHARDWARE );
    void ( *fnHardwareSetLinkSpeed )( PHARDWARE );
    void ( *fnHardwareSetupPhy )( PHARDWARE );

    void ( *fnHardwareGetCableStatus )( PHARDWARE, void* );
    void ( *fnHardwareGetLinkStatus )( PHARDWARE, void* );
    void ( *fnHardwareSetCapabilities )( PHARDWARE, UINT );
};


extern struct hw_fn* ks8692_fn;


#ifdef NDIS_MINIPORT_DRIVER
#if defined( NDIS50_MINIPORT )  ||  defined( NDIS51_MINIPORT )
#include <poppack.h>
#else

#ifdef UNDER_CE
    #pragma warning(disable:4103)
    #pragma pack(1)

#else
#include <packoff.h>
#endif
#endif
#endif

/* -------------------------------------------------------------------------- */

#if 0
#define RXCHECKSUM_DEFAULT      FALSE     /* HW Rx IP/TCP/UDP checksum enable */
#else
#define RXCHECKSUM_DEFAULT      TRUE      /* HW Rx IP/TCP/UDP checksum enable */
#endif
#if 1
#define TXCHECKSUM_DEFAULT      FALSE     /* HW Tx IP/TCP/UDP checksum enable */
#else
#define TXCHECKSUM_DEFAULT      TRUE      /* HW Tx IP/TCP/UDP checksum enable */
#endif
#define FLOWCONTROL_DEFAULT     TRUE      /* Flow control enable */
#define PBL_DEFAULT             8         /* DMA Tx/Rx burst Size. 0:unlimited, other value for (4 * x) */


#ifdef VERIFY_CSUM_GEN
#undef TXCHECKSUM_DEFAULT
#define TXCHECKSUM_DEFAULT      FALSE
#endif

/* -------------------------------------------------------------------------- */

/*
    Initial setup routines
*/

#ifdef DBG
void CheckDescriptors (
    PTDescInfo pInfo );
#endif

void CheckDescriptorNum (
    PTDescInfo pInfo );

BOOLEAN HardwareInitialize (
    PHARDWARE pHardware );

BOOLEAN HardwareReset (
    PHARDWARE pHardware );

void HardwareSetup (
    PHARDWARE pHardware );

void HardwareSetupInterrupt (
    PHARDWARE pHardware );

/* -------------------------------------------------------------------------- */

/*
    Link processing primary routines
*/

void HardwareCheckLink (
    PHARDWARE pHardware );

/* -------------------------------------------------------------------------- */

/*
    Receive processing primary routines
*/

#ifdef INLINE
#define HardwareAcknowledgeReceive( pHardware )                             \
{                                                                           \
    KS_WRITE_REG( KS_INT_STATUS, INT_RX << pHardware->m_nShift );           \
}

#else
void HardwareReleaseReceive (
    PHARDWARE pHardware );

void HardwareAcknowledgeReceive (
    PHARDWARE pHardware );
#endif

#define HardwareResumeReceive( pHardware )                                  \
{                                                                           \
    HW_WRITE_DWORD( pHardware, REG_DMA_RX_START, DMA_START );               \
}

void HardwareStartReceive (
    PHARDWARE pHardware );

void HardwareStopReceive (
    PHARDWARE pHardware );

/* -------------------------------------------------------------------------- */

/*
    Transmit processing primary routines
*/

#ifdef INLINE
#define HardwareAcknowledgeTransmit( pHardware )                            \
{                                                                           \
    KS_WRITE_REG( KS_INT_STATUS, INT_TX << pHardware->m_nShift );           \
}

#else
void HardwareAcknowledgeTransmit (
    PHARDWARE pHardware );
#endif

void HardwareStartTransmit (
    PHARDWARE pHardware );

void HardwareStopTransmit (
    PHARDWARE pHardware );

/* -------------------------------------------------------------------------- */

/*
    Interrupt processing primary routines
*/

#ifdef INLINE
#define HardwareAcknowledgeInterrupt( pHardware, ulInterrupt )              \
{                                                                           \
    KS_WRITE_REG( KS_INT_STATUS, ulInterrupt << ( pHardware )->m_nShift );  \
}

#define HardwareDisableInterrupt( pHardware )                               \
{                                                                           \
    UINT data;                                                              \
    ( pHardware )->m_InterruptBlocked = ( pHardware )->m_ulInterruptMask;   \
    data = KS_READ_REG( KS_INT_ENABLE );                                    \
    data &= ~( INT_CHECK << ( pHardware )->m_nShift );                      \
    KS_WRITE_REG( KS_INT_ENABLE, data );                                    \
    data = KS_READ_REG( KS_INT_ENABLE );                                    \
    ( pHardware )->m_ulInterruptSet = ( data >> ( pHardware )->m_nShift )   \
        & INT_CHECK;                                                        \
}

#define HardwareEnableInterrupt( pHardware )                                \
{                                                                           \
    ( pHardware )->m_InterruptBlocked = 0;                                  \
    HardwareSetInterrupt( pHardware, ( pHardware )->m_ulInterruptMask );    \
}

#define HardwareReadInterrupt( pHardware, pulStatus )                       \
{                                                                           \
    *( pulStatus ) = ( KS_READ_REG( KS_INT_STATUS ) >>                      \
        ( pHardware )->m_nShift ) & INT_CHECK;                              \
}

#define HardwareRestoreInterrupt( pHardware, Interrupt )                    \
{                                                                           \
    if ( Interrupt )                                                        \
    {                                                                       \
        ( pHardware )->m_InterruptBlocked = 0;                              \
        HardwareSetInterrupt( pHardware,                                    \
            ( pHardware )->m_ulInterruptMask );                             \
    }                                                                       \
}

#define HardwareSetInterrupt( pHardware, ulInterrupt )                      \
{                                                                           \
    UINT data;                                                              \
    ( pHardware )->m_ulInterruptSet = ulInterrupt;                          \
    data = KS_READ_REG( KS_INT_ENABLE );                                    \
    data &= ~( INT_CHECK << ( pHardware )->m_nShift );                      \
    data |= ( pHardware )->m_ulInterruptMask << ( pHardware )->m_nShift;    \
    KS_WRITE_REG( KS_INT_ENABLE, data );                                    \
}

#else
void HardwareAcknowledgeInterrupt (
    PHARDWARE pHardware,
    UINT      Interrupt );

void HardwareDisableInterrupt (
    PHARDWARE pHardware );

void HardwareEnableInterrupt (
    PHARDWARE pHardware );

#if 0
void HardwareDisableInterruptBit (
    PHARDWARE pHardware,
    UINT32    ulInterrupt );

void HardwareEnableInterruptBit (
    PHARDWARE pHardware,
    UINT32    ulInterrupt );
#endif

void HardwareReadInterrupt (
    PHARDWARE pHardware,
    UINT*     pStatus );

void HardwareRestoreInterrupt (
    PHARDWARE pHardware,
    UINT      Interrupt );

void HardwareSetInterrupt (
    PHARDWARE pHardware,
    UINT      Interrupt );
#endif

UINT HardwareBlockInterrupt (
    PHARDWARE pHardware );

/* -------------------------------------------------------------------------- */

/*
    Other interrupt primary routines
*/

#ifdef INLINE
#define HardwareTurnOffInterrupt( pHardware, ulInterruptBit )               \
{                                                                           \
    ( pHardware )->m_ulInterruptMask &= ~( ulInterruptBit );                \
}

#else
void HardwareTurnOffInterrupt (
    PHARDWARE pHardware,
    UINT      ulInterruptBit );
#endif

void HardwareTurnOnInterrupt (
    PHARDWARE pHardware,
    UINT      ulInterruptBit,
    UINT*     pulInterruptMask );

#define HardwareAcknowledgeEmpty( pHardware )                               \
    HardwareAcknowledgeInterrupt( pHardware, INT_WAN_TX_BUF_UNAVAIL )

#define HardwareAcknowledgeOverrun( pHardware )                             \
    HardwareAcknowledgeInterrupt( pHardware, INT_RX_OVERRUN )

#define HardwareTurnOffEmptyInterrupt( pHardware )                          \
    HardwareTurnOffInterrupt( pHardware, INT_WAN_TX_BUF_UNAVAIL )

#define HardwareTurnOnEmptyInterrupt( pHardware, pulInterruptMask )         \
    HardwareTurnOnInterrupt( pHardware, INT_WAN_TX_BUF_UNAVAIL,             \
        pulInterruptMask )

/* -------------------------------------------------------------------------- */

/*
    Register saving routines
*/

/* -------------------------------------------------------------------------- */

/*
    Hardware enable/disable secondary routines
*/

void HardwareDisable (
    PHARDWARE pHardware );

void HardwareEnable (
    PHARDWARE pHardware );

/* -------------------------------------------------------------------------- */

/*
    Receive processing secondary routines
*/

#ifdef INLINE
#define GetReceivedPacket( pInfo, iNext, pDesc, status )                    \
{                                                                           \
    pDesc = &( pInfo )->pRing[ iNext ];                                     \
    status = LE32_TO_CPU( pDesc->phw->Control.ulData );                     \
}

#define FreeReceivedPacket( pInfo, iNext )                                  \
{                                                                           \
    iNext++;                                                                \
    iNext &= ( pInfo )->iMax;                                               \
}

#define GetRxPacket( pInfo, pDesc )                                         \
{                                                                           \
    pDesc = &( pInfo )->pRing[( pInfo )->iLast ];                           \
    ( pInfo )->iLast++;                                                     \
    ( pInfo )->iLast &= ( pInfo )->iMax;                                    \
    ( pInfo )->cnAvail--;                                                   \
    ( pDesc )->sw.BufSize.ulData &= ~DESC_RX_MASK;                          \
}

#else
void HardwareFreeReceivedPacket (
    PTDescInfo pInfo,
    int*       piNext );

#define FreeReceivedPacket( pInfo, iNext )                                  \
    HardwareFreeReceivedPacket( pInfo, &( iNext ))

PTDesc HardwareGetReceivedPacket (
    PTDescInfo pInfo,
    int        iNext,
    UINT32*    pulData );

#define GetReceivedPacket( pInfo, iNext, pDesc, status )                    \
    pDesc = HardwareGetReceivedPacket( pInfo, iNext, &( status ))

PTDesc HardwareGetRxPacket (
    PTDescInfo pInfo );

#define GetRxPacket( pInfo, pDesc )                                         \
    pDesc = HardwareGetRxPacket( pInfo )
#endif

void HardwareReceive (
    PHARDWARE pHardware );

/* -------------------------------------------------------------------------- */

/*
    Transmit processing secondary routines
*/

int HardwareAllocPacket (
    PHARDWARE pHardware,
    int       length,
    int       physical );

#ifdef INLINE
#define FreeTransmittedPacket( pInfo, iLast )                               \
{                                                                           \
    iLast++;                                                                \
    iLast &= ( pInfo )->iMax;                                               \
    ( pInfo )->cnAvail++;                                                   \
}

#define HardwareFreeTxPacket( pInfo )                                       \
{                                                                           \
    ( pInfo )->iNext--;                                                     \
    ( pInfo )->iNext &= ( pInfo )->iMax;                                    \
    ( pInfo )->cnAvail++;                                                   \
}

#define GetTransmittedPacket( pInfo, iLast, pDesc, status )                 \
{                                                                           \
    pDesc = &( pInfo )->pRing[ iLast ];                                     \
    status = LE32_TO_CPU( pDesc->phw->Control.ulData );                     \
}

#define GetTxPacket( pInfo, pDesc )                                         \
{                                                                           \
    pDesc = &( pInfo )->pRing[( pInfo )->iNext ];                           \
    ( pInfo )->iNext++;                                                     \
    ( pInfo )->iNext &= ( pInfo )->iMax;                                    \
    ( pInfo )->cnAvail--;                                                   \
    pDesc->sw.BufSize.ulData &= ~DESC_TX_MASK;                              \
}

#else
void HardwareFreeTransmittedPacket (
    PTDescInfo pInfo,
    int*       piLast );

#define FreeTransmittedPacket( pInfo, iLast )                               \
    HardwareFreeTransmittedPacket( pInfo, &iLast )

void HardwareFreeTxPacket (
    PTDescInfo pInfo );

PTDesc HardwareGetTransmittedPacket (
    PTDescInfo pInfo,
    int        iLast,
    UINT32*    pulData );

#define GetTransmittedPacket( pInfo, iLast, pDesc, status )                 \
    pDesc = HardwareGetTransmittedPacket( pInfo, iLast, &( status ))

PTDesc HardwareGetTxPacket (
    PTDescInfo pInfo );

#define GetTxPacket( pInfo, pDesc )                                         \
    pDesc = HardwareGetTxPacket( pInfo )

#endif

BOOLEAN HardwareSendPacket (
    PHARDWARE pHardware );

#define SetTransmitBuffer( pDesc, addr )                                    \
    ( pDesc )->phw->ulBufAddr = CPU_TO_LE32( addr )

#define SetTransmitLength( pDesc, len )                                     \
    ( pDesc )->sw.BufSize.tx.wBufSize = len

#ifdef INLINE
#define HardwareSetTransmitBuffer( pHardware, addr )                        \
    SetTransmitBuffer(( pHardware )->m_TxDescInfo.pCurrent, addr )

#else

void HardwareSetTransmitBuffer (
    PHARDWARE pHardware,
    UINT32    ulBufAddr );
#endif

#define HardwareSetTransmitLength( pHardware, len )                         \
    SetTransmitLength(( pHardware )->m_TxDescInfo.pCurrent, len )

/* -------------------------------------------------------------------------- */

/*
    Other secondary routines
*/

UINT32 ether_crc (
    int  length,
    unsigned char *data );

BOOLEAN HardwareReadAddress (
    PHARDWARE pHardware );

void HardwareSetAddress (
    PHARDWARE pHardware );

void HardwareClearMulticast (
    PHARDWARE pHardware );

BOOLEAN HardwareSetGroupAddress (
    PHARDWARE pHardware );

BOOLEAN HardwareSetMulticast (
    PHARDWARE pHardware,
    UCHAR     bMulticast );

BOOLEAN HardwareSetPromiscuous (
    PHARDWARE pHardware,
    UCHAR     bPromiscuous );

/* -------------------------------------------------------------------------- */

void HardwareClearCounters (
    PHARDWARE pHardware );

/* -------------------------------------------------------------------------- */

#define HardwareGetLinkSpeed( phwi )                                        \
    ( phwi )->m_hwfn->fnHardwareGetLinkSpeed( phwi )

#define HardwareSetLinkSpeed( phwi )                                        \
    ( phwi )->m_hwfn->fnHardwareSetLinkSpeed( phwi )

#define HardwareSetupPhy( phwi )                                            \
    ( phwi )->m_hwfn->fnHardwareSetupPhy( phwi )

#define HardwareGetCableStatus( phwi, pBuffer )                             \
    ( phwi )->m_hwfn->fnHardwareGetCableStatus( phwi, pBuffer )

#define HardwareGetLinkStatus( phwi, pBuffer )                              \
    ( phwi )->m_hwfn->fnHardwareGetLinkStatus( phwi, pBuffer )

#define HardwareSetCapabilities( phwi, caps )                               \
    ( phwi )->m_hwfn->fnHardwareSetCapabilities( phwi, caps )

int ks8001_InitPhy ( void );
int vsc8201_InitPhy ( void );
int Generic_InitPhy ( void );

/* -------------------------------------------------------------------------- */

#define ReleaseDescriptor( pDesc, status )                                  \
{                                                                           \
    status.rx.fHWOwned = FALSE;                                             \
    ( pDesc )->phw->Control.ulData = CPU_TO_LE32( status.ulData );          \
}

#define ReleasePacket( pDesc )                                              \
{                                                                           \
    ( pDesc )->sw.Control.tx.fHWOwned = TRUE;                               \
    if ( ( pDesc )->sw.ulBufSize != ( pDesc )->sw.BufSize.ulData )          \
    {                                                                       \
        ( pDesc )->sw.ulBufSize = ( pDesc )->sw.BufSize.ulData;             \
        ( pDesc )->phw->BufSize.ulData =                                    \
            CPU_TO_LE32(( pDesc )->sw.BufSize.ulData );                     \
    }                                                                       \
    ( pDesc )->phw->Control.ulData =                                        \
        CPU_TO_LE32(( pDesc )->sw.Control.ulData );                         \
}

#define SetReceiveBuffer( pDesc, addr )                                     \
    ( pDesc )->phw->ulBufAddr = CPU_TO_LE32( addr )

#define SetReceiveLength( pDesc, len )                                      \
    ( pDesc )->sw.BufSize.rx.wBufSize = len

/* -------------------------------------------------------------------------- */

void HardwareInitDescriptors (
    PTDescInfo pDescInfo,
    int        fTransmit );

void HardwareSetDescriptorBase (
    PHARDWARE  pHardware,
    UINT32     TxDescBaseAddr,
    UINT32     RxDescBaseAddr );

void HardwareResetPackets (
    PTDescInfo pInfo );

#define HardwareResetRxPackets( pHardware )                                 \
    HardwareResetPackets( &pHardware->m_RxDescInfo )

#define HardwareResetTxPackets( pHardware )                                 \
    HardwareResetPackets( &pHardware->m_TxDescInfo )

/* -------------------------------------------------------------------------- */

#define STA_TIMEOUT  12

int HardwareReadPhy (
    int       port,
    USHORT    wPhyReg,
    PUSHORT   pwData );

int HardwareWritePhy (
    int       port,
    USHORT    wPhyReg,
    USHORT    wRegData );

/* -------------------------------------------------------------------------- */

void HardwareInitCounters (
    PHARDWARE pHardware );

int HardwareReadCounters (
    PHARDWARE pHardware );

/* -------------------------------------------------------------------------- */

void HardwareSetWolEnable (
    PHARDWARE pHardware,
    USHORT    frame,
    int       enable );

void HardwareSetWolFrame (
    PHARDWARE pHardware,
    int       index,
    ULONG     masksize,
    PUCHAR    mask,
    ULONG     patternsize,
    PUCHAR    pattern );

#define HardwareEnableMagicPacket( pHardware, enable )                      \
    HardwareSetWolEnable( pHardware, WOL_MAGIC_ENABLE, enable )

#define HardwareEnableWakeup( pHardware, index, enable )                    \
    HardwareSetWolEnable( pHardware, 1 << ( index ), enable )

/* -------------------------------------------------------------------------- */

#ifdef CONFIG_KSZ8692VB
void HardwareSetACL (
    PHARDWARE  pHardware,
    int        acl,
    int        enable,
    UINT       mask );

#define HardwareSet_ACL_Enable( pHardware, acl, enable )                    \
    HardwareSetACL( pHardware, acl, enable, ACL_ENABLE )

#define HardwareSet_ACL_Filter( pHardware, acl, filter )                    \
    HardwareSetACL( pHardware, acl, filter, ACL_FILTER_EN )

#define HardwareSet_ACL_Priority( pHardware, acl, priority )                \
    HardwareSetACL( pHardware, acl, priority, ACL_HI_PRIO )

#define HardwareSet_ACL_V6( pHardware, acl, enable )                        \
    HardwareSetACL( pHardware, acl, enable, ACL_IPV6 )

void HardwareSet_ACL_Block (
    PHARDWARE  pHardware,
    int        enable );

void HardwareSet_ACL_Mac (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    UCHAR*     addr );

void HardwareSet_ACL_Offset (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    int        offset,
    UINT       data,
    UINT       mask );

void HardwareSet_ACL_Protocol (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    UCHAR      data,
    UCHAR      mask );

void HardwareSet_ACL_Port (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    int        cfg,
    USHORT     data,
    USHORT     mask );

void HardwareSet_ACL_IPv4 (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    int        cfg,
    UCHAR*     addr,
    UCHAR*     mask );

void HardwareSet_ACL_IPv6 (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    int        cfg,
    UCHAR*     addr,
    UCHAR*     mask );


typedef enum {
    ACL_MODE_MAC,

    /* Need to be in this order. */
    ACL_MODE_OFFSET,
    ACL_MODE_PROTOCOL,
    ACL_MODE_PORT_DST,
    ACL_MODE_PORT_SRC,
    ACL_MODE_PORT_BOTH,

    ACL_MODE_IPV4_DST,
    ACL_MODE_IPV4_SRC,
    ACL_MODE_IPV4_BOTH,

    ACL_MODE_IPV6_DST,
    ACL_MODE_IPV6_SRC,
    ACL_MODE_IPV6_BOTH,

    ACL_MODE_LAST
} TAclMode;


void HardwareSetupACL (
    PHARDWARE pHardware,
    PTAclInfo acl,
    int       first,
    int       last );

/* -------------------------------------------------------------------------- */

void HardwareSetBroadcastTraffic (
    PHARDWARE pHardware,
    UCHAR     bEnable,
    UCHAR     bValue );

void HardwareSetDiffServPriority (
    PHARDWARE pHardware,
    UCHAR     bEnable,
    UINT32    dwMapping0,
    UINT32    dwMapping1 );

void HardwareSet802_1P_Priority (
    PHARDWARE pHardware,
    UCHAR     bEnable,
    UCHAR     bMapping );
#endif

/* -------------------------------------------------------------------------- */

#if 0
USHORT EepromReadWord (
    PHARDWARE pHardware,
    UCHAR     Reg );

void EepromWriteWord (
    PHARDWARE pHardware,
    UCHAR     Reg,
    USHORT    Data );
#endif

/* -------------------------------------------------------------------------- */


#endif

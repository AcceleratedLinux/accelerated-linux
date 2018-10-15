/* ---------------------------------------------------------------------------
          Copyright (c) 2006-2008 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    device.h - Linux network device driver header.

    Author      Date        Description
    THa         12/05/06    Created file.
    Ken         06/22/07    Updated for dma usage instead of PCI usage
    THa         11/01/07    Add delay enable receive interrupt code for
                            testing purpose.
    THa         11/20/07    Allocate socket buffers close to MTU so that
                            skb->truesize is used for correct TCP window
                            calculation.
    THa         04/25/08    Add SNMP MIB support.
    THa         07/17/08    Add Access Control List (ACL) support.
    THa         07/24/08    Workaround transmit failure problem.
    THa         09/16/08    Add netconsole support.
   ---------------------------------------------------------------------------
*/


#ifndef __DEVICE_H
#define __DEVICE_H


#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/mii.h>


#if 1
#define RX_COPY
#endif

/* Scatter/gather requires hardware checksumming. */
#if TXCHECKSUM_DEFAULT
#define SCATTER_GATHER
#endif

#if 1
#define HIGH_MEM_SUPPORT
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 20))
#if 0
#define CONFIG_KSZ_NAPI
#endif
#endif

/* -------------------------------------------------------------------------- */

typedef struct {
    struct timer_list* pTimer;
    void*              pData;
    unsigned int       cnCount;
    int                nCount;
    int                nTime;
} TTimerInfo, *PTTimerInfo;

/* -------------------------------------------------------------------------- */

typedef struct
{
    dma_addr_t dma_addr;
    UINT       ulAllocSize;
    UINT       ulPhysical;
    PUCHAR     pAllocVirtual;
    PUCHAR     pVirtual;
} SHARED_MEM, *PSHARED_MEM;


typedef struct _DMA_BUFFER
{
    struct sk_buff*     skb;
    dma_addr_t          dma;
    int                 len;
#if 0
    UINT                ulSize;
#endif
    struct _DMA_BUFFER* pNext;
} DMA_BUFFER, *PDMA_BUFFER;

#define DMA_BUFFER( pDesc )  (( PDMA_BUFFER )(( pDesc )->pReserved ))


typedef struct
{

    /* Array of buffers. */
    PDMA_BUFFER BufArray;

    /* Current buffer being manipulated .*/
    PDMA_BUFFER pCurrent;

    /* First buffer in the free stack. */
    PDMA_BUFFER pTop;

    /* Number of buffers allocated. */
    int         cnAlloc;

    /* Number of buffers available for use. */
    int         cnAvail;
} BUFFER_INFO, *PBUFFER_INFO;

/* -------------------------------------------------------------------------- */

typedef struct {
    wait_queue_head_t       wqhCounter;
    unsigned long           time;
    int                     fRead;
} COUNTER_INFO, *PCOUNTER_INFO;

/* -------------------------------------------------------------------------- */

typedef enum {

    /* Read-only entries. */
    PROC_INFO,
    PROC_GET_CABLE_STATUS,
    PROC_GET_LINK_STATUS,

    /* Read-write entries. */
    PROC_SET_DUPLEX,
    PROC_SET_SPEED,
    PROC_SET_CAPABILITIES,
    PROC_SET_MIB,

#ifdef CONFIG_KSZ8692VB
    PROC_SET_BROADCAST_ENABLE,
    PROC_SET_BROADCAST_COUNTER,

    PROC_SET_DIFFSERV_ENABLE,
    PROC_SET_DIFFSERV_MAPPING0,
    PROC_SET_DIFFSERV_MAPPING1,
    PROC_SET_DIFFSERV_SET,
    PROC_SET_DIFFSERV_UNSET,

    PROC_SET_802_1P_ENABLE,
    PROC_SET_802_1P_MAPPING,
    PROC_SET_802_1P_SET,
    PROC_SET_802_1P_UNSET,

    /* ACL specific read-write entries. */
    PROC_SET_ACL_BLOCK_UNMATCHED,

    PROC_SET_ACL_ENABLE,
    PROC_SET_ACL_RX_HI_PRIORITY,
    PROC_SET_ACL_FILTER_MATCHED,
    PROC_SET_ACL_MODE,

    PROC_SET_ACL_DATA,
    PROC_SET_ACL_MASK,
    PROC_SET_ACL_MAC,
    PROC_SET_ACL_OFFSET,
    PROC_SET_ACL_PROTOCOL,
    PROC_SET_ACL_PORT,
    PROC_SET_ACL_IPV4_ADDR,
    PROC_SET_ACL_IPV4_MASK,
    PROC_SET_ACL_IPV6_ADDR,
    PROC_SET_ACL_IPV6_MASK,
#endif

    /* Port specific read-only entries. */

    /* Port specific read-write entries. */

    PROC_LAST
} TProcNum;


typedef struct {
    int   proc_num;
    char* proc_name;
} TProcInfo, *PTProcInfo;


typedef struct {
    void*      priv;
    PTProcInfo info;
    int        port;
} TProcPortInfo, *PTProcPortInfo;

/* -------------------------------------------------------------------------- */

struct dev_info {
    struct net_device*      dev;
    struct device*          pdev;

    spinlock_t              lockHardware;
    wait_queue_head_t       wqhHardware;
    TProcPortInfo           ProcPortInfo[ PROC_LAST ];
    struct proc_dir_entry*  proc_main;

#ifdef CONFIG_KSZ8692VB
    TProcPortInfo           ProcAclInfo[ TOTAL_ACL_NUM ]
        [ PROC_SET_ACL_IPV6_MASK - PROC_SET_ACL_ENABLE + 1 ];
    struct proc_dir_entry*  proc_acldir;
    struct proc_dir_entry*  proc_acl[ TOTAL_ACL_NUM ];
#endif

    struct semaphore        proc_sem;

    SHARED_MEM              m_DescPool;

    int                     m_nMTU;
    int                     m_nMaxBufSize;
    int                     m_nRxBufSize;
    int                     m_nTxBufSize;
    BUFFER_INFO             m_RxBufInfo;
    BUFFER_INFO             m_TxBufInfo;

    HARDWARE                hw;

    int                     id;

    TTimerInfo              MonitorTimerInfo;
    struct timer_list       timerMonitor;

    struct tasklet_struct   rx_tasklet;
    struct tasklet_struct   tx_tasklet;

#ifdef AVOID_TX_FAILURE
    struct tasklet_struct   reset_tasklet;
    int                     gigabit_rate;
#endif

#ifdef CONFIG_DELAYED_TIMER
    struct delayed_tasklet  delay_rx_taskinfo;
#endif

    COUNTER_INFO            Counter;

    unsigned long           pme_wait;

    char                    irq_name[ 6 ][ 40 ];
};


struct dev_priv {
    struct net_device_stats stats;

    struct dev_info         dev_info;
    struct dev_info*        pDevInfo;

    spinlock_t              lock;

    struct mii_if_info      mii_if;
    u32                     advertising;

    /* debug message level */
    u32                     msg_enable;

    struct sk_buff*         skb_waiting;

    int                     media_state;
    int                     opened;

#ifdef DEBUG_COUNTER
    unsigned long           query_jiffies;
#endif
};

/* -------------------------------------------------------------------------- */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
void

#else
irqreturn_t
#endif
ks8692_dev_interrupt (
    int             irq,
    void*           dev_id
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 21))
    ,
    struct pt_regs* regs
#endif
    );

/* -------------------------------------------------------------------------- */

void ks8692_transmit_alloc_done (
    struct net_device* dev );

void ks8692_transmit_done (
    struct net_device* dev );

void transmit_reset (
    struct net_device* dev );

int ks8692_dev_transmit (
    struct sk_buff*    skb,
    struct net_device* dev );

void ks8692_dev_transmit_timeout (
    struct net_device *dev );

#ifdef CONFIG_KSZ_NAPI
int ks8692_dev_poll (
    struct net_device* dev,
    int*               budget );

#define DEV_POLL              ks8692_dev_poll
#endif

#define DEV_INTERRUPT_PTR     ks8692_dev_interrupt

#define TRANSMIT_ALLOC_DONE   ks8692_transmit_alloc_done
#define TRANSMIT_DONE         ks8692_transmit_done
#define TRANSMIT_EMPTY        ks8692_transmit_empty
#define DEV_TRANSMIT          ks8692_dev_transmit
#define DEV_TRANSMIT_TIMEOUT  ks8692_dev_transmit_timeout

/* -------------------------------------------------------------------------- */

int AcquireHardware (
    struct dev_info* priv,
    int              in_intr,
    int              wait );

void ReleaseHardware (
    struct dev_info* priv,
    int              in_intr );

/* -------------------------------------------------------------------------- */

void InitBuffers (
    PBUFFER_INFO pBufInfo );

void InitReceiveBuffers (
    struct dev_info* pAdapter );

/* -------------------------------------------------------------------------- */

#endif

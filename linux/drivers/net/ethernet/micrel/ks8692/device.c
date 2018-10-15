/* ---------------------------------------------------------------------------
          Copyright (c) 2006-2008 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    device.c - Linux network device driver.

    Author      Date        Description
    THa         12/05/06    Created file.
    THa         03/09/07    Add ethtool hardware checksum functions.
    Ken         06/22/07    Updated for dma usage instead of PCI usage
    THa         08/29/07    Update for Linux 2.6.21.
    THa         11/01/07    Add delay enable receive interrupt code for
                            testing purpose.
    THa         11/13/07    Change receive buffer size to 1744 for better TCP
                            performance when using IP header alignment.
    THa         11/20/07    Allocate socket buffers close to MTU so that
                            skb->truesize is used for correct TCP window
                            calculation.
    THa         11/27/07    Correct huge frame support.
    THa         11/30/07    Correct MIB counter reporting.
    THa         02/08/08    Use platform device model.
    THa         03/28/08    Update for Linux 2.6.24.
    THa         04/25/08    Add SNMP MIB support.
    THa         07/17/08    Add Access Control List (ACL) support.
    THa         07/24/08    Workaround transmit failure problem.
    THa         09/16/08    Add netconsole support.
   ---------------------------------------------------------------------------
*/


#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <mach/platform.h>

MODULE_LICENSE("GPL");

#include "target.h"
#include "hardware.h"
#include "device.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17))
#include <linux/platform_device.h>
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9))
#include <linux/device.h>
#endif
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 21))
#include <linux/ethtool.h>
#endif
#include <linux/etherdevice.h>
#include <asm/irq.h>
#include "ks_ioctl.h"

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))
/*
 * The proc API has completely changed from 3.10 onwards. Maintain the old
 * old code for older kernels. Someone will need to sit down and rewirte the
 * proc support code for newer kernels though.
 */
#define	PROC_OLD_API
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
long interruptible_sleep_on_timeout(wait_queue_head_t *q, long timeout)
{
	long t;

	DEFINE_WAIT(wait);
	prepare_to_wait(q, &wait, TASK_INTERRUPTIBLE);
	t = schedule_timeout(timeout);
	finish_wait(q, &wait);
	return t;
}
#endif
#ifndef IRQF_DISABLED
/* linux-4.1 and later no longer need or define IRQF_DISABLED */
#define IRQF_DISABLED	0
#endif

#ifdef CONFIG_SIM_IPSEC
void test_ipsec ( void );
#endif


#ifdef DBG
#define NET_MSG_ENABLE  ( NETIF_MSG_DRV | NETIF_MSG_PROBE | NETIF_MSG_LINK )
#else
#define NET_MSG_ENABLE  0
#endif


#define BASE_IO_ADDR  KS_VIO_BASE
#define BASE_IRQ      0


#if defined( CONFIG_PEGASUS )
#define DRV_NAME     "Pegasus"
#define DEVICE_NAME  "pegasus-net"
#else
#define DRV_NAME     "Centaur"
#define DEVICE_NAME  "centaur-net"
#endif
#define DRV_VERSION  "1.0.0"
#define DRV_RELDATE  "Oct 7, 2008"

static char version[] =
    "Micrel " DRV_NAME " " DRV_VERSION " (" DRV_RELDATE ")";


#ifdef CONFIG_DELAYED_TIMER
void DelayRxIntrTask ( unsigned long data );
#endif
void RxProcessTask ( unsigned long data );
void TxProcessTask ( unsigned long data );


extern unsigned int ksz_system_bus_clock;

#ifdef CONFIG_KSZ8692VB
extern unsigned long system_time_tick;

#if 1
#define VERIFY_TIMETICK
#endif
#ifdef VERIFY_TIMETICK
static ULONG last_tick = 0;
static ULONG time_tick = 0;
#endif
#endif


#ifdef DBG

/* 2 lines buffer. */
#define DEBUG_MSG_BUF   ( 80 * 2 )

#define PRINT_MSG_SIZE  ( 80 * 20 )
#define PRINT_INT_SIZE  ( 80 * 10 )

#define DEBUG_MSG_SIZE  ( PRINT_MSG_SIZE + PRINT_INT_SIZE + \
    DEBUG_MSG_BUF * 2 )


static char* strDebug = NULL;
static char* strIntBuf = NULL;
static char* strMsg = NULL;
static char* strIntMsg = NULL;
static int   cnDebug = 0;
static int   cnIntBuf = 0;
static int   fLastDebugLine = 1;
static int   fLastIntBufLine = 1;

static unsigned long lockDebug = 0;
#endif


#define DBG_CH  '-'


void DbgMsg (
    char *fmt, ... )
{
#ifdef DBG
    va_list args;
    char**  pstrMsg;
    int*    pcnDebug;
    int     nLeft;
    int     in_intr = in_interrupt();

    pcnDebug = &cnDebug;
    pstrMsg = &strMsg;
    nLeft = PRINT_MSG_SIZE - cnDebug - 1;

    /* Called within interrupt routines. */
    if ( in_intr ) {

        /* If not able to get lock then put in the interrupt message buffer.
        */
        if ( test_bit( 1, &lockDebug ) ) {
            pcnDebug = &cnIntBuf;
            pstrMsg = &strIntMsg;
            nLeft = PRINT_INT_SIZE - cnIntBuf - 1;
            in_intr = 0;
        }
    }
    else {
        set_bit( 1, &lockDebug );
    }
    va_start( args, fmt );
    nLeft = vsnprintf( *pstrMsg, nLeft, fmt, args );
    va_end( args );
    if ( nLeft ) {
        *pcnDebug += nLeft;
        *pstrMsg += nLeft;
    }
    if ( !in_intr ) {
        clear_bit( 1, &lockDebug );
    }
#endif
}  /* DbgMsg */

/* -------------------------------------------------------------------------- */

/*
    StartTimer

    Description:
        This routine starts the kernel timer after the specified time tick.

    Parameters:
        PTTimerInfo pInfo
            Pointer to kernel timer information.

        int nTime
            The time tick.

    Return (None):
*/

static void StartTimer (
    PTTimerInfo pInfo,
    int         nTime )
{
    pInfo->cnCount = 0;
    pInfo->pTimer->expires = jiffies + nTime;
    add_timer( pInfo->pTimer );

    /* infinity */
    pInfo->nCount = -1;
}  /* StartTimer */


/*
    StopTimer

    Description:
        This routine stops the kernel timer.

    Parameters:
        PTTimerInfo pInfo
            Pointer to timer information.

    Return (None):
*/

static void StopTimer (
    PTTimerInfo pInfo )
{
    pInfo->nCount = 0;
    del_timer_sync( pInfo->pTimer );
}  /* StopTimer */

/* -------------------------------------------------------------------------- */

/*
    InitHardware

    Description:
        This routine initializes system variables required to acquire the
        hardware.

    Parameters:
        struct dev_info* priv
            Pointer to real hardware device private data.

    Return (None):
*/

static void InitHardware (
    struct dev_info* priv )
{
    spin_lock_init( &priv->lockHardware );
    init_waitqueue_head( &priv->wqhHardware );
}  /* InitHardware */


/*
    AcquireHardware

    Description:
        This function is used to acquire the hardware so that only one process
        has access to the hardware.

    Parameters:
        struct dev_info* priv
            Pointer to real hardware device private data.

        int in_intr
            Indicate calling from interrupt routines.

        int wait
            Indicate whether to wait or not.  User functions should wait, while
            periodic timer routine should not.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

int AcquireHardware (
    struct dev_info* priv,
    int              in_intr,
    int              wait )
{
    int rc;

    do {

        /* Acquire access to the hardware acquire count variable. */
        if ( in_intr )
            spin_lock( &priv->lockHardware );
        else
            spin_lock_irq( &priv->lockHardware );
        if ( 0 == priv->hw.m_bAcquire ) {

            /* Increase hardware acquire count. */
            priv->hw.m_bAcquire++;

            /* Release access to the hardware acquire count variable. */
            if ( in_intr )
                spin_unlock( &priv->lockHardware );
            else
                spin_unlock_irq( &priv->lockHardware );

            /* Hardware acquired. */
            return 0;
        }

        /* Release access to the hardware acquire count variable. */
        if ( in_intr )
            spin_unlock( &priv->lockHardware );
        else
            spin_unlock_irq( &priv->lockHardware );

        /* Willing to wait. */
        if ( wait ) {
            if ( ( rc = wait_event_interruptible( priv->wqhHardware,
                    !priv->hw.m_bAcquire )) ) {

                /* System failure. */
                return( rc );
            }
        }
    } while ( wait );

    /* Hardware not acquired. */
    return -EBUSY;
}  /* AcquireHardware */


/*
    ReleaseHardware

    Description:
        This routine is used to release the hardware so that other process can
        have access to the hardware.

    Parameters:
        struct dev_info* priv
            Pointer to real hardware device private data.

        int in_intr
            Indicate calling from interrupt routines.

    Return (None):
*/

void ReleaseHardware (
    struct dev_info* priv,
    int              in_intr )
{
    /* Acquire access to the hardware acquire count variable. */
    if ( in_intr )
        spin_lock( &priv->lockHardware );
    else
        spin_lock_irq( &priv->lockHardware );

    /* Decrease hardware acquire count. */
    priv->hw.m_bAcquire--;

    /* Wake up other processes waiting in the queue. */
    if ( 0 == priv->hw.m_bAcquire )
        wake_up_interruptible( &priv->wqhHardware );

    /* Release access to the hardware acquire count variable. */
    if ( in_intr )
        spin_unlock( &priv->lockHardware );
    else
        spin_unlock_irq( &priv->lockHardware );
}  /* ReleaseHardware */

/* -------------------------------------------------------------------------- */

#ifndef HardwareDisableInterruptSync
/*
    HardwareDisableInterruptSync

    Description:
        This procedure makes sure the interrupt routine is not entered after
        the hardware interrupts are disabled.

    Parameters:
        void* ptr
            Pointer to hardware information structure.

    Return (None):
*/

void HardwareDisableInterruptSync (
    void* ptr )
{
    PHARDWARE          pHardware = ( PHARDWARE ) ptr;
    struct net_device* dev = ( struct net_device* ) pHardware->m_pDevice;
    struct dev_priv*   priv = netdev_priv(dev);
    unsigned long      flags;

    spin_lock_irqsave( &priv->lock, flags );
    HardwareDisableInterrupt( pHardware );
    spin_unlock_irqrestore( &priv->lock, flags );
}  /* HardwareDisableInterruptSync */
#endif

/* -------------------------------------------------------------------------- */

/*
    AllocateSoftDescriptors

    Description:
        This local function allocates software descriptors for manipulation in
        memory.

    Parameters:
        PTDescInfo pDescInfo
            Pointer to descriptor information structure.

        int fTransmit
            Indication that descriptors are for transmit.

    Return (int):
        Zero if successful; otherwise 1.
*/

static int AllocateSoftDescriptors (
    PTDescInfo pDescInfo,
    int        fTransmit )
{
    pDescInfo->pRing = kmalloc( sizeof( TDesc ) * pDescInfo->cnAlloc,
        GFP_KERNEL );
    if ( !pDescInfo->pRing ) {
        return 1;
    }
    memset(( void* ) pDescInfo->pRing, 0,
        sizeof( TDesc ) * pDescInfo->cnAlloc );
    HardwareInitDescriptors( pDescInfo, fTransmit );
    return 0;
}  /* AllocateSoftDescriptors */


/*
    AllocateDescriptors

    Description:
        This local function allocates hardware descriptors for receiving and
        transmitting.

    Parameters:
        struct dev_info* pAdapter
            Pointer to adapter information structure.

    Return (int):
        Zero if successful; otherwise 1.
*/

static int AllocateDescriptors (
    struct dev_info* pAdapter )
{
    PHARDWARE pHardware = &pAdapter->hw;
    int       nOffset;

    /* Allocate memory for RX & TX descriptors. */
    pAdapter->m_DescPool.ulAllocSize =
        pHardware->m_RxDescInfo.nSize * pHardware->m_RxDescInfo.cnAlloc +
        pHardware->m_TxDescInfo.nSize * pHardware->m_TxDescInfo.cnAlloc +
        DESC_ALIGNMENT;

    pAdapter->m_DescPool.pAllocVirtual =
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 9))
        dma_alloc_coherent(
        pAdapter->pdev, pAdapter->m_DescPool.ulAllocSize,
        &( pAdapter->m_DescPool.dma_addr ), GFP_KERNEL );
#else
        pci_alloc_consistent(
        pAdapter->pdev, pAdapter->m_DescPool.ulAllocSize,
        &( pAdapter->m_DescPool.dma_addr ));
#endif
    if ( pAdapter->m_DescPool.pAllocVirtual == NULL )
    {
        pAdapter->m_DescPool.ulAllocSize = 0;
        return 1;
    }
    memset( pAdapter->m_DescPool.pAllocVirtual, 0,
        pAdapter->m_DescPool.ulAllocSize );

    /* Align to the next cache line boundary. */
    nOffset = ((( ULONG ) pAdapter->m_DescPool.pAllocVirtual % DESC_ALIGNMENT )
        ? ( DESC_ALIGNMENT -
        (( ULONG ) pAdapter->m_DescPool.pAllocVirtual % DESC_ALIGNMENT )) : 0 );
    pAdapter->m_DescPool.pVirtual = pAdapter->m_DescPool.pAllocVirtual +
        nOffset;
    pAdapter->m_DescPool.ulPhysical = pAdapter->m_DescPool.dma_addr +
        nOffset;

    /* Allocate receive/transmit descriptors. */
    pHardware->m_RxDescInfo.phwRing = ( PTHw_Desc )
        pAdapter->m_DescPool.pVirtual;
    pHardware->m_RxDescInfo.ulRing = pAdapter->m_DescPool.ulPhysical;
    nOffset = pHardware->m_RxDescInfo.cnAlloc * pHardware->m_RxDescInfo.nSize;
    pHardware->m_TxDescInfo.phwRing = ( PTHw_Desc )
        ( pAdapter->m_DescPool.pVirtual + nOffset );
    pHardware->m_TxDescInfo.ulRing = pAdapter->m_DescPool.ulPhysical + nOffset;

    if ( AllocateSoftDescriptors( &pHardware->m_RxDescInfo, FALSE ) ) {
        return 1;
    }
    if ( AllocateSoftDescriptors( &pHardware->m_TxDescInfo, TRUE ) ) {
        return 1;
    }

    return 0;
}  /* AllocateDescriptors */


/*
    InitBuffers

    Description:
        This local routine initializes the DMA buffer information structure.

    Parameters:
        PBUFFER_INFO pBufInfo
            Pointer to DMA buffer information structure.

    Return (None):
*/

void InitBuffers (
    PBUFFER_INFO pBufInfo )
{
    PDMA_BUFFER pDma;
    PDMA_BUFFER pPrevious = NULL;
    int         i;

    pBufInfo->pTop = pBufInfo->BufArray;
    pBufInfo->cnAvail = pBufInfo->cnAlloc;

    pDma = pBufInfo->BufArray;
    for ( i = 0; i < pBufInfo->cnAlloc; i++ )
    {
        pPrevious = pDma;
        pDma++;
        pPrevious->pNext = pDma;
    }
    pPrevious->pNext = NULL;
}  /* InitBuffers */


/*
    AllocateBuffers

    Description:
        This local function allocates DMA buffers for use in receiving and
        transmitting.

    Parameters:
        struct dev_info* pAdapter
            Pointer to adapter information structure.

        int nBufSize
            Size of the buffer.

        PBUFFER_INFO pBufInfo
            Pointer to DMA buffer information structure.

    Return (int):
        Zero if successful; otherwise 1.
*/

static int AllocateBuffers (
    struct dev_info* pAdapter,
    int              nBufSize,
    PBUFFER_INFO     pBufInfo )
{
    PDMA_BUFFER pDma;
    int         i;

    pBufInfo->BufArray = kmalloc( sizeof( DMA_BUFFER ) * pBufInfo->cnAlloc,
        GFP_KERNEL );
    if ( !pBufInfo->BufArray )
        return 1;
    memset(( void* ) pBufInfo->BufArray, 0,
        sizeof( DMA_BUFFER ) * pBufInfo->cnAlloc );

    /* Set up each buffer. */
    pDma = pBufInfo->BufArray;
    for ( i = 0; i < pBufInfo->cnAlloc; i++ )
    {
        pDma->len = nBufSize;

        pDma++;
    }
    InitBuffers( pBufInfo );

    return 0;
}  /* AllocateBuffers */


/*
    alloc_rx_buf

    Description:
        This function allocates a DMA buffer information block for either
        transmitting or receiving.

    Parameters:
        PBUFFER_INFO pBufInfo
            Pointer to DMA buffer information structure.

        PTDesc pCurrent
            Pointer to current descriptor.

    Return (PDMA_BUFFER):
        Pointer to the DMA buffer information block if successful; NULL
        otherwise.
*/

PDMA_BUFFER alloc_rx_buf (
    PBUFFER_INFO pBufInfo,
    PTDesc       pCurrent )
{
    PDMA_BUFFER pDma;

#if 0
    pDma = NULL;
    if ( pBufInfo->cnAvail )
#else
    ASSERT( pBufInfo->cnAvail );
#endif
    {
        pDma = pBufInfo->pTop;
        pBufInfo->pTop = pDma->pNext;
        pBufInfo->cnAvail--;
#if 0
        /* Remember current receive buffer. */
        pAdapter->m_RxBufInfo.pCurrent = pDma;
#endif

        /* Link to current descriptor. */
        pCurrent->pReserved = pDma;
    }
    return( pDma );
}  /* alloc_rx_buf */


/*
    free_rx_buf

    Description:
        This routine frees the DMA buffer information block.

    Parameters:
        PBUFFER_INFO pBufInfo
            Pointer to DMA buffer information structure.

        PDMA_BUFFER pDma
            Pointer to DMA buffer information block.

    Return (None):
*/

void free_rx_buf (
    PBUFFER_INFO pBufInfo,
    PDMA_BUFFER  pDma )
{
    pDma->pNext = pBufInfo->pTop;
    pBufInfo->pTop = pDma;
    pBufInfo->cnAvail++;
}  /* free_rx_buf */


/*
    InitReceiveBuffers

    Description:
        This routine initializes DMA buffers for receiving.

    Parameters:
        struct dev_info* pAdapter
            Pointer to adapter information structure.

    Return (None):
*/

void InitReceiveBuffers (
    struct dev_info* pAdapter )
{
    int         i;
    PTDesc      pDesc;
    PDMA_BUFFER pDma;
    PHARDWARE   pHardware = &pAdapter->hw;
    PTDescInfo  pInfo = &pHardware->m_RxDescInfo;

    for ( i = 0; i < pHardware->m_RxDescInfo.cnAlloc; i++ )
    {
        GetRxPacket( pInfo, pDesc );

        /* Get a new buffer for this descriptor. */
        pDma = alloc_rx_buf( &pAdapter->m_RxBufInfo, pDesc );
        if ( !pDma->skb ) {
#ifdef ALIGN_IP_HEADER
            /* skb->data != skb->head */
            pDma->skb = dev_alloc_skb( pDma->len );
#else
            /* skb->data == skb->head */
            pDma->skb = alloc_skb( pDma->len, GFP_ATOMIC );
#endif
        }
        if ( pDma->skb  &&  !pDma->dma ) {
            pDma->skb->dev = pAdapter->dev;

#ifdef HIGH_MEM_SUPPORT
            pDma->dma = dma_map_page(
                pAdapter->pdev, virt_to_page( pDma->skb->tail ),
                (( unsigned long ) pDma->skb->tail & ~PAGE_MASK ), pDma->len,
                DMA_FROM_DEVICE );

#else
            pDma->dma = dma_map_single(
                pAdapter->pdev, pDma->skb->tail, pDma->len,
                DMA_FROM_DEVICE );
#endif
        }

        /* Set descriptor. */
        SetReceiveBuffer( pDesc, pDma->dma );
        SetReceiveLength( pDesc, pDma->len - SKB_RESERVED );
        ReleasePacket( pDesc );
    }
}  /* InitReceiveBuffers */


/*
    AllocateMemory

    Description:
        This function allocates memory for use by hardware descriptors for
        receiving and transmitting.

    Parameters:
        struct dev_info* pAdapter
            Pointer to adapter information structure.

    Return (int):
        Zero if successful; otherwise 1.
*/

int AllocateMemory (
    struct dev_info* pAdapter )
{
    PHARDWARE pHardware = &pAdapter->hw;

    /* Determine the number of receive and transmit descriptors. */
    pHardware->m_RxDescInfo.cnAlloc = NUM_OF_RX_DESC;
    pHardware->m_TxDescInfo.cnAlloc = NUM_OF_TX_DESC;

#ifdef SKIP_TX_INT
    pHardware->m_TxIntCnt = 0;
    pHardware->m_TxIntMask = NUM_OF_TX_DESC / 4;
    if ( pHardware->m_TxIntMask > 8 )
    {
        pHardware->m_TxIntMask = 8;
    }
    while ( pHardware->m_TxIntMask ) {
        pHardware->m_TxIntCnt++;
        pHardware->m_TxIntMask >>= 1;
    }
    if ( pHardware->m_TxIntCnt ) {
        pHardware->m_TxIntMask = ( 1 << ( pHardware->m_TxIntCnt - 1 )) - 1;
        pHardware->m_TxIntCnt = 0;
    }
#endif

    /* Determine the descriptor size. */
    pHardware->m_RxDescInfo.nSize =
        ((( sizeof( THw_Desc ) + DESC_ALIGNMENT - 1 ) / DESC_ALIGNMENT ) *
        DESC_ALIGNMENT );
    pHardware->m_TxDescInfo.nSize =
        ((( sizeof( THw_Desc ) + DESC_ALIGNMENT - 1 ) / DESC_ALIGNMENT ) *
        DESC_ALIGNMENT );
    if ( pHardware->m_RxDescInfo.nSize != sizeof( THw_Desc ) )
    {
        DBG_PRINT( "Hardware descriptor size not right!"NEWLINE );
    }
    CheckDescriptorNum( &pHardware->m_RxDescInfo );
    CheckDescriptorNum( &pHardware->m_TxDescInfo );

    /* TX/RX buffers we need. */
    pAdapter->m_RxBufInfo.cnAlloc = pHardware->m_RxDescInfo.cnAlloc;
    pAdapter->m_TxBufInfo.cnAlloc = pHardware->m_TxDescInfo.cnAlloc;

    /* Allocate descriptors */
    if ( AllocateDescriptors( pAdapter ) )
        return 1;

    /* Allocate receive buffers. */
    if ( AllocateBuffers( pAdapter, pAdapter->m_nMTU,
            &pAdapter->m_RxBufInfo ) )
        return 1;

    /* Allocate transmit buffers. */
    if ( AllocateBuffers( pAdapter, pAdapter->m_nTxBufSize,
            &pAdapter->m_TxBufInfo ) )
        return 1;

    InitReceiveBuffers( pAdapter );

    return 0;
}  /* AllocateMemory */


/*
    FreeDescriptors

    Description:
        This local routine frees the software and hardware descriptors
        allocated by AllocateDescriptors().

    Parameters:
        struct dev_info* pAdapter
            Pointer to adapter information structure.

    Return (None):
*/

static void FreeDescriptors (
    struct dev_info* pAdapter )
{
    PHARDWARE pHardware = &pAdapter->hw;

    /* reset descriptor */
    pHardware->m_RxDescInfo.phwRing = NULL;
    pHardware->m_TxDescInfo.phwRing = NULL;
    pHardware->m_RxDescInfo.ulRing = 0;
    pHardware->m_TxDescInfo.ulRing = 0;

    /* Free memory */
    if ( pAdapter->m_DescPool.pAllocVirtual )
        dma_free_coherent(
            pAdapter->pdev,
            pAdapter->m_DescPool.ulAllocSize,
            pAdapter->m_DescPool.pAllocVirtual,
            pAdapter->m_DescPool.dma_addr );

    /* reset resource pool */
    pAdapter->m_DescPool.ulAllocSize = 0;
    pAdapter->m_DescPool.pAllocVirtual = NULL;

    if ( pHardware->m_RxDescInfo.pRing ) {
        kfree( pHardware->m_RxDescInfo.pRing );
        pHardware->m_RxDescInfo.pRing = NULL;
    }
    if ( pHardware->m_TxDescInfo.pRing ) {
        kfree( pHardware->m_TxDescInfo.pRing );
        pHardware->m_TxDescInfo.pRing = NULL;
    }
}  /* FreeDescriptors */


/*
    FreeBuffers

    Description:
        This local routine frees the DMA buffers allocated by
        AllocateBuffers().

    Parameters:
        struct dev_info* pAdapter
            Pointer to adapter information structure.

        PBUFFER_INFO pBufInfo
            Pointer to DMA buffer information structure.

    Return (None):
*/

static void FreeBuffers (
    struct dev_info* pAdapter,
    PBUFFER_INFO     pBufInfo )
{
    int         i;
    PDMA_BUFFER pDma;

    if ( !pBufInfo->BufArray )
        return;

    pDma = pBufInfo->BufArray;
    for ( i = 0; i < pBufInfo->cnAlloc; i++ )
    {
        if ( pDma->skb )
        {

#ifdef HIGH_MEM_SUPPORT
            dma_unmap_page(
                pAdapter->pdev, pDma->dma, pDma->len,
                DMA_FROM_DEVICE );

#else
            dma_unmap_single(
                pAdapter->pdev, pDma->dma, pDma->len,
                DMA_FROM_DEVICE );
#endif
            dev_kfree_skb( pDma->skb );
            pDma->skb = NULL;
            pDma->dma = 0;
        }
        pDma++;
    }
    kfree( pBufInfo->BufArray );
    pBufInfo->BufArray = NULL;
}  /* FreeBuffers */


/*
    FreeMemory

    Description:
        This local routine frees all the resources allocated by
        AllocateMemory().

    Parameters:
        struct dev_info* pAdapter
            Pointer to adapter information structure.

    Return (None):
*/

void FreeMemory (
    struct dev_info* pAdapter )
{
    /* Free transmit buffers. */
    FreeBuffers( pAdapter, &pAdapter->m_TxBufInfo );

    /* Free receive buffers */
    FreeBuffers( pAdapter, &pAdapter->m_RxBufInfo );

    /* Free descriptors. */
    FreeDescriptors( pAdapter );
}  /* FreeMemory */

/* -------------------------------------------------------------------------- */

static int display_mib_counters (
    PHARDWARE pHardware,
    char*     buf )
{
    int port;
    int len = 0;
    int num = pHardware->m_nRegCounter;

#ifdef CONFIG_KSZ8692VB
    if ( !buf )
        num += 2;
#endif
    for ( port = 0; port < num; port++ ) {
        if ( buf )
            len += sprintf( buf + len, "%2x = %-10lu  ", port,
                ( unsigned long )( pHardware->m_cnMIB[ port ]));
        else
            printk( "%2x = %-10lu  ", port,
                ( unsigned long )( pHardware->m_cnMIB[ port ]));
        if ( ( port % 4 ) == 3 ) {
            if ( buf )
                len += sprintf( buf + len, "\n" );
            else
                printk( "\n" );
        }
    }
    if ( ( port % 4 ) != 0 ) {
        if ( buf )
            len += sprintf( buf + len, "\n" );
        else
            printk( "\n" );
    }
    return len;
}  /* display_mib_counter */

/* -------------------------------------------------------------------------- */
#ifdef PROC_OLD_API

#ifdef CONFIG_KSZ8692VB
#define IN6ADDRSZ   16
#define INADDRSZ     4
#define INT16SZ      2


/* int
 * inet_ntop4(src, dst, size)
 *  format an IPv4 address, more or less like inet_ntoa()
 * return:
 *  `dst' (as a const)
 * notes:
 *  (1) uses no statics
 *  (2) takes a u_char* not an in_addr as input
 * author:
 *  Paul Vixie, 1996.
 */
static int
inet_ntop4 (
    const uint8_t *src,
    char          *dst,
    size_t        size )
{
    static const char fmt[] = "%u.%u.%u.%u";

    int len;
    char tmp[ sizeof "255.255.255.255" ];

    sprintf( tmp, fmt, src[ 0 ], src[ 1 ], src[ 2 ], src[ 3 ]);
    len = strlen( tmp );
    if ( len + 1 > size ) {
        return( 0 );
    }
    strcpy( dst, tmp );
    return( len );
}  /* inet_ntop4 */


/* int
 * inet_ntop6(src, dst, size)
 *  convert IPv6 binary address into presentation (printable) format
 * author:
 *  Paul Vixie, 1996.
 */
static int
inet_ntop6 (
    const uint8_t *src,
    char          *dst,
    size_t        size )
{
    /*
     * Note that int32_t and int16_t need only be "at least" large enough
     * to contain a value of the specified size.  On some systems, like
     * Crays, there is no such thing as an integer variable with 16 bits.
     * Keep this in mind if you think this function should have been coded
     * to use pointer overlays.  All the world's not a VAX.
     */
    char     tmp[ sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255" ];
    char     *tp;
    uint32_t words[ IN6ADDRSZ / INT16SZ ];
    int      i;
    int      len;
    struct { int base, len; } best, cur;

    /*
     * Preprocess:
     *  Copy the input (bytewise) array into a wordwise array.
     *  Find the longest run of 0x00's in src[] for :: shorthanding.
     */
    memset( words, 0, sizeof words );
    for ( i = 0; i < IN6ADDRSZ; i++ )
        words[ i / 2 ] |= ( src[ i ] << (( 1 - ( i % 2 )) << 3 ));
    best.len = 0;
    cur.len = 0;
    best.base = -1;
    cur.base = -1;
    for ( i = 0; i < ( IN6ADDRSZ / INT16SZ ); i++ ) {
        if ( words[ i ] == 0 ) {
            if ( cur.base == -1 )
                cur.base = i, cur.len = 1;
            else
                cur.len++;
        }
        else {
            if ( cur.base != -1 ) {
                if ( best.base == -1  ||  cur.len > best.len )
                    best = cur;
                cur.base = -1;
            }
        }
    }
    if ( cur.base != -1 ) {
        if ( best.base == -1  ||  cur.len > best.len )
            best = cur;
    }
    if ( best.base != -1  &&  best.len < 2 )
        best.base = -1;

    /*
     * Format the result.
     */
    tp = tmp;
    for ( i = 0; i < ( IN6ADDRSZ / INT16SZ ); i++ ) {

        /* Are we inside the best run of 0x00's? */
        if ( best.base != -1  &&  i >= best.base  &&
                i < ( best.base + best.len ) ) {
            if ( i == best.base )
                *tp++ = ':';
            continue;
        }

        /* Are we following an initial run of 0x00s or any real hex? */
        if ( i != 0 )
            *tp++ = ':';

        /* Is this address an encapsulated IPv4? */
        if ( i == 6  &&  best.base == 0  &&
                ( best.len == 6  ||  ( best.len == 5  &&
                words[ 5 ] == 0xffff )) ) {
            if ( !inet_ntop4( src + 12, tp, sizeof tmp - ( tp - tmp )) )
                return( 0 );
            tp += strlen( tp );
            break;
        }
        sprintf( tp, "%x", words[ i ]);
        tp += strlen( tp );
    }

    /* Was it a trailing run of 0x00's? */
    if ( best.base != -1  &&  ( best.base + best.len ) ==
            ( IN6ADDRSZ / INT16SZ ) )
        *tp++ = ':';
    *tp++ = '\0';

    /*
     * Check for overflow, copy, and we're done.
     */
    len = tp - tmp;
    if ( ( size_t ) len > size ) {
        return( 0 );
    }
    strcpy( dst, tmp );
    return( len - 1 );
}  /* inet_ntop6 */


/* int
 * inet_pton4(src, dst)
 *  like inet_aton() but without all the hexadecimal and shorthand.
 * return:
 *  1 if `src' is a valid dotted quad, else 0.
 * notice:
 *  does not touch `dst' unless it's returning 1.
 * author:
 *  Paul Vixie, 1996.
 */
static int
inet_pton4 (
    const char *src,
    uint8_t    *dst )
{
    static const char digits[] = "0123456789";

    int     saw_digit;
    int     octets;
    int     ch;
    uint8_t tmp[ INADDRSZ ];
    uint8_t *tp;

    saw_digit = 0;
    octets = 0;
    *( tp = tmp ) = 0;
    while ( ( ch = *src++ ) != '\0' ) {
        const char *pch;

        if ( ( pch = strchr( digits, ch )) != NULL ) {
            uint32_t new_word = *tp * 10 + ( pch - digits );

            if ( new_word > 255 )
                return( 0 );
            *tp = ( uint8_t ) new_word;
            if ( !saw_digit ) {
                if ( ++octets > 4 )
                    return( 0 );
                saw_digit = 1;
            }
        }
        else if ( ch == '.'  &&  saw_digit ) {
            if ( octets == 4 )
                return( 0 );
            *++tp = 0;
            saw_digit = 0;
        }
        else
            return( 0 );
    }
    if ( octets < 4 )
        return( 0 );
    memcpy( dst, tmp, INADDRSZ );
    return( 1 );
}  /* inet_pton4 */


/* int
 * inet_pton6(src, dst)
 *  convert presentation level address to network order binary form.
 * return:
 *  1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *  (1) does not touch `dst' unless it's returning 1.
 *  (2) :: in a full address is silently ignored.
 * credit:
 *  inspired by Mark Andrews.
 * author:
 *  Paul Vixie, 1996.
 */
static int
inet_pton6 (
    const char *src,
    uint8_t    *dst )
{
    static const char xdigits_l[] = "0123456789abcdef",
                      xdigits_u[] = "0123456789ABCDEF";

    uint8_t    tmp[ IN6ADDRSZ ];
    uint8_t    *tp;
    uint8_t    *endp;
    uint8_t    *colonp;
    const char *xdigits;
    const char *curtok;
    int        ch;
    int        saw_xdigit;
    uint32_t   val;

    memset(( tp = tmp ), 0, IN6ADDRSZ );
    endp = tp + IN6ADDRSZ;
    colonp = NULL;

    /* Leading :: requires some special handling. */
    if ( *src == ':' )
        if ( *++src != ':' )
            return( 0 );
    curtok = src;
    saw_xdigit = 0;
    val = 0;
    while ( ( ch = *src++ ) != '\0' ) {
        const char *pch;

        if ( ( pch = strchr(( xdigits = xdigits_l ), ch )) == NULL )
            pch = strchr(( xdigits = xdigits_u ), ch );
        if ( pch != NULL ) {
            val <<= 4;
            val |= ( pch - xdigits );
            if ( val > 0xffff )
                return( 0 );
            saw_xdigit = 1;
            continue;
        }
        if ( ch == ':' ) {
            curtok = src;
            if ( !saw_xdigit ) {
                if ( colonp )
                    return( 0 );
                colonp = tp;
                continue;
            }
            if ( tp + INT16SZ > endp )
                return( 0 );
            *tp++ = ( uint8_t )( val >> 8 );
            *tp++ = ( uint8_t ) val;
            saw_xdigit = 0;
            val = 0;
            continue;
        }
        if ( ch == '.'  &&  (( tp + INADDRSZ ) <= endp )  &&
                inet_pton4( curtok, tp ) > 0 ) {
            tp += INADDRSZ;
            saw_xdigit = 0;

            /* '\0' was seen by inet_pton4(). */
            break;
        }
        return( 0 );
    }
    if ( saw_xdigit ) {
        if ( tp + INT16SZ > endp )
            return( 0 );
        *tp++ = ( uint8_t )( val >> 8 );
        *tp++ = ( uint8_t ) val;
    }
    if ( colonp != NULL ) {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;
        int i;

        for ( i = 1; i <= n; i++ ) {
            endp[ - i ] = colonp[ n - i ];
            colonp[ n - i ] = 0;
        }
        tp = endp;
    }
    if ( tp != endp )
        return( 0 );
    memcpy( dst, tmp, IN6ADDRSZ );
    return( 1 );
}  /* inet_pton6 */
#endif

/* -------------------------------------------------------------------------- */

#define proc_dir_name  "driver/kszeth"


static TProcInfo ProcInfo[ PROC_LAST ] = {

    /* Read-only entries. */
    { PROC_INFO,                    "info",                 },
    { PROC_GET_CABLE_STATUS,        "cable_status",         },
    { PROC_GET_LINK_STATUS,         "link_status",          },

    /* Read-write entries. */
    { PROC_SET_DUPLEX,              "duplex",               },
    { PROC_SET_SPEED,               "speed",                },
    { PROC_SET_CAPABILITIES,        "capabilities",         },
    { PROC_SET_MIB,                 "mib",                  },

#ifdef CONFIG_KSZ8692VB
    { PROC_SET_BROADCAST_ENABLE,    "bcast_enable",         },
    { PROC_SET_BROADCAST_COUNTER,   "bcast_cnt",            },

    { PROC_SET_DIFFSERV_ENABLE,     "diffserv_enable",      },
    { PROC_SET_DIFFSERV_MAPPING0,   "diffserv_mapping0",    },
    { PROC_SET_DIFFSERV_MAPPING1,   "diffserv_mapping1",    },
    { PROC_SET_DIFFSERV_SET,        "diffserv_set",         },
    { PROC_SET_DIFFSERV_UNSET,      "diffserv_unset",       },

    { PROC_SET_802_1P_ENABLE,       "802_1p_enable",        },
    { PROC_SET_802_1P_MAPPING,      "802_1p_mapping",       },
    { PROC_SET_802_1P_SET,          "802_1p_set",           },
    { PROC_SET_802_1P_UNSET,        "802_1p_unset",         },

    /* ACL specific read-write entries. */
    { PROC_SET_ACL_BLOCK_UNMATCHED, "block",                },

    { PROC_SET_ACL_ENABLE,          "enable",               },
    { PROC_SET_ACL_RX_HI_PRIORITY,  "hi_priority",          },
    { PROC_SET_ACL_FILTER_MATCHED,  "filter",               },
    { PROC_SET_ACL_MODE,            "mode",                 },

    { PROC_SET_ACL_DATA,            "data",                 },
    { PROC_SET_ACL_MASK,            "mask",                 },
    { PROC_SET_ACL_MAC,             "mac",                  },
    { PROC_SET_ACL_OFFSET,          "offset",               },
    { PROC_SET_ACL_PROTOCOL,        "protocol",             },
    { PROC_SET_ACL_PORT,            "port",                 },
    { PROC_SET_ACL_IPV4_ADDR,       "ip4_addr",             },
    { PROC_SET_ACL_IPV4_MASK,       "ip4_mask",             },
    { PROC_SET_ACL_IPV6_ADDR,       "ip6_addr",             },
    { PROC_SET_ACL_IPV6_MASK,       "ip6_mask",             },
#endif

    /* Port specific read-only entries. */

    /* Port specific read-write entries. */
};


/*
    read_proc

    Description:
        This function process the read operation of /proc files.

    Parameters:

    Return (int):
        The length of buffer data returned.
*/

static
int read_proc (
    char*  buf,
    char** start,
    off_t  offset,
    int    count,
    int*   eof,
    void*  data )
{
    int              len = 0;
    int              processed = TRUE;
    int              rc;
    PTProcPortInfo   proc = ( PTProcPortInfo ) data;
    PTProcInfo       info = proc->info;
    struct dev_info* priv = proc->priv;
    PHARDWARE        pHardware = &priv->hw;
#ifdef CONFIG_KSZ8692VB
    PTAclInfo        acl = &pHardware->AclInfo[ proc->port ];
#endif
    DWORD            dwData[ 5 * 2 ];

    /* Why want more? */
    if ( offset ) {
        *eof = 1;
        return( 0 );
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_INC_USE_COUNT;
#endif

    if ( down_interruptible( &priv->proc_sem ) ) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
        MOD_DEC_USE_COUNT;
#endif
        return -ERESTARTSYS;
    }

#if 0
    len += sprintf( buf + len, "%s:%d %d\n", info->proc_name, info->proc_num,
        proc->port );
#endif
    switch ( info->proc_num ) {
        case PROC_SET_DUPLEX:
            len += sprintf( buf + len, "%u; ",
                pHardware->m_bDuplex );
            if ( MediaStateConnected == pHardware->m_ulHardwareState ) {
                if ( 1 == pHardware->m_ulDuplex )
                    len += sprintf( buf + len, "half-duplex\n" );
                else if ( 2 == pHardware->m_ulDuplex )
                    len += sprintf( buf + len, "full-duplex\n" );
            }
            else
                len += sprintf( buf + len, "unlinked\n" );
            break;
        case PROC_SET_SPEED:
            len += sprintf( buf + len, "%u; ",
                pHardware->m_wSpeed );
            if ( MediaStateConnected == pHardware->m_ulHardwareState ) {
                len += sprintf( buf + len, "%u\n",
                    pHardware->m_ulTransmitRate / 10000 );
            }
            else
                len += sprintf( buf + len, "unlinked\n" );
            break;
        case PROC_SET_MIB:
            priv->Counter.fRead = 1;
            rc = interruptible_sleep_on_timeout(
                &priv->Counter.wqhCounter, HZ * 2 );
            len += display_mib_counters( pHardware, buf + len );
            break;

#ifdef CONFIG_KSZ8692VB
        case PROC_SET_BROADCAST_ENABLE:
            len += sprintf( buf + len, "%u; ",
                pHardware->m_fBroadcast );
            if ( pHardware->m_fBroadcast )
                len += sprintf( buf + len, "enabled\n" );
            else
                len += sprintf( buf + len, "disabled\n" );
            break;
        case PROC_SET_BROADCAST_COUNTER:
            len += sprintf( buf + len, "%u\n",
                pHardware->m_bBroadcastCounter );
            break;
        case PROC_SET_DIFFSERV_ENABLE:
            len += sprintf( buf + len, "%u; ",
                pHardware->m_fDiffServ );
            if ( pHardware->m_fDiffServ )
                len += sprintf( buf + len, "enabled\n" );
            else
                len += sprintf( buf + len, "disabled\n" );
            break;
        case PROC_SET_DIFFSERV_MAPPING0:
            len += sprintf( buf + len, "0x%08x\n", pHardware->m_dwDiffServ0 );
            break;
        case PROC_SET_DIFFSERV_MAPPING1:
            len += sprintf( buf + len, "0x%08x\n", pHardware->m_dwDiffServ1 );
            break;
        case PROC_SET_DIFFSERV_SET:
        case PROC_SET_DIFFSERV_UNSET:
            break;
        case PROC_SET_802_1P_ENABLE:
            len += sprintf( buf + len, "%u; ",
                pHardware->m_f802_1p );
            if ( pHardware->m_f802_1p )
                len += sprintf( buf + len, "enabled\n" );
            else
                len += sprintf( buf + len, "disabled\n" );
            break;
        case PROC_SET_802_1P_MAPPING:
            len += sprintf( buf + len, "0x%02x\n",
                pHardware->m_b802_1p_mapping );
            break;
        case PROC_SET_802_1P_SET:
        case PROC_SET_802_1P_UNSET:
            break;
        case PROC_SET_ACL_BLOCK_UNMATCHED:
            len += sprintf( buf + len, "%u\n", pHardware->m_fBlock );
            break;
        case PROC_SET_ACL_ENABLE:
            len += sprintf( buf + len, "%u\n", acl->enable );
            break;
        case PROC_SET_ACL_RX_HI_PRIORITY:
            len += sprintf( buf + len, "%u\n", acl->priority );
            break;
        case PROC_SET_ACL_FILTER_MATCHED:
            len += sprintf( buf + len, "%u\n", acl->filter );
            break;
        case PROC_SET_ACL_MODE:
            len += sprintf( buf + len, "%u;\n", acl->mode );
            len += sprintf( buf + len, "\t0 = mac\n" );
            len += sprintf( buf + len, "\t1 = offset\n" );
            len += sprintf( buf + len, "\t2 = protocol\n" );
            len += sprintf( buf + len, "\t3 = destination port\n" );
            len += sprintf( buf + len, "\t4 = source port\n" );
            len += sprintf( buf + len, "\t5 = both ports\n" );
            len += sprintf( buf + len, "\t6 = destination IPv4 address\n" );
            len += sprintf( buf + len, "\t7 = source IPv4 address \n" );
            len += sprintf( buf + len, "\t8 = both IPv4 addresses\n" );
            if ( ( acl->acl % 4 ) )
                break;
            len += sprintf( buf + len, "\t9 = destination IPv6 address\n" );
            len += sprintf( buf + len, "\ta = source IPv6 address \n" );
            len += sprintf( buf + len, "\tb = both IPv6 addresses\n" );
            break;
        case PROC_SET_ACL_DATA:
            len += sprintf( buf + len, "0x%08x\n", acl->data );
            break;
        case PROC_SET_ACL_MASK:
            len += sprintf( buf + len, "0x%08x\n", acl->mask );
            break;
        case PROC_SET_ACL_MAC:
            len += sprintf( buf + len,
                "%02x:%02x:%02x:%02x:%02x:%02x\n",
                acl->mac[ 0 ], acl->mac[ 1 ], acl->mac[ 2 ],
                acl->mac[ 3 ], acl->mac[ 4 ], acl->mac[ 5 ]);
            break;
        case PROC_SET_ACL_OFFSET:
            len += sprintf( buf + len, "%x\n", acl->offset );
            break;
        case PROC_SET_ACL_PROTOCOL:
            len += sprintf( buf + len, "%x\n", acl->protocol );
            break;
        case PROC_SET_ACL_PORT:
            len += sprintf( buf + len, "%x\n", acl->port );
            break;
        case PROC_SET_ACL_IPV4_ADDR:
            len += inet_ntop4( acl->ip4_addr, buf + len, count - len );
            len += sprintf( buf + len, "\n" );
            break;
        case PROC_SET_ACL_IPV4_MASK:
            len += inet_ntop4( acl->ip4_mask, buf + len, count - len );
            len += sprintf( buf + len, "\n" );
            break;
        case PROC_SET_ACL_IPV6_ADDR:
            len += inet_ntop6( acl->ip6_addr, buf + len, count - len );
            len += sprintf( buf + len, "\n" );
            break;
        case PROC_SET_ACL_IPV6_MASK:
            len += inet_ntop6( acl->ip6_mask, buf + len, count - len );
            len += sprintf( buf + len, "\n" );
            break;
#endif
        default:
            processed = FALSE;
    }

    /* Require hardware to be acquired first. */
    if ( !processed ) {
        if ( ( rc = AcquireHardware( priv, FALSE, TRUE )) ) {
            up( &priv->proc_sem );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
            MOD_DEC_USE_COUNT;
#endif
            return( rc );
        }
        switch ( info->proc_num ) {
            case PROC_GET_CABLE_STATUS:
                HardwareGetCableStatus( pHardware, dwData );
                len += sprintf( buf + len,
                    "^[%u:%u, %u:%u, %u:%u, %u:%u, %u:%u]$\n",
                    dwData[ 0 ], dwData[ 1 ],
                    dwData[ 2 ], dwData[ 3 ],
                    dwData[ 4 ], dwData[ 5 ],
                    dwData[ 6 ], dwData[ 7 ],
                    dwData[ 8 ], dwData[ 9 ]);
                break;
            case PROC_GET_LINK_STATUS:
                HardwareGetLinkStatus( pHardware, dwData );
                len += sprintf( buf + len,
                    "^[%08X, %X, %08X, %08X, %08X, %08X]$\n",
                    dwData[ 0 ], dwData[ 1 ], dwData[ 2 ],
                    dwData[ 3 ], dwData[ 4 ], dwData[ 5 ]);
                break;
            default:
                break;
        }
        ReleaseHardware( priv, FALSE );
    }
    up( &priv->proc_sem );
    *eof = 1;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_DEC_USE_COUNT;
#endif

    return len;
}  /* read_proc */


/*
    write_proc

    Description:
        This function process the write operation of /proc files.

    Parameters:

    Return (int):
        The length of buffer data accepted.
*/

static
int write_proc (
    struct file*  file,
    const char*   buffer,
    unsigned long count,
    void*         data )
{
    int              len;
    int              num;
    int              rc;
    char             str[ 44 ];
    PTProcPortInfo   proc = ( PTProcPortInfo ) data;
    PTProcInfo       info = proc->info;
    struct dev_info* priv = proc->priv;
    PHARDWARE        pHardware = &priv->hw;
#ifdef CONFIG_KSZ8692VB
    PTAclInfo        acl = &pHardware->AclInfo[ proc->port ];
    BYTE             addr[ 16 ];
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_INC_USE_COUNT;
#endif

    if ( count > 40 )
        len = 40;
    else
        len = count;
    if ( copy_from_user( str, buffer, len ) ) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
        MOD_DEC_USE_COUNT;
#endif
        return -EFAULT;
    }
    str[ len ] = '\0';
    if ( '\n' == str[ len - 1 ] )
        str[ len - 1 ] = '\0';
    if ( down_interruptible( &priv->proc_sem ) ) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
        MOD_DEC_USE_COUNT;
#endif
        return -ERESTARTSYS;
    }
    if ( ( rc = AcquireHardware( priv, FALSE, TRUE )) ) {
        up( &priv->proc_sem );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
        MOD_DEC_USE_COUNT;
#endif
        return( rc );
    }
    if ( '0' == str[ 0 ]  &&  'x' == str[ 1 ] ) {
        sscanf( &str[ 2 ], "%x", ( unsigned int* ) &num );
    }
    else if ( '0' == str[ 0 ]  &&  'b' == str[ 1 ] ) {
        int i = 2;

        num = 0;
        while ( str[ i ] ) {
            num <<= 1;
            num |= str[ i ] - '0';
            i++;
        }
    }
    else if ( '0' == str[ 0 ]  &&  'd' == str[ 1 ] ) {
        sscanf( &str[ 2 ], "%u", &num );
    }
    else
        sscanf( str, "%d", &num );
    switch ( info->proc_num ) {
        case PROC_SET_DUPLEX:
            if ( num <= 2 )
                pHardware->m_bDuplex = ( BYTE ) num;
            break;
        case PROC_SET_SPEED:
            if ( 0 == num  ||  10 == num  ||  100 == num  ||  1000 == num )
                pHardware->m_wSpeed = ( USHORT ) num;
            break;
        case PROC_SET_CAPABILITIES:
            HardwareSetCapabilities( pHardware, num );
            break;
        case PROC_SET_MIB:
            break;

#ifdef CONFIG_KSZ8692VB
        case PROC_SET_BROADCAST_ENABLE:
            if ( 0 == num )
                pHardware->m_fBroadcast = 0;
            else
                pHardware->m_fBroadcast = 1;
            HardwareSetBroadcastTraffic( pHardware,
                pHardware->m_fBroadcast,
                pHardware->m_bBroadcastCounter );
            break;
        case PROC_SET_BROADCAST_COUNTER:
            if ( num < 0x100 ) {
                pHardware->m_bBroadcastCounter = ( UCHAR ) num;
                if ( pHardware->m_fBroadcast )
                    HardwareSetBroadcastTraffic( pHardware,
                        pHardware->m_fBroadcast,
                        pHardware->m_bBroadcastCounter );
            }
            break;
        case PROC_SET_DIFFSERV_ENABLE:
            if ( 0 == num )
                pHardware->m_fDiffServ = 0;
            else
                pHardware->m_fDiffServ = 1;
            HardwareSetDiffServPriority( pHardware,
                pHardware->m_fDiffServ,
                pHardware->m_dwDiffServ0, pHardware->m_dwDiffServ1 );
            break;
        case PROC_SET_DIFFSERV_MAPPING0:
            if ( '0' != str[ 0 ]  ||  'x' != str[ 1 ] )
                sscanf( str, "%x", &num );
            pHardware->m_dwDiffServ0 = num;
            if ( pHardware->m_fDiffServ )
                HardwareSetDiffServPriority( pHardware,
                    pHardware->m_fDiffServ,
                    pHardware->m_dwDiffServ0, pHardware->m_dwDiffServ1 );
            break;
        case PROC_SET_DIFFSERV_MAPPING1:
            if ( '0' != str[ 0 ]  ||  'x' != str[ 1 ] )
                sscanf( str, "%x", &num );
            pHardware->m_dwDiffServ1 = num;
            if ( pHardware->m_fDiffServ )
                HardwareSetDiffServPriority( pHardware,
                    pHardware->m_fDiffServ,
                    pHardware->m_dwDiffServ0, pHardware->m_dwDiffServ1 );
            break;
        case PROC_SET_DIFFSERV_SET:
            if ( num < 64 ) {
                if ( num < 32 )
                    pHardware->m_dwDiffServ0 |= 1 << num;
                else
                    pHardware->m_dwDiffServ1 |= 1 << ( num - 32 );
                if ( pHardware->m_fDiffServ )
                    HardwareSetDiffServPriority( pHardware,
                        pHardware->m_fDiffServ,
                        pHardware->m_dwDiffServ0, pHardware->m_dwDiffServ1 );
            }
            break;
        case PROC_SET_DIFFSERV_UNSET:
            if ( num < 64 ) {
                if ( num < 32 )
                    pHardware->m_dwDiffServ0 &= ~( 1 << num );
                else
                    pHardware->m_dwDiffServ1 &= ~( 1 << ( num - 32 ));
                if ( pHardware->m_fDiffServ )
                    HardwareSetDiffServPriority( pHardware,
                        pHardware->m_fDiffServ,
                        pHardware->m_dwDiffServ0, pHardware->m_dwDiffServ1 );
            }
            break;
        case PROC_SET_802_1P_ENABLE:
            if ( 0 == num )
                pHardware->m_f802_1p = 0;
            else
                pHardware->m_f802_1p = 1;
            HardwareSet802_1P_Priority( pHardware,
                pHardware->m_f802_1p, pHardware->m_b802_1p_mapping );
            break;
        case PROC_SET_802_1P_MAPPING:
            if ( '0' != str[ 0 ]  ||  'x' != str[ 1 ] )
                sscanf( str, "%x", &num );
            if ( num < 0x100 ) {
                pHardware->m_b802_1p_mapping = ( UCHAR ) num;
                if ( pHardware->m_f802_1p )
                    HardwareSet802_1P_Priority( pHardware,
                        pHardware->m_f802_1p, pHardware->m_b802_1p_mapping );
            }
            break;
        case PROC_SET_802_1P_SET:
            if ( num < 8 ) {
                pHardware->m_b802_1p_mapping |= 1 << num;
                if ( pHardware->m_f802_1p )
                    HardwareSet802_1P_Priority( pHardware,
                        pHardware->m_f802_1p, pHardware->m_b802_1p_mapping );
            }
            break;
        case PROC_SET_802_1P_UNSET:
            if ( num < 8 ) {
                pHardware->m_b802_1p_mapping &= ~( 1 << num );
                if ( pHardware->m_f802_1p )
                    HardwareSet802_1P_Priority( pHardware,
                        pHardware->m_f802_1p, pHardware->m_b802_1p_mapping );
            }
            break;
        case PROC_SET_ACL_BLOCK_UNMATCHED:
            if ( 0 == num )
                pHardware->m_fBlock = 0;
            else
                pHardware->m_fBlock = 1;
            HardwareSet_ACL_Block( pHardware, pHardware->m_fBlock );
            break;
        case PROC_SET_ACL_ENABLE:
        {
            int ip6_acl = acl->acl & ~3;

            if ( ip6_acl != acl->acl ) {
                PTAclInfo ip6 = &pHardware->AclInfo[ ip6_acl ];

                if ( ACL_MODE_IPV6_DST <= ip6->mode  &&
                        ip6->mode <= ACL_MODE_IPV6_BOTH ) {
                    if ( ip6->enable )
                        break;
                    if ( 0 != num )
                        ip6->changed = 1;
                }
            }
            if ( 0 == num )
                acl->enable = 0;
            else
                acl->enable = 1;
            if ( acl->enable  &&  acl->changed )
                HardwareSetupACL( pHardware, acl, acl->mode, acl->mode );
            else {
                HardwareSet_ACL_Enable( pHardware, acl->acl, acl->enable );
                if ( !( acl->acl % 4 ) ) {
                    HardwareSet_ACL_V6( pHardware, acl->acl, acl->enable );
                }
            }
            break;
        }
        case PROC_SET_ACL_FILTER_MATCHED:
            if ( 0 == num )
                acl->filter = 0;
            else
                acl->filter = 1;
            if ( acl->enable )
                HardwareSet_ACL_Filter( pHardware, acl->acl, acl->filter );
            break;
        case PROC_SET_ACL_RX_HI_PRIORITY:
            if ( 0 == num )
                acl->priority = 0;
            else
                acl->priority = 1;
            if ( acl->enable )
                HardwareSet_ACL_Priority( pHardware, acl->acl, acl->priority );
            break;
        case PROC_SET_ACL_MODE:
        {
            int last_mode = acl->mode;

            if ( '0' != str[ 0 ]  ||  'x' != str[ 1 ] )
                sscanf( str, "%x", &num );
            if ( ACL_MODE_MAC <= num  &&  num <= ACL_MODE_IPV4_BOTH )
                acl->mode = num;
            else if ( num <= ACL_MODE_IPV6_BOTH  &&  !( acl->acl % 4 ) )
                acl->mode = num;
            if ( last_mode != acl->mode )
                HardwareSetupACL( pHardware, acl, acl->mode, acl->mode );
            break;
        }
        case PROC_SET_ACL_DATA:
            if ( '0' != str[ 0 ]  ||  'x' != str[ 1 ] )
                sscanf( str, "%x", &num );
            acl->data = num;
            HardwareSetupACL( pHardware, acl,
                ACL_MODE_OFFSET, ACL_MODE_OFFSET );
            break;
        case PROC_SET_ACL_MASK:
            if ( '0' != str[ 0 ]  ||  'x' != str[ 1 ] )
                sscanf( str, "%x", &num );
            acl->mask = num;
            HardwareSetupACL( pHardware, acl,
                ACL_MODE_OFFSET, ACL_MODE_PORT_BOTH );
            break;
        case PROC_SET_ACL_MAC:
        {
            int i;
            int j;
            char* ch;

            ch = str;
            for ( i = 0; i < 6; i++ ) {
                num = 0;
                for ( j = 0; j < 2; j++ ) {
                    num *= 16;
                    if ( '0' <= *ch  &&  *ch <= '9' )
                        num += *ch - '0';
                    else if ( 'a' <= *ch  &&  *ch <= 'f' )
                        num += *ch - 'a' + 10;
                    else if ( 'A' <= *ch  &&  *ch <= 'F' )
                        num += *ch - 'A' + 10;
                    else
                        break;
                    ch++;
                }
                if ( j < 2 )
                    break;
                if ( i != 5 ) {
                    if ( ':' != *ch )
                        break;
                    ch++;
                }
                else {
                    if ( ( '0' <= *ch  &&  *ch <= '9' )  ||
                            ( 'a' <= *ch  &&  *ch <= 'f' )  ||
                            ( 'A' <= *ch  &&  *ch <= 'F' )  ||
                            ':' == *ch )
                        break;
                }
                addr[ i ] = ( BYTE ) num;
            }
            if ( 6 == i ) {
                memcpy( acl->mac, addr, 6 );
                HardwareSetupACL( pHardware, acl,
                    ACL_MODE_MAC, ACL_MODE_MAC );
            }
            break;
        }
        case PROC_SET_ACL_OFFSET:
            if ( '0' != str[ 0 ]  ||  'x' != str[ 1 ] )
                sscanf( str, "%x", &num );
            if ( 0 <= num  &&  num < 0x40 ) {
                acl->offset = num;
                HardwareSetupACL( pHardware, acl,
                    ACL_MODE_OFFSET, ACL_MODE_OFFSET );
            }
            break;
        case PROC_SET_ACL_PROTOCOL:
            if ( '0' != str[ 0 ]  ||  'x' != str[ 1 ] )
                sscanf( str, "%x", &num );
            if ( 0 <= num  &&  num < 0x100 ) {
                acl->protocol = num;
                HardwareSetupACL( pHardware, acl,
                    ACL_MODE_PROTOCOL, ACL_MODE_PROTOCOL );
            }
            break;
        case PROC_SET_ACL_PORT:
            if ( '0' != str[ 0 ]  ||  'x' != str[ 1 ] )
                sscanf( str, "%x", &num );
            if ( 0 <= num  &&  num < 0x10000 ) {
                acl->port = num;
                HardwareSetupACL( pHardware, acl,
                    ACL_MODE_PORT_DST, ACL_MODE_PORT_BOTH );
            }
            break;
        case PROC_SET_ACL_IPV4_ADDR:
            if ( inet_pton4( str, addr ) ) {
                memcpy( acl->ip4_addr, addr, 4 );
                HardwareSetupACL( pHardware, acl,
                    ACL_MODE_IPV4_DST, ACL_MODE_IPV4_BOTH );
            }
            break;
        case PROC_SET_ACL_IPV4_MASK:
            if ( inet_pton4( str, addr ) ) {
                memcpy( acl->ip4_mask, addr, 4 );
                HardwareSetupACL( pHardware, acl,
                    ACL_MODE_IPV4_DST, ACL_MODE_IPV4_BOTH );
            }
            break;
        case PROC_SET_ACL_IPV6_ADDR:
            if ( inet_pton6( str, addr ) ) {
                memcpy( acl->ip6_addr, addr, 16 );
                HardwareSetupACL( pHardware, acl,
                    ACL_MODE_IPV6_DST, ACL_MODE_IPV6_BOTH );
            }
            break;
        case PROC_SET_ACL_IPV6_MASK:
            if ( inet_pton6( str, addr ) ) {
                memcpy( acl->ip6_mask, addr, 16 );
                HardwareSetupACL( pHardware, acl,
                    ACL_MODE_IPV6_DST, ACL_MODE_IPV6_BOTH );
            }
            break;
#endif
        default:
            printk( KERN_ALERT "write_proc:%d\n", info->proc_num );
    }
    ReleaseHardware( priv, FALSE );
    up( &priv->proc_sem );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_DEC_USE_COUNT;
#endif

    return len;
}  /* write_proc */

#endif /* PROC_OLD_API */
/* -------------------------------------------------------------------------- */

#ifdef CONFIG_NET_POLL_CONTROLLER
static void netdev_netpoll ( struct net_device* dev )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;

    HardwareDisableInterruptSync( &hw_priv->hw );
    DEV_INTERRUPT_PTR( dev->irq, dev );
}  /* netdev_netpoll */
#endif


/*
    netdev_close

    Description:
        This function process the close operation of network device.  This is
        caused by the user command "ifconfig ethX down."

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

static int netdev_close (
    struct net_device* dev )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              irq;
    int              rc;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_DEC_USE_COUNT;
#endif
    priv->opened--;

    if ( !( priv->opened ) ) {
        if ( priv->skb_waiting ) {
            priv->stats.tx_dropped++;
            dev_kfree_skb( priv->skb_waiting );
            priv->skb_waiting = NULL;
        }
        netif_stop_queue( dev );

        if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
            return( rc );
        }

        if ( hw_priv->MonitorTimerInfo.nCount ) {
            StopTimer( &hw_priv->MonitorTimerInfo );
        }

#ifdef CONFIG_DELAYED_TIMER
        tasklet_delayed_kill( &hw_priv->delay_rx_taskinfo );
        tasklet_disable( &hw_priv->delay_rx_taskinfo.tasklet );
#endif

        HardwareDisableInterrupt( pHardware );
        HardwareDisable( pHardware );

        /* Delay for receive task to stop scheduling itself. */
        DelayMillisec( 2000 / HZ );

#ifdef AVOID_TX_FAILURE
        tasklet_disable( &hw_priv->reset_tasklet );
#endif

        tasklet_disable( &hw_priv->rx_tasklet );
        tasklet_disable( &hw_priv->tx_tasklet );
        for ( irq = 0; irq < 6; irq++ ) {
            free_irq( dev->irq + irq, dev );
        }

        transmit_reset( dev );
        HardwareResetRxPackets( pHardware );
        HardwareResetTxPackets( pHardware );
        InitBuffers( &hw_priv->m_RxBufInfo );
        InitBuffers( &hw_priv->m_TxBufInfo );
        InitReceiveBuffers( hw_priv );

        ReleaseHardware( hw_priv, FALSE );
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 21))
#ifdef CONFIG_SIM_IPSEC
    if ( hw_priv->hw.m_nPhyShift )
        test_ipsec();
#endif
#endif

#ifdef DBG_
{
    int i;

    DBG_PRINT( "counters:\n" );
    for ( i = OID_COUNTER_FIRST; i < OID_COUNTER_LAST; i++ )
    {
        DBG_PRINT( "%u = %u\n", i,
            ( int ) pHardware->m_cnCounter[ i ]);
    }
    DBG_PRINT( "wait delays:\n" );
    for ( i = WAIT_DELAY_FIRST; i < WAIT_DELAY_LAST; i++ )
    {
        DBG_PRINT( "%u = %u\n", i, pHardware->m_nWaitDelay[ i ]);
    }
    DBG_PRINT( "bad:\n" );
    for ( i = COUNT_BAD_FIRST; i < COUNT_BAD_LAST; i++ )
    {
        DBG_PRINT( "%u = %u\n", i, ( int ) pHardware->m_nBad[ i ]);
    }
    DBG_PRINT( "good:\n" );
    for ( i = COUNT_GOOD_FIRST; i < COUNT_GOOD_LAST; i++ )
    {
        DBG_PRINT( "%u = %u\n", i, ( int ) pHardware->m_nGood[ i ]);
    }
}
#endif

    return 0;
}  /* netdev_close */


/*
    netdev_open

    Description:
        This function process the open operation of network device.  This is
        caused by the user command "ifconfig ethX up."

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

static int netdev_open (
    struct net_device* dev )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              rc = 0;
    int              irq;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_INC_USE_COUNT;
#endif

    if ( !( priv->opened ) ) {

        /* Reset device statistics. */
        memset( &priv->stats, 0, sizeof( struct net_device_stats ));

        if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
            MOD_DEC_USE_COUNT;
#endif
            return( rc );
        }

        HardwareReset( pHardware );

        for ( irq = 0; irq < 6; irq++ ) {
            switch ( irq ) {
                case 0:
                    sprintf( hw_priv->irq_name[ irq ],
                        "%s rx stopped", dev->name );
                    break;
                case 1:
                    sprintf( hw_priv->irq_name[ irq ],
                        "%s tx stopped", dev->name );
                    break;
                case 2:
                    sprintf( hw_priv->irq_name[ irq ],
                        "%s rx buffer empty", dev->name );
                    break;
                case 3:
                    sprintf( hw_priv->irq_name[ irq ],
                        "%s tx buffer empty", dev->name );
                    break;
                case 4:
                    sprintf( hw_priv->irq_name[ irq ],
                        "%s rx", dev->name );
                    break;
                case 5:
                    sprintf( hw_priv->irq_name[ irq ],
                        "%s tx", dev->name );
                    break;
                default:
                    break;
            }
            if ( ( rc = request_irq( dev->irq + irq, DEV_INTERRUPT_PTR,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
                    IRQF_DISABLED,
#else
                    SA_INTERRUPT,
#endif
                    hw_priv->irq_name[ irq ], dev )) ) {
                while ( irq-- )
                    free_irq( dev->irq + irq, dev );
                ReleaseHardware( hw_priv, FALSE );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
                MOD_DEC_USE_COUNT;
#endif
                return( rc );
            }
        }
        tasklet_enable( &hw_priv->rx_tasklet );
        tasklet_enable( &hw_priv->tx_tasklet );

#ifdef AVOID_TX_FAILURE
        tasklet_enable( &hw_priv->reset_tasklet );
#endif

#ifdef CONFIG_DELAYED_TIMER
        tasklet_enable( &hw_priv->delay_rx_taskinfo.tasklet );
#endif

        pHardware->m_bPromiscuous = FALSE;
        pHardware->m_bAllMulticast = FALSE;
        pHardware->m_bMulticastListSize = 0;
        pHardware->m_ulHardwareState = MediaStateDisconnected;

        hw_priv->Counter.time = jiffies + HZ * 4;

        HardwareSetup( pHardware );
        HardwareSetDescriptorBase( pHardware,
            pHardware->m_TxDescInfo.ulRing,
            pHardware->m_RxDescInfo.ulRing );
        HardwareGetLinkSpeed( pHardware );

#ifdef AVOID_TX_FAILURE
        hw_priv->gigabit_rate = FALSE;
        if ( 10000000 == pHardware->m_ulTransmitRate ) {
            hw_priv->gigabit_rate = TRUE;
        }
#endif
        HardwareSetLinkSpeed( pHardware );

        HardwareSetupInterrupt( pHardware );
        HardwareEnable( pHardware );
        HardwareEnableInterrupt( pHardware );

        StartTimer( &hw_priv->MonitorTimerInfo,
            hw_priv->MonitorTimerInfo.nTime );

        memset(( void* ) pHardware->m_cnCounter, 0,
            ( sizeof( ULONGLONG ) * OID_COUNTER_LAST ) );
        HardwareInitCounters( pHardware );

        priv->media_state = pHardware->m_ulHardwareState;
#ifndef CONFIG_PEGASUS
        priv->media_state = MediaStateConnected;
#endif

        ReleaseHardware( hw_priv, FALSE );

        if ( MediaStateConnected == priv->media_state )
            netif_carrier_on( dev );
        else
            netif_carrier_off( dev );
        if ( netif_msg_link( priv ) ) {
            printk(KERN_INFO "%s link %s\n", dev->name,
                ( MediaStateConnected == priv->media_state ? "on" : "off" ));
        }

        netif_start_queue( dev );
    }
    priv->opened++;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 21))
#ifdef CONFIG_SIM_IPSEC
    if ( hw_priv->hw.m_nPhyShift )
        test_ipsec();
#endif
#endif

    return 0;
}  /* netdev_open */


/* RX errors = rx_errors */
/* RX dropped = rx_dropped */
/* RX overruns = rx_fifo_errors */
/* RX frame = rx_crc_errors + rx_frame_errors + rx_length_errors */
/* TX errors = tx_errors */
/* TX dropped = tx_dropped */
/* TX overruns = tx_fifo_errors */
/* TX carrier = tx_aborted_errors + tx_carrier_errors + tx_window_errors */
/* collisions = collisions */

/*
    dev_query_statistics

    Description:
        This function returns the statistics of the network device.  The device
        needs not be opened.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (struct net_device_stat*):
        Network device statistics.
*/

static struct net_device_stats* dev_query_statistics (
    struct net_device* dev )
{
    struct dev_priv* priv = netdev_priv(dev);
    PHARDWARE        pHardware = &priv->pDevInfo->hw;

    priv->stats.rx_errors = pHardware->m_cnCounter
        [ OID_COUNTER_RCV_ERROR ];
    priv->stats.tx_errors = pHardware->m_cnCounter
        [ OID_COUNTER_XMIT_ERROR ];
#if 0
    priv->stats.tx_carrier_errors =
        pHardware->m_cnCounter[ OID_COUNTER_XMIT_LOST_CARRIER ];
#endif

    priv->stats.multicast = ( unsigned long ) pHardware->m_cnMIB[
        MIB_COUNTER_RX_MULTICAST ];

    priv->stats.collisions = ( unsigned long ) pHardware->m_cnMIB[
        MIB_COUNTER_TX_TOTAL_COLLISION ];

    priv->stats.rx_length_errors = ( unsigned long )( pHardware->m_cnMIB[
        MIB_COUNTER_RX_UNDERSIZE ] + pHardware->m_cnMIB[
        MIB_COUNTER_RX_FRAGMENT ] + pHardware->m_cnMIB[
        MIB_COUNTER_RX_OVERSIZE ] + pHardware->m_cnMIB[
        MIB_COUNTER_RX_JABBER ]);
    priv->stats.rx_crc_errors = ( unsigned long ) pHardware->m_cnMIB[
        MIB_COUNTER_RX_CRC_ERR ];
    priv->stats.rx_frame_errors = ( unsigned long )( pHardware->m_cnMIB[
        MIB_COUNTER_RX_ALIGNMENT_ERR ] + pHardware->m_cnMIB[
        MIB_COUNTER_RX_SYMBOL_ERR ]);

    priv->stats.tx_window_errors = ( unsigned long ) pHardware->m_cnMIB[
        MIB_COUNTER_TX_LATE_COLLISION ];

#ifdef DEBUG_COUNTER
    if ( jiffies - priv->query_jiffies >= 1000 * HZ / 1000 ) {
        if ( pHardware->m_nGood[ COUNT_GOOD_INT ] ) {
            display_mib_counters( pHardware, NULL );
        }
        printk( "%u %u %u; ",
            pHardware->m_nGood[ COUNT_GOOD_RCV_CNT_1 ],
            pHardware->m_nGood[ COUNT_GOOD_RCV_CNT_2 ],
            pHardware->m_nGood[ COUNT_GOOD_RCV_CNT_3 ]);
        printk( "I=%u L=%u R=%u; N=%u C=%u; Z=%u, T=%u\n",
            pHardware->m_nGood[ COUNT_GOOD_INT ],
            pHardware->m_nGood[ COUNT_GOOD_INT_LOOP ],
            pHardware->m_nGood[ COUNT_GOOD_INT_RX ],
            pHardware->m_nGood[ COUNT_GOOD_NEXT_PACKET ],
            pHardware->m_nGood[ COUNT_GOOD_RCV_COMPLETE ],
            pHardware->m_nGood[ COUNT_GOOD_NO_NEXT_PACKET ],
            pHardware->m_nGood[ COUNT_GOOD_RCV_NOT_DISCARD ]);
        pHardware->m_nGood[ COUNT_GOOD_INT ] =
        pHardware->m_nGood[ COUNT_GOOD_INT_LOOP ] =
        pHardware->m_nGood[ COUNT_GOOD_INT_RX ] =
        pHardware->m_nGood[ COUNT_GOOD_NEXT_PACKET ] =
        pHardware->m_nGood[ COUNT_GOOD_RCV_COMPLETE ] =
        pHardware->m_nGood[ COUNT_GOOD_NO_NEXT_PACKET ] =
        pHardware->m_nGood[ COUNT_GOOD_RCV_NOT_DISCARD ] =
        pHardware->m_nGood[ COUNT_GOOD_RCV_CNT_1 ] =
        pHardware->m_nGood[ COUNT_GOOD_RCV_CNT_2 ] =
        pHardware->m_nGood[ COUNT_GOOD_RCV_CNT_3 ] =
            0;
    }
    priv->query_jiffies = jiffies;
#endif

    return( &priv->stats );
}  /* dev_query_statistics */


/*
    device_set_mac_address

    Description:
        This function is used to set the MAC address of the network device.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        void* addr
            Buffer of MAC address.

    Return (int):
        Zero to indicate success.
*/

static int device_set_mac_address (
    struct net_device* dev,
    void*              addr )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    struct sockaddr* mac = addr;
    int              rc;
    UINT             InterruptMask;

#ifdef DBG
    printk( "set mac address\n" );
#endif

    memcpy( dev->dev_addr, mac->sa_data, MAX_ADDR_LEN );

    if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return( rc );
    }

    pHardware->m_bMacOverrideAddr = TRUE;
    memcpy( pHardware->m_bOverrideAddress, mac->sa_data, MAC_ADDRESS_LENGTH );

    InterruptMask = HardwareBlockInterrupt( pHardware );
    HardwareSetAddress( pHardware );
    HardwareRestoreInterrupt( pHardware, InterruptMask );

    ReleaseHardware( hw_priv, FALSE );

    return 0;
}  /* device_set_mac_address */


/*
    dev_set_multicast_list

    Description:
        This routine is used to set multicast addresses or put the network
        device into promiscuous mode.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        void* addr
            Buffer of MAC address.

    Return (None):
*/

static void dev_set_multicast_list (
    struct net_device* dev )
{
    struct dev_priv*    priv = netdev_priv(dev);
    struct dev_info*    hw_priv = priv->pDevInfo;
    PHARDWARE           pHardware = &hw_priv->hw;
    int                 rc;

#ifdef CONFIG_PEGASUS
    struct netdev_hw_addr* mc_ptr;
#endif

    if ( ( rc = AcquireHardware( hw_priv, FALSE, FALSE )) ) {
        return;
    }

    /* Turn on/off promiscuous mode. */
    HardwareSetPromiscuous( pHardware,
        ( BYTE )(( dev->flags & IFF_PROMISC ) == IFF_PROMISC ));

#ifdef CONFIG_PEGASUS
    /* Turn on/off all multicast mode. */
    HardwareSetMulticast( pHardware,
        ( BYTE )(( dev->flags & IFF_ALLMULTI ) == IFF_ALLMULTI ));

    if ( ( dev->flags & IFF_MULTICAST )  &&  netdev_mc_count(dev) ) {
        if ( netdev_mc_count(dev) <= MAX_MULTICAST_LIST ) {
            int i = 0;

	    netdev_for_each_mc_addr(mc_ptr, dev) {
                if ( !( *mc_ptr->addr & 1 ) )
                    continue;
                if ( i >= MAX_MULTICAST_LIST )
                    break;
                memcpy( pHardware->m_bMulticastList[ i++ ], mc_ptr->addr,
                    MAC_ADDRESS_LENGTH );
            }
            pHardware->m_bMulticastListSize = ( BYTE ) i;
            HardwareSetGroupAddress( pHardware );
        }

        /* List too big to support so turn on all multicast mode. */
        else {
            pHardware->m_bMulticastListSize = 255;
            HardwareSetMulticast( pHardware, TRUE );
        }
    }
    else {
        pHardware->m_bMulticastListSize = 0;
        HardwareClearMulticast( pHardware );
    }

#else
    if ( ( dev->flags & ( IFF_ALLMULTI | IFF_MULTICAST )) )
        HardwareSetMulticast( pHardware, TRUE );
    else
        HardwareSetMulticast( pHardware, FALSE );
#endif

    ReleaseHardware( hw_priv, FALSE );
}  /* dev_set_multicast_list */


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 21))
/*
    mdio_read

    Description:
        This function returns the PHY register value.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        int phy_id
            The PHY id.

        int reg_num
            The register number.

    Return (int):
        The register value.
*/

static int mdio_read (
    struct net_device *dev,
    int               phy_id,
    int               reg_num )
{
    u16 val_out;

    HardwareReadPhy( phy_id, reg_num, &val_out );

    return val_out;
}  /* mdio_read */


/*
    mdio_write

    Description:
        This procedure sets the PHY register value.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        int phy_id
            The PHY id.

        int reg_num
            The register number.

        int val
            The register value.

    Return (None):
*/

static void mdio_write (
    struct net_device *dev,
    int               phy_id,
    int               reg_num,
    int               val )
{
    HardwareWritePhy( phy_id, reg_num, val );
}  /* mdio_write */


#include "device_ethtool.c"
#endif


/*
    netdev_ioctl

    Description:
        This function is used to process I/O control calls.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ifreq* ifr
            Pointer to interface request structure.

        int cmd
            I/O control code.

    Return (int):
        Zero to indicate success.
*/

#define SIOCDEVDEBUG  ( SIOCDEVPRIVATE + 10 )

static int netdev_ioctl (
    struct net_device* dev,
    struct ifreq*      ifr,
    int                cmd )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    PTRequest        req = ( PTRequest ) ifr->ifr_data;
    int              nResult;
    int              rc;
    int              result = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    struct mii_ioctl_data *data = ( struct mii_ioctl_data* ) &ifr->ifr_data;

#else
    struct mii_ioctl_data *data = if_mii( ifr );
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_INC_USE_COUNT;
#endif

    if ( down_interruptible( &hw_priv->proc_sem ) ) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
        MOD_DEC_USE_COUNT;
#endif
        return -ERESTARTSYS;
    }

    // assume success
    nResult = DEV_IOC_OK;
    switch ( cmd ) {
    	case SIOCGMIIPHY:               /* Get address of MII PHY in use. */
        case SIOCDEVPRIVATE:            /* for binary compat, remove in 2.5 */
            data->phy_id = pHardware->m_wPhyAddr;

            /* Fallthrough... */

	case SIOCGMIIREG:               /* Read MII PHY register. */
        case SIOCDEVPRIVATE+1:          /* for binary compat, remove in 2.5 */
            if ( data->phy_id > 32  ||
                    data->reg_num > 32 )
                result = -EIO;
            else {
                HardwareReadPhy( data->phy_id, data->reg_num,
                    &data->val_out );
            }
            break;

        case SIOCSMIIREG:               /* Write MII PHY register. */
        case SIOCDEVPRIVATE+2:          /* for binary compat, remove in 2.5 */
            if ( !capable( CAP_NET_ADMIN ) )
                result = -EPERM;
            else if ( data->phy_id > 32  ||
                    data->reg_num > 32 )
                result = -EIO;
            else {
                HardwareWritePhy( data->phy_id, data->reg_num,
                    data->val_in );
            }
            break;

        case SIOCDEVDEBUG:
            if ( req ) {
                switch ( req->nCmd ) {
                    case DEV_CMD_INIT:
                        req->param.bData[ 0 ] = 'M';
                        req->param.bData[ 1 ] = 'i';
                        req->param.bData[ 2 ] = 'c';
                        req->param.bData[ 3 ] = 'r';
                        req->nSize = 8 + 4;
                        break;
                    case DEV_CMD_GET:
                        switch ( req->nSubCmd ) {
                            case DEV_READ_REG:
                                break;
                            case DEV_LINK_STATUS:
                                HardwareGetLinkStatus( pHardware,
                                    &req->param.LinkStatus );
                                break;
                        }
                        break;
                    case DEV_CMD_PUT:
                        if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
                            result = -EAGAIN;
                            break;
                        }
                        switch ( req->nSubCmd ) {
                            case DEV_CAPABILITIES:
                                HardwareSetCapabilities( pHardware,
                                    req->param.nData[ 0 ]);
                                break;
                            case DEV_CABLE_STATUS:
                                HardwareGetCableStatus( pHardware,
                                    req->param.CableStatus );
                                break;
                        }
                        ReleaseHardware( hw_priv, FALSE );
                        break;
                    default:
                        nResult = DEV_IOC_INVALID_CMD;
                        break;
                }
            }
            if ( req ) {
                req->nResult = nResult;
            }
            else if ( !result )
                result = -EFAULT;
            break;
        default:
            result = -EOPNOTSUPP;
    }

    up( &hw_priv->proc_sem );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
    MOD_DEC_USE_COUNT;
#endif

    return( result );
}  /* netdev_ioctl */


void update_link (
    struct net_device* dev,
    struct dev_priv*   priv,
    PHARDWARE          pHardware )
{
    if ( priv->media_state != pHardware->m_ulHardwareState ) {
        priv->media_state = pHardware->m_ulHardwareState;
        if ( priv->opened ) {
            if ( MediaStateConnected == pHardware->m_ulHardwareState )
                netif_carrier_on( dev );
            else
                netif_carrier_off( dev );
            if ( netif_msg_link( priv ) ) {
                printk(KERN_INFO "%s link %s\n", dev->name,
                    ( MediaStateConnected == priv->media_state ?
                    "on" : "off" ));
            }
        }
    }
}  /* update_link */


/*
    dev_monitor

    Description:
        This routine is run in a kernel timer to monitor the network device.

    Parameters:
        unsigned long ptr
            Pointer value to supplied data.

    Return (None):
*/

static void dev_monitor (
    struct timer_list *t )
{
    struct dev_info*   hw_priv = from_timer(hw_priv, t, timerMonitor);
    struct net_device* dev = hw_priv->dev;
    struct dev_priv*   priv = netdev_priv(dev);
    PHARDWARE          pHardware = &hw_priv->hw;
    PTTimerInfo        pInfo = &hw_priv->MonitorTimerInfo;
    int                rc;

#ifdef VERIFY_TIMETICK
    if ( !last_tick ) {
        last_tick = system_time_tick;
        time_tick = KS_READ_REG( 0xE424 );
        printk( "time: %lu ms\n", ( time_tick - last_tick ) /
            ksz_system_bus_clock );
        last_tick = time_tick;
    }
#endif
    if ( !( rc = AcquireHardware( hw_priv, FALSE, FALSE )) ) {

#ifdef CONFIG_PEGASUS
        if ( !pHardware->m_bLinkIntWorking ) {
            HardwareGetLinkSpeed( pHardware );
            update_link( dev, priv, pHardware );
        }
#endif

#ifdef CONFIG_PEGASUS
        {

            /* Reading MIB counters or requested to read. */
            if ( pHardware->m_bCurrentCounter  ||
                    hw_priv->Counter.fRead ) {

                /* Need to process receive interrupt. */
                if ( HardwareReadCounters( pHardware ) )
                    goto dev_monitor_release;
                hw_priv->Counter.fRead = 0;

                /* Finish reading counters. */
                if ( 0 == pHardware->m_bCurrentCounter ) {
                    wake_up_interruptible(
                        &hw_priv->Counter.wqhCounter );
                }
            }
            else if ( jiffies >= hw_priv->Counter.time ) {
                hw_priv->Counter.fRead = 1;
                hw_priv->Counter.time = jiffies + HZ * 6;
            }
        }

dev_monitor_release:
#endif
        ReleaseHardware( hw_priv, FALSE );
    }

#ifdef DBG
    if ( strMsg != strDebug ) {
        if ( fLastDebugLine )
            printk( "%c", DBG_CH );
        fLastDebugLine = 0;
        if ( '\n' == strDebug[ cnDebug - 2 ]  &&
                DBG_CH == strDebug[ cnDebug - 1 ] ) {
            strDebug[ cnDebug - 1 ] = '\0';
            fLastDebugLine = 1;
        }
        strMsg = strDebug;
        cnDebug = 0;
        printk( strDebug );
    }
    if ( strIntMsg != strIntBuf ) {
        printk( "---\n" );
        if ( fLastIntBufLine )
            printk( "%c", DBG_CH );
        fLastIntBufLine = 0;
        if ( '\n' == strIntBuf[ cnIntBuf - 2 ]  &&
                DBG_CH == strIntBuf[ cnIntBuf - 1 ] ) {
            strIntBuf[ cnIntBuf - 1 ] = '\0';
            fLastIntBufLine = 1;
        }
        strIntMsg = strIntBuf;
        cnIntBuf = 0;
        printk( strIntBuf );
    }
#endif

    ++pInfo->cnCount;
    if ( pInfo->nCount > 0 ) {
        if ( pInfo->cnCount < pInfo->nCount ) {
            pInfo->pTimer->expires = jiffies + pInfo->nTime;
            add_timer( pInfo->pTimer );
        }
        else
            pInfo->nCount = 0;
    }
    else if ( pInfo->nCount < 0 ) {
        pInfo->pTimer->expires = jiffies + pInfo->nTime;
        add_timer( pInfo->pTimer );
    }
}  /* dev_monitor */


static struct net_device* last_dev = NULL;
static int device_present = 0;

struct hw_fn* ks8692_fn = NULL;

#ifdef PROC_OLD_API
static void init_proc (
    struct dev_info* priv )
{
    struct proc_dir_entry* proc_info;
    struct proc_dir_entry* proc_set;
    int                    i;
    int                    port;
    char                   proc_name[ 40 ];

    sprintf( proc_name, "%s-%d", proc_dir_name, priv->id );
    sema_init( &priv->proc_sem, 1 );
    priv->proc_main = proc_mkdir( proc_name, NULL );
    if ( priv->proc_main ) {
        port = priv->id;

        /* Read-only entries. */
        for ( i = 0; i < PROC_SET_DUPLEX; i++ ) {
            priv->ProcPortInfo[ i ].priv = priv;
            priv->ProcPortInfo[ i ].info = &ProcInfo[ i ];
            priv->ProcPortInfo[ i ].port = port;
            proc_info = create_proc_read_entry( ProcInfo[ i ].proc_name,
                0444, priv->proc_main, read_proc, &priv->ProcPortInfo[ i ]);
        }

        /* Can be written to. */
#ifdef CONFIG_KSZ8692VB
        for ( i = PROC_SET_DUPLEX; i < PROC_SET_ACL_BLOCK_UNMATCHED; i++ ) {
#else
        for ( i = PROC_SET_DUPLEX; i < PROC_LAST; i++ ) {
#endif
            priv->ProcPortInfo[ i ].priv = priv;
            priv->ProcPortInfo[ i ].info = &ProcInfo[ i ];
            priv->ProcPortInfo[ i ].port = port;
            proc_set = create_proc_entry( ProcInfo[ i ].proc_name, 0644,
                priv->proc_main );
            if ( proc_set ) {
                proc_set->data = &priv->ProcPortInfo[ i ];
                proc_set->read_proc = read_proc;
                proc_set->write_proc = write_proc;
            }
        }

#ifdef CONFIG_KSZ8692VB
        priv->proc_acldir = proc_mkdir( "acl", priv->proc_main );

        i = PROC_SET_ACL_BLOCK_UNMATCHED;
        priv->ProcPortInfo[ i ].priv = priv;
        priv->ProcPortInfo[ i ].info = &ProcInfo[ i ];
        priv->ProcPortInfo[ i ].port = port;
        proc_set = create_proc_entry( ProcInfo[ i ].proc_name, 0644,
            priv->proc_acldir );
        if ( proc_set ) {
            proc_set->data = &priv->ProcPortInfo[ i ];
            proc_set->read_proc = read_proc;
            proc_set->write_proc = write_proc;
        }
        for ( port = 0; port < TOTAL_ACL_NUM; port++ ) {
            priv->hw.AclInfo[ port ].acl = port;

            sprintf( proc_name, "%02x", port );
            priv->proc_acl[ port ] = proc_mkdir( proc_name, priv->proc_acldir );
            for ( i = PROC_SET_ACL_ENABLE; i < PROC_LAST; i++ ) {
                int j = i - PROC_SET_ACL_ENABLE;

                if ( PROC_SET_ACL_IPV6_ADDR <= i  &&
                        i <= PROC_SET_ACL_IPV6_MASK  &&
                        ( port % 4 ) )
                    continue;
                priv->ProcAclInfo[ port ][ j ].priv = priv;
                priv->ProcAclInfo[ port ][ j ].info = &ProcInfo[ i ];
                priv->ProcAclInfo[ port ][ j ].port = port;
                proc_set = create_proc_entry( ProcInfo[ i ].proc_name, 0644,
                    priv->proc_acl[ port ]);
                if ( proc_set ) {
                    proc_set->data = &priv->ProcAclInfo[ port ][ j ];
                    proc_set->read_proc = read_proc;
                    proc_set->write_proc = write_proc;
                }
            }
        }
#endif
    }
}  /* init_proc */


static void exit_proc (
    struct dev_info* priv )
{
    int  acl;
    int  j;
    char proc_name[ 40 ];

#ifdef CONFIG_KSZ8692VB
    acl = PROC_SET_ACL_BLOCK_UNMATCHED;
#else
    acl = PROC_LAST;
#endif
    for ( j = 0; j < acl; j++ ) {
        if ( priv->ProcPortInfo[ j ].info ) {
            remove_proc_entry( ProcInfo[ j ].proc_name, priv->proc_main );
        }
    }

#ifdef CONFIG_KSZ8692VB
    for ( acl = 0; acl < TOTAL_ACL_NUM; acl++ ) {
        if ( priv->proc_acl[ acl ] ) {
            for ( j = PROC_SET_ACL_ENABLE; j < PROC_LAST; j++ ) {
                if ( priv->ProcAclInfo[ acl ]
                        [ j - PROC_SET_ACL_ENABLE ].info ) {
                    remove_proc_entry( ProcInfo[ j ].proc_name,
                        priv->proc_acl[ acl ]);
                }
            }
        }
        sprintf( proc_name, "%02x", acl );
        remove_proc_entry( proc_name, priv->proc_acldir );
    }
    remove_proc_entry( ProcInfo[ PROC_SET_ACL_BLOCK_UNMATCHED ].proc_name,
        priv->proc_acldir );
    remove_proc_entry( "acl", priv->proc_main );
#endif

    sprintf( proc_name, "%s-%d", proc_dir_name, priv->id );
    remove_proc_entry( proc_name, NULL );
}  /* exit_proc */
#endif /* PROC_OLD_API */


#ifdef CONFIG_KSZ8692VA
int rx_ip_csum = 0;
int rx_tcp_csum = 1;
int rx_udp_csum = 0;

#else
int rx_icmp_csum = 0;
int rx_ip_csum = 0;
int rx_tcp_csum = 0;
int rx_udp_csum = 0;
#endif

#ifdef CONFIG_KSZ8692VA
int rx_err = 1;

#else
int rx_err = 0;
#endif

static char* macaddr = ":";

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
module_param( macaddr, charp, 0 );
module_param( rx_ip_csum, int, 0 );
module_param( rx_tcp_csum, int, 0 );
module_param( rx_udp_csum, int, 0 );
module_param( rx_err, int, 0 );

#else
MODULE_PARM( macaddr, "s" );
MODULE_PARM( rx_ip_csum, "i" );
MODULE_PARM( rx_tcp_csum, "i" );
MODULE_PARM( rx_udp_csum, "i" );
MODULE_PARM( rx_err, "i" );
#endif
MODULE_PARM_DESC( macaddr, "MAC address" );
MODULE_PARM_DESC( rx_ip_csum, "Enable receive IP checksum verification" );
MODULE_PARM_DESC( rx_tcp_csum, "Enable receive TCP checksum verification" );
MODULE_PARM_DESC( rx_udp_csum, "Enable receive UDP checksum verification" );
MODULE_PARM_DESC( rx_err, "Enable receive error frames" );


#if 0
static UCHAR PauseFrame[] =
{
/*  0 */    0x01, 0x80, 0xC2, 0x00, 0x00, 0x01,     /* 01:80:c2:00:00:01 (DA) */
/*  6 */    0x08, 0x00, 0x70, 0x22, 0x44, 0x55,     /* 08:00:70:22:44:55 (SA) */
/* 12 */    0x88, 0x08,                             /* MAC control */
/* 14 */    0x00, 0x01,                             /* PAUSE */
/* 16 */    0x00, 0x00,                             /* time */
/* 18 */    0x00, 0x00,
};
#endif

#ifdef ALIGN_IP_HEADER
static int UpdateReceiveBuffers (
    struct dev_info* hw_priv )
{
    int         i;
    PTDesc      pDesc;
    PDMA_BUFFER pDma;
    PHARDWARE   pHardware = &hw_priv->hw;
    PTDescInfo  pInfo = &pHardware->m_RxDescInfo;

    if ( ( i = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return( i );
    }
    HardwareStopReceive( pHardware );
    DelayMillisec( 1000 / HZ );
    for ( i = 0; i < pHardware->m_RxDescInfo.cnAlloc; i++ )
    {
        GetRxPacket( pInfo, pDesc );
        pDma = DMA_BUFFER( pDesc );

        /* Need to increase length. */
        if ( pDma->len < hw_priv->m_nMTU ) {

#ifdef HIGH_MEM_SUPPORT
            dma_unmap_page(
                hw_priv->pdev, pDma->dma, pDma->len,
                DMA_FROM_DEVICE );

#else
            dma_unmap_single(
                hw_priv->pdev, pDma->dma, pDma->len,
                DMA_FROM_DEVICE );
#endif
            dev_kfree_skb( pDma->skb );
            pDma->len = hw_priv->m_nMTU;

            /* skb->data != skb->head */
            pDma->skb = dev_alloc_skb( pDma->len );
            pDma->skb->dev = hw_priv->dev;

#ifdef HIGH_MEM_SUPPORT
            pDma->dma = dma_map_page(
                hw_priv->pdev, virt_to_page( pDma->skb->tail ),
                (( unsigned long ) pDma->skb->tail & ~PAGE_MASK ), pDma->len,
                DMA_FROM_DEVICE );

#else
            pDma->dma = dma_map_single(
                hw_priv->pdev, pDma->skb->tail, pDma->len,
                DMA_FROM_DEVICE );
#endif

            /* Set descriptor. */
            SetReceiveBuffer( pDesc, pDma->dma );
        }
        pDma->len = hw_priv->m_nMTU;
        SetReceiveLength( pDesc, pDma->len - SKB_RESERVED );
        ReleasePacket( pDesc );
    }
    if ( pHardware->m_bEnabled )
        HardwareStartReceive( pHardware );
    ReleaseHardware( hw_priv, FALSE );
    return 0;
}  /* UpdateReceiveBuffers */
#endif


extern UCHAR TestPacket[];

#if 0
static UCHAR PauseFrame[] = {
/*  0 */  0x01, 0x80, 0xC2, 0x00, 0x00, 0x01,  /* 01:80:C2:00:00:01 (DA) */
/*  6 */  0x00, 0x10, 0xA1, 0x96, 0x92, 0x20,  /* 00:10:A1:96:92:20 (SA) */
/* 12 */  0x88, 0x08,                          /* MAC control */
/* 14 */  0x00, 0x01,                          /* PAUSE */
/* 16 */  0x00, 0x00,                          /* time */
/* 16 */  0x00, 0x00,
};

static UCHAR ICMPv6_Solicitation[] = {
/*  0 */  0x33, 0x33, 0xFF, 0x27, 0x54, 0x11,  /* 33:33:FF:27:54:11 (DA) */
/*  6 */  0x00, 0x10, 0xA1, 0x96, 0x92, 0x20,  /* 00:10:A1:96:92:20 (SA) */
/* 12 */  0x86, 0xDD,                          /* IPv6 */
/* 14 */  0x60, 0x00, 0x00, 0x00,
/* 18 */  0x00, 0x20,
/* 20 */  0x3C,                                /* ICMPv6 */
/* 21 */  0xFF,                                /* hop */
/* 22 */  0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 30 */  0x02, 0x10, 0xA1, 0xFF, 0xFE, 0x96, 0x92, 0x20,
/* 38 */  0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 46 */  0x00, 0x00, 0x00, 0x01, 0xFF, 0x27, 0x54, 0x11,
/* 54 */  0x87, 0x00,
/* 56 */  0xB1, 0x98,                          /* ICMPv6 checksum */
/* 58 */  0x00, 0x00, 0x00, 0x00,
/* 62 */  0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 70 */  0x02, 0x19, 0xB9, 0xFF, 0xFE, 0x27, 0x54, 0x11,
/* 78 */  0x01, 0x01,
/* 80 */  0x00, 0x10, 0xA1, 0x96, 0x92, 0x20,  /* SA */
/* 86 */
};

static UCHAR ICMPv6_Advertisement[] = {
/*  0 */  0x00, 0x10, 0xA1, 0x96, 0x92, 0x20,  /* 00:10:A1:96:92:20 (DA) */
/*  6 */  0x00, 0x19, 0xB9, 0x27, 0x54, 0x11,  /* 00:19:B9:27:54:11 (SA) */
/* 12 */  0x86, 0xDD,                          /* IPv6 */
/* 14 */  0x60, 0x00, 0x00, 0x00,
/* 18 */  0x00, 0x20,
/* 20 */  0x3A,                                /* ICMPv6 */
/* 21 */  0xFF,                                /* hop */
/* 22 */  0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 30 */  0x02, 0x19, 0xB9, 0xFF, 0xFE, 0x27, 0x54, 0x11,
/* 38 */  0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 46 */  0x02, 0x10, 0xA1, 0xFF, 0xFE, 0x96, 0x92, 0x20,
/* 54 */  0x88, 0x00,
/* 56 */  0xBB, 0x6D,                          /* ICMPv6 checksum */
/* 58 */  0x60, 0x00, 0x00, 0x00,
/* 62 */  0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 70 */  0x02, 0x19, 0xB9, 0xFF, 0xFE, 0x27, 0x54, 0x11,
/* 78 */  0x02, 0x01,
/* 80 */  0x00, 0x19, 0xB9, 0x27, 0x54, 0x11,  /* SA */
/* 86 */
};

static UCHAR IPX_SAP[] = {
/*  0 */  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/*  6 */  0x00, 0x03, 0x47, 0x43, 0xDE, 0x9A,
/* 12 */  0x00, 0x25,
/* 14 */  0xE0, 0xE0, 0x03,
/* 17 */  0xFF, 0xFF,
/* 19 */  0x00, 0x22,
/* 21 */  0x00,
/* 22 */  0x11,
/* 23 */  0x00, 0x00, 0x00, 0x00,
/* 27 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 33 */  0x04, 0x52,  /* SAP */
/* 35 */  0x00, 0x00, 0x00, 0x00,
/* 39 */  0x00, 0x03, 0x47, 0x43, 0xDE, 0x9A,
/* 45 */  0x41, 0x3C,
/* 47 */  0x00, 0x01,
/* 49 */  0x00, 0x04,
/* 51 */  0x5A, 0x5A,
};
#endif


int netdev_change_mtu (
    struct net_device* dev,
    int                new_mtu )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;

#ifdef ALIGN_IP_HEADER
    int              reserved = SKB_RESERVED;
    int              rx_size;
#endif

#if 0
    PHARDWARE        pHardware = &hw_priv->hw;
    struct sk_buff*  skb;
#if 0
    unsigned short*  wptr;
#endif

    if ( 0 == new_mtu  ||  1 == new_mtu  ||  2 == new_mtu ) {
        skb = dev_alloc_skb( 96 );
        if ( skb ) {
            skb->dev = dev;
#if 0
            skb->len = 18;
            memcpy( skb->data, PauseFrame, skb->len );
#if 0
            wptr = ( unsigned short* ) &skb->data[ 16 ];
            *wptr = htons( pause_time );
#endif
#else
            if ( 0 == new_mtu ) {
                skb->len = 42;
                memcpy( skb->data, TestPacket, skb->len );
#if 0
                memcpy( &skb->data[ 22 ], pHardware->m_bOverrideAddress, 6 );
#endif
                *(( unsigned int* ) &skb->data[ 28 ]) = 0x6503200A;
                *(( unsigned int* ) &skb->data[ 38 ]) = 0x3803200A;
                memcpy( &skb->data[ 6 ], pHardware->m_bOverrideAddress, 6 );
            }
            else if ( 1 == new_mtu ) {
                skb->len = 86;
#if 1
                memcpy( skb->data, ICMPv6_Solicitation, skb->len );
                memcpy( &skb->data[ 31 ], &pHardware->m_bOverrideAddress[ 1 ],
                    2 );
                memcpy( &skb->data[ 35 ], &pHardware->m_bOverrideAddress[ 3 ],
                    3 );
                memcpy( &skb->data[ 80 ], pHardware->m_bOverrideAddress, 6 );
                memcpy( &skb->data[ 6 ], pHardware->m_bOverrideAddress, 6 );
                skb->data[ 56 ] = 0xBE;
                skb->data[ 57 ] = 0xEF;
#else
                memcpy( skb->data, ICMPv6_Advertisement, skb->len );
                memcpy( &skb->data[ 0 ], pHardware->m_bOverrideAddress, 6 );
                memcpy( &skb->data[ 47 ], &pHardware->m_bOverrideAddress[ 1 ],
                    2 );
                memcpy( &skb->data[ 51 ], &pHardware->m_bOverrideAddress[ 3 ],
                    3 );
                skb->data[ 56 ] = 0xBE;
                skb->data[ 57 ] = 0xEF;
#endif
            }
            else if ( 2 == new_mtu ) {
                skb->len = 51;
                memcpy( skb->data, IPX_SAP, skb->len );
                memcpy( &skb->data[ 0 ], pHardware->m_bOverrideAddress, 6 );
            }
#endif
            dev->hard_start_xmit( skb, dev );
        }
        return 0;
    }
#endif
    if ( new_mtu < 60 )
        return -EINVAL;
    if ( new_mtu > hw_priv->m_nRxBufSize - ETHERNET_HEADER_SIZE - 4 )
        return -EINVAL;

#ifdef ALIGN_IP_HEADER
    rx_size = ( new_mtu + ETHERNET_HEADER_SIZE + 4 + reserved + 3 ) & ~3;
#ifdef CONFIG_KSZ8692VA
    if ( rx_size > hw_priv->m_nMaxBufSize  &&  rx_size < 2600 )
        return -ERANGE;
#endif
#endif
    if ( dev->mtu != new_mtu ) {

#ifdef ALIGN_IP_HEADER
        hw_priv->m_nMTU = rx_size + SKB_RESERVED;
        UpdateReceiveBuffers( hw_priv );
#endif
        dev->mtu = new_mtu;
    }
    return 0;
}  /* netdev_change_mtu */


#ifdef AVOID_TX_FAILURE
void ResetTask (
    unsigned long data )
{
    struct net_device* dev = ( struct net_device* ) data;

    DEV_TRANSMIT_TIMEOUT( dev );
}  /* ResetTask */
#endif

/*
    netdev_init

    Description:
        This function initializes the network device.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

int __init
ks8692_netdev_init (
    struct net_device* dev )
{
    struct dev_priv* priv;
    struct dev_info* hw_priv = NULL;
    int              rc = -1;

    /* This is a real device. */
    if ( !last_dev ) {
        if ( !device_present )
            dev->mem_start = dev->base_addr + KS_WAN_DMA_TX;
        else
            dev->mem_start = dev->base_addr + KS_LAN_DMA_TX;
        dev->mem_end = dev->mem_start + BASE_IO_RANGE - 1;
        dev->base_addr = 0;
    }

    priv = netdev_priv(dev);
    spin_lock_init( &priv->lock );

    if ( !last_dev ) {
        priv->pDevInfo = &priv->dev_info;
        hw_priv = priv->pDevInfo;

        if ( !device_present ) {
            hw_priv->m_nMaxBufSize = WAN_MAX_BUF_SIZE;
            hw_priv->m_nRxBufSize = WAN_RX_BUF_SIZE;
            hw_priv->m_nTxBufSize = WAN_TX_BUF_SIZE;
        }
        else {
            hw_priv->m_nMaxBufSize = LAN_MAX_BUF_SIZE;
            hw_priv->m_nRxBufSize = LAN_RX_BUF_SIZE;
            hw_priv->m_nTxBufSize = LAN_TX_BUF_SIZE;
        }

#ifdef ALIGN_IP_HEADER
        hw_priv->m_nMTU = ( MAXIMUM_ETHERNET_PACKET_SIZE + 4 + 3 ) & ~3;
        hw_priv->m_nMTU += SKB_RESERVED;
#else
        hw_priv->m_nMTU = ( hw_priv->m_nRxBufSize + 3 ) & ~3;
#endif
        if ( AllocateMemory( hw_priv ) ) {
            FreeMemory( hw_priv );
            return -ENOMEM;
        }

        hw_priv->hw.m_pVirtualMemory = ( PUCHAR ) dev->mem_start;
        hw_priv->hw.m_wPhyAddr = 1;
        hw_priv->hw.m_pDevice = dev;

#ifdef CONFIG_PEGASUS
        if ( !device_present ) {
            hw_priv->hw.m_nShift = 25;
            hw_priv->hw.m_nPhyShift = 0;
            hw_priv->hw.m_dwRegCounter = 0xED00;
            hw_priv->hw.m_nRegCounter = WAN_PORT_COUNTER_NUM;
            hw_priv->hw.m_nTxCounterStart = WAN_RX_MIB_COUNTER_NUM;
            dev->mem_start = KS_IO_BASE + KS_WAN_DMA_TX;
        }
        else {
            hw_priv->hw.m_nShift = 12;
            hw_priv->hw.m_nPhyShift = 8;
            hw_priv->hw.m_dwRegCounter = 0xEDC0;
            hw_priv->hw.m_nRegCounter = LAN_PORT_COUNTER_NUM;
            hw_priv->hw.m_nTxCounterStart = LAN_RX_MIB_COUNTER_NUM;
            dev->mem_start = KS_IO_BASE + KS_LAN_DMA_TX;
        }
#else
        if ( !device_present ) {
            hw_priv->hw.m_nShift = 25;
            hw_priv->hw.m_nPhyShift = 0;
            dev->mem_start = KS_IO_BASE + KS_WAN_DMA_TX;
        }
        else {
            hw_priv->hw.m_nShift = 12;
            hw_priv->hw.m_nPhyShift = 8;
            dev->mem_start = KS_IO_BASE + KS_LAN_DMA_TX;
        }
#endif
        dev->mem_end = dev->mem_start + BASE_IO_RANGE - 1;

        request_mem_region( dev->mem_start, BASE_IO_RANGE, dev->name );

        InitHardware( hw_priv );

        hw_priv->hw.m_hwfn = ks8692_fn;
        hw_priv->hw.m_ulHardwareState = MediaStateDisconnected;

        hw_priv->MonitorTimerInfo.pTimer = &hw_priv->timerMonitor;
        hw_priv->MonitorTimerInfo.nCount = 0;

        /* 500 ms timeout */
        hw_priv->MonitorTimerInfo.nTime = 500 * HZ / 1000;
        timer_setup( &hw_priv->timerMonitor, dev_monitor, 0 );

        /* tasklet is enabled. */
        tasklet_init( &hw_priv->rx_tasklet, RxProcessTask,
            ( unsigned long ) dev );
        tasklet_init( &hw_priv->tx_tasklet, TxProcessTask,
            ( unsigned long ) dev );

        /* tasklet_enable will decrement the atomic counter. */
        tasklet_disable( &hw_priv->rx_tasklet );
        tasklet_disable( &hw_priv->tx_tasklet );

#ifdef AVOID_TX_FAILURE
        tasklet_init( &hw_priv->reset_tasklet, ResetTask,
            ( unsigned long ) dev );
        tasklet_disable( &hw_priv->reset_tasklet );
#endif

#ifdef CONFIG_DELAYED_TIMER
        tasklet_init( &hw_priv->delay_rx_taskinfo.tasklet, DelayRxIntrTask,
            ( unsigned long ) dev );
        tasklet_disable( &hw_priv->delay_rx_taskinfo.tasklet );
#endif

        init_waitqueue_head( &hw_priv->Counter.wqhCounter );
    }

    /* Fill in the fields of the device structure with default values. */
    ether_setup( dev );

    /* 500 ms timeout */
    dev->watchdog_timeo     = HZ / 2;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 21))
    dev->ethtool_ops        = &netdev_ethtool_ops;
#endif

#ifdef CONFIG_KSZ_NAPI
    dev->poll               = DEV_POLL;
    dev->weight             = hw_priv->hw.m_RxDescInfo.cnAlloc;
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
    dev->poll_controller    = netdev_netpoll;
#endif

#if TXCHECKSUM_DEFAULT
    dev->features |= NETIF_F_IP_CSUM;
#endif
#ifdef SCATTER_GATHER
    dev->features |= NETIF_F_SG;
#endif
#ifdef DBG
printk( "features: %lx\n", dev->features );
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    SET_MODULE_OWNER( dev );
#endif

    /* This is a real device. */
    if ( !last_dev ) {
        if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
            return( rc );
        }

        dev->irq = hw_priv->hw.m_nShift;

        HardwareInitialize( &hw_priv->hw );
        HardwareReadAddress( &hw_priv->hw );
        ReleaseHardware( hw_priv, FALSE );
        if ( macaddr[ 0 ] != ':' ) {
            int i;
            int j;
            int got_num;
            int num;

            i = j = num = got_num = 0;
            while ( j < MAC_ADDRESS_LENGTH ) {
                if ( macaddr[ i ] ) {
                    got_num = 1;
                    if ( '0' <= macaddr[ i ]  &&  macaddr[ i ] <= '9' )
                        num = num * 16 + macaddr[ i ] - '0';
                    else if ( 'A' <= macaddr[ i ]  &&  macaddr[ i ] <= 'F' )
                        num = num * 16 + 10 + macaddr[ i ] - 'A';
                    else if ( 'a' <= macaddr[ i ]  &&  macaddr[ i ] <= 'f' )
                        num = num * 16 + 10 + macaddr[ i ] - 'a';
                    else if ( ':' == macaddr[ i ] )
                        got_num = 2;
                    else
                        break;
                }
                else if ( got_num )
                    got_num = 2;
                else
                    break;
                if ( 2 == got_num ) {
                    hw_priv->hw.m_bOverrideAddress[ j++ ] = ( BYTE ) num;
                    num = got_num = 0;
                }
                i++;
            }
            if ( MAC_ADDRESS_LENGTH == j ) {
                hw_priv->hw.m_bMacOverrideAddr = TRUE;
                memcpy( dev->dev_addr, hw_priv->hw.m_bOverrideAddress,
                    MAC_ADDRESS_LENGTH );
            }
        }
        if ( !hw_priv->hw.m_bMacOverrideAddr )
            memcpy( dev->dev_addr, hw_priv->hw.m_bPermanentAddress,
                MAC_ADDRESS_LENGTH );

        hw_priv->id = device_present;
#ifdef PROC_OLD_API
        init_proc( hw_priv );
#endif /* PROC_OLD_API */
    }
    ++device_present;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 21))
    priv->mii_if.phy_id_mask = 0x1f;
    priv->mii_if.reg_num_mask = 0x1f;
    priv->mii_if.dev = dev;
    priv->mii_if.mdio_read = mdio_read;
    priv->mii_if.mdio_write = mdio_write;
    priv->mii_if.phy_id = hw_priv->hw.m_wPhyAddr;
#endif
    priv->msg_enable = NET_MSG_ENABLE;

    return 0;
}  /* netdev_init */


#define MAX_DEVICES  2

static struct net_device* NetDevice[ MAX_DEVICES ];

static int io = BASE_IO_ADDR;
static int irq = BASE_IRQ;
static int ifport;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
module_param( io, int, 0 );
module_param( irq, int, 0 );
module_param( ifport, int, 0 );

#else
MODULE_PARM( io, "i" );
MODULE_PARM( irq, "i" );
MODULE_PARM( ifport, "i" );
#endif
MODULE_PARM_DESC( io, "Device I/O base address" );
MODULE_PARM_DESC( irq, "Devie IRQ number" );
MODULE_PARM_DESC( ifport, "Device interface port (0-default, 1-TP, 2-AUI)" );

/* -------------------------------------------------------------------------- */

#ifdef CONFIG_PEGASUS
irqreturn_t
mdio_interrupt (
    int             irq,
    void*           dev_id
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 21))
    ,
    struct pt_regs* regs
#endif
    )
{
    UINT               status;
    struct net_device* dev;
    struct dev_priv*   priv;
    struct dev_info*   hw_priv;
    PHARDWARE          pHardware;

    status = KS_READ_REG( KS8692_STA_INT_STATUS );

#ifdef DBG
printk( "mdio: %x\n", status );
#endif
    dev = NetDevice[ 0 ];
    priv = netdev_priv(dev);
    hw_priv = priv->pDevInfo;
    pHardware = &hw_priv->hw;
    if ( ( status & PHY0_UP ) ) {
        pHardware->m_bLinkIntWorking = TRUE;
        HardwareGetLinkSpeed( pHardware );

#ifdef AVOID_TX_FAILURE
        if ( 10000000 == pHardware->m_ulTransmitRate ) {
            hw_priv->gigabit_rate = TRUE;
        }
        else if ( hw_priv->gigabit_rate ) {
            hw_priv->gigabit_rate = FALSE;
            tasklet_hi_schedule( &hw_priv->reset_tasklet );
        }
#endif
    }
    else if ( ( status & PHY0_DN ) ) {
        pHardware->m_bLinkIntWorking = TRUE;
        pHardware->m_ulHardwareState = MediaStateDisconnected;
    }
    update_link( dev, priv, pHardware );

#if MAX_DEVICES > 1
    dev = NetDevice[ 1 ];
    priv = netdev_priv(dev);
    hw_priv = priv->pDevInfo;
    pHardware = &hw_priv->hw;
    if ( ( status & PHY1_UP ) ) {
        pHardware->m_bLinkIntWorking = TRUE;
        HardwareGetLinkSpeed( pHardware );

#ifdef AVOID_TX_FAILURE
        if ( 10000000 == pHardware->m_ulTransmitRate ) {
            hw_priv->gigabit_rate = TRUE;
        }
        else if ( hw_priv->gigabit_rate ) {
            hw_priv->gigabit_rate = FALSE;
            tasklet_hi_schedule( &hw_priv->reset_tasklet );
        }
#endif
    }
    else if ( ( status & PHY1_DN ) ) {
        pHardware->m_bLinkIntWorking = TRUE;
        pHardware->m_ulHardwareState = MediaStateDisconnected;
    }
    update_link( dev, priv, pHardware );
#endif

    return IRQ_HANDLED;
}  /* mdio_interrupt */


static char link_name[ 40 ];

int request_mdio_irq (
    struct net_device* dev )
{
    int rc;

    KS_READ_REG( KS8692_STA_INT_STATUS );
    sprintf( link_name, "net link" );
    if ( ( rc = request_irq( KS8692_INT_MDIO + LOW_IRQS, mdio_interrupt,
            0, link_name, dev )) ) {
    }
    return( rc );
}  /* request_mdio_irq */


void free_mdio_irq (
    struct net_device* dev )
{
    free_irq( KS8692_INT_MDIO + LOW_IRQS, dev );
}  /* free_mdio_irq */
#endif

/* -------------------------------------------------------------------------- */

static int HardwareInitPhyCtrl ( void )
{
#ifdef CONFIG_KSZ8692VA
#define F_MDC  1000  /* 5.00 MHz * 2 */
#elif defined( CONFIG_KSZ8692VB )
#define F_MDC  800  /* 4.00 MHz * 2 */
#else
#define F_MDC  1250  /* 6.25 MHz * 2 */
#endif

    UINT RegData;
    UINT clk_dividend;
    UINT fAPB;

    /* Get APB system clock in Hz */
    RegData = KS_READ_REG( KS8692_SYSTEM_BUS_CLOCK );
    fAPB = RegData & SYSTEM_BUS_CLOCK_MASK;

    switch ( fAPB ) {
        case SYSTEM_BUS_CLOCK_200:
            fAPB = 20000;
            break;
        case SYSTEM_BUS_CLOCK_166:
            fAPB = 16600;
            break;
        case SYSTEM_BUS_CLOCK_125:
            fAPB = 12500;
            break;
        case SYSTEM_BUS_CLOCK_50:
        default:
            fAPB = 5000;
            break;
    }

    /* Calculate CLK_DIVIDEND value -- F_mdc = F_apb / (CLK_DIVIDEND * 2) */
    clk_dividend = fAPB / F_MDC;

    RegData = KS_READ_REG( KS8692_STA_CONF );

    if ( ( RegData & STA_CLK_DIVIDEND_MASK ) != ( clk_dividend << 1 ) ) {
        int  status;
        int  timeout = STA_TIMEOUT;

        /* Disable STA first */
        RegData &= ~STA_MDIO_ENABLE;
        do {
            DelayMicrosec( 2 );
            status = KS_READ_REG( KS8692_STA_STATUS ) & STA_STATUS_MASK;
        } while ( status != STA_IDLE  &&  --timeout );
        KS_WRITE_REG( KS8692_STA_CONF, RegData );

        /* Set CLK_DIVIDEND value to STA Configuration Register */
        RegData &= ~STA_CLK_DIVIDEND_MASK;
        RegData |= ( clk_dividend << 1 );
    }
    RegData |= STA_MDIO_ENABLE;
    RegData |= STA_AUTO_POLL;
    KS_WRITE_REG( KS8692_STA_CONF, RegData );

    KS_WRITE_REG( KS8692_STA_INT_CTRL,
        KS_READ_REG( KS8692_STA_INT_CTRL ) |
        PHY1_DN_ENABLE | PHY1_UP_ENABLE |
        PHY0_DN_ENABLE | PHY0_UP_ENABLE );

    return TRUE;
}  /* HardwareInitPhyCtrl */

static const struct net_device_ops ks8692_netdev_ops= {
	.ndo_init		= ks8692_netdev_init,
	.ndo_open		= netdev_open,
	.ndo_stop		= netdev_close,
	.ndo_start_xmit		= DEV_TRANSMIT,
	.ndo_tx_timeout		= DEV_TRANSMIT_TIMEOUT,
	.ndo_get_stats		= dev_query_statistics,
	.ndo_set_mac_address	= device_set_mac_address,
	.ndo_set_rx_mode	= dev_set_multicast_list,
	.ndo_do_ioctl		= netdev_ioctl,
	.ndo_change_mtu		= netdev_change_mtu,
};

static int __init netdev_create (
    struct net_device* dev )
{
    /* Copy the parameters from insmod into the device structure. */
    dev->base_addr  = io;
    dev->irq        = irq;
    dev->if_port    = ifport;
    dev->netdev_ops = &ks8692_netdev_ops;
    return( register_netdev( dev ));
}  /* netdev_create */


static void netdev_free (
    struct net_device* dev )
{
    struct dev_priv* priv;
    struct dev_info* hw_priv;

    priv = netdev_priv(dev);
    hw_priv = &priv->dev_info;

    unregister_netdev( dev );

    /* Not a pseudo device. */
    if ( hw_priv == priv->pDevInfo ) {
#ifdef PROC_OLD_API
        if ( hw_priv->proc_main ) {
            exit_proc( hw_priv );
        }
#endif /* PROC_OLD_API */

        release_mem_region( dev->mem_start, BASE_IO_RANGE );
    }
    free_netdev( dev );
}  /* netdev_free */


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 9))
static char PLATFORM_DRV_NAME[] = DEVICE_NAME;

static struct resource net_resources[ 2 ] = {
{
	.start		= KS_IO_BASE + KS_WAN_DMA_TX,
	.end		= KS_IO_BASE + KS_WAN_DMA_TX + BASE_IO_RANGE - 1,
	.flags		= IORESOURCE_MEM,
},
{
	.start		= KS_IO_BASE + KS_LAN_DMA_TX,
	.end		= KS_IO_BASE + KS_LAN_DMA_TX + BASE_IO_RANGE - 1,
	.flags		= IORESOURCE_MEM,
}
};

static int netdev_resume (

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
    struct platform_device *pdev )
{
    struct net_device *dev = platform_get_drvdata( pdev );

#else
    struct device *pdev,
    u32            level )
{
    struct net_device *dev = dev_get_drvdata( pdev );

    if ( RESUME_ENABLE == level )
#endif

    if ( netif_running( dev ) ) {
        netdev_open( dev );
        netif_device_attach( dev );
    }
    return 0;
}  /* netdev_resume */


static int netdev_suspend (

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
    struct platform_device *pdev,
    pm_message_t            state )
{
    struct net_device *dev = platform_get_drvdata( pdev );

#else
    struct device *pdev,

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14))
    pm_message_t   state,

#else
    u32            state,
#endif
    u32            level )
{
    struct net_device *dev = dev_get_drvdata( pdev );

    if ( SUSPEND_DISABLE == level )
#endif

    if ( netif_running( dev ) ) {
        netif_device_detach( dev );
        netdev_close( dev );
    }
    return 0;
}  /* netdev_suspend */


static void netdev_shutdown (

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
    struct platform_device *pdev )
{
    struct net_device *dev = platform_get_drvdata( pdev );

#else
    struct device *pdev )
{
    struct net_device *dev = dev_get_drvdata( pdev );
#endif

    if ( netif_running( dev ) ) {
    }
}  /* netdev_shutdown */


static int __init netdev_probe (

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
    struct platform_device *pdev )

#else
    struct device *pdev )
#endif
{
    int                err;
    struct net_device* dev;

    dev = alloc_etherdev( sizeof( struct dev_priv ));
    if ( !dev )
        return -ENOMEM;
    NetDevice[ device_present ] = dev;

    err = netdev_create( dev );
    if ( err )
        return err;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
    platform_set_drvdata( pdev, dev );

#else
    dev_set_drvdata( pdev, dev );
#endif
    return 0;
}  /* netdev_probe */


static int netdev_remove (

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
    struct platform_device *pdev )
{
    struct net_device *dev = platform_get_drvdata( pdev );

    platform_set_drvdata( pdev, NULL );

#else
    struct device *pdev )
{
    struct net_device *dev = dev_get_drvdata( pdev );

    dev_set_drvdata( pdev, NULL );
#endif

    netdev_free( dev );
    return 0;
}  /* netdev_remove */


static void netdev_platform_release (
    struct device *device )
{
    struct platform_device *pdev;

    pdev = to_platform_device( device );
    kfree( pdev );
}  /* netdev_platform_release */


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
static struct platform_driver netdev_driver = {
    .probe      = netdev_probe,
    .remove     = netdev_remove,
    .suspend    = netdev_suspend,
    .resume     = netdev_resume,
    .shutdown   = netdev_shutdown,
    .driver     = {
        .name   = PLATFORM_DRV_NAME,
    },
};

#else
static struct device_driver netdev_driver = {
    .name       = PLATFORM_DRV_NAME,
    .bus        = &platform_bus_type,
    .probe      = netdev_probe,
    .remove     = netdev_remove,
    .suspend    = netdev_suspend,
    .resume     = netdev_resume,
    .shutdown   = netdev_shutdown,
};
#endif


struct platform_device* PlatformDevice[ MAX_DEVICES ];

static void platform_free_devices (
    int num )
{
    int                     i;
    struct platform_device* pdev;

    for ( i = 0; i < num; i++ ) {
        pdev = PlatformDevice[ i ];
        if ( pdev )
            platform_device_unregister( pdev );
    }
}  /* platform_free_devices */


static int __init platform_init ( void )
{
    int                     err;
    int                     i;
    int                     num = 0;
    struct platform_device* pdev;

    memset( PlatformDevice, 0, sizeof( PlatformDevice ));
    for ( i = 0; i < MAX_DEVICES; i++ ) {
        if ( !( pdev = kmalloc( sizeof( struct platform_device ),
                GFP_KERNEL )) ) {
            err = -ENOMEM;
            goto device_reg_err;
        }

        memset( pdev, 0, sizeof( struct platform_device ));
        pdev->name = PLATFORM_DRV_NAME;
        pdev->id = i;
        pdev->dev.release = netdev_platform_release;
        pdev->num_resources = 1;
        pdev->resource = &net_resources[ i ];

        if ( platform_device_register( pdev ) ) {
            if ( device_present ) {
                break;
            }
            err = -ENODEV;
            goto device_reg_err;
        }

        PlatformDevice[ i ] = pdev;
        ++num;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
    err = platform_driver_register( &netdev_driver );

#else
    err = driver_register( &netdev_driver );
#endif
    if ( err )
        goto device_reg_err;

    return 0;

device_reg_err:
    if ( num ) {
        platform_free_devices( num );
    }
    return err;
}  /* platform_init */


static void __exit platform_exit ( void )
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
    platform_driver_unregister( &netdev_driver );

#else
    driver_unregister( &netdev_driver );
#endif
    platform_free_devices( device_present );
}  /* platform_exit */


#else
static int __init platform_init ( void )
{
    struct net_device* dev;
    int                i;
    int                result;

    for ( i = 0; i < MAX_DEVICES; i++ ) {
        dev = alloc_etherdev( sizeof( struct dev_priv ));
        if ( !dev ) {
            if ( device_present )
                break;
            return -ENOMEM;
        }
        NetDevice[ i ] = dev;
        result = netdev_create( NetDevice[ i ]);
        if ( result < 0 ) {
            if ( device_present )
                break;
            return result;
        }
    }
    return 0;
}  /* platform_init */


static void __exit platform_exit ( void )
{
    int i;

    for ( i = 0; i < device_present; i++ ) {
        netdev_free( NetDevice[ i ]);
    }
}  /* platform_exit */
#endif


/*
    init_module

    Description:
        This function starts the device driver.

    Parameters:
        None.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

#ifdef MODULE
int __init init_module ( void )
#else
static
int __init netdev_init_module ( void )
#endif
{
    int result;

#ifdef DBG
    printk( "I/O = %x; IRQ = %d\n", io, irq );
#endif

#ifdef DBG
    strDebug = ( char* ) kmalloc( DEBUG_MSG_SIZE, GFP_KERNEL );
    if ( strDebug ) {
        strMsg = strDebug;
        *strMsg = '\0';
        strIntBuf = strDebug + PRINT_MSG_SIZE + DEBUG_MSG_BUF;
        strIntMsg = strIntBuf;
        *strIntMsg = '\0';
    }
    else
        return -ENOMEM;
#endif

#ifdef CONFIG_PEGASUS
    HardwareInitPhyCtrl();
    if ( ks8001_InitPhy() )
        ;
    else if ( vsc8201_InitPhy() )
        ;
    else
        Generic_InitPhy();
#endif
    printk(KERN_INFO "%s\n", version );

    result = platform_init();
    if ( result < 0 ) {
        return result;
    }

#ifdef CONFIG_PEGASUS
    request_mdio_irq( NetDevice[ 0 ]);
#endif

    return 0;
}  /* init_module */


/*
    cleanup_module

    Description:
        This routine unloads the device driver.

    Parameters:
        None.

    Return (None):
*/

#ifdef MODULE
void __exit cleanup_module ( void )
#else
static
void __exit netdev_cleanup_module ( void )
#endif
{
#ifdef DBG
    if ( strDebug ) {
        if ( strMsg != strDebug ) {
            printk( "%s\n", strDebug );
        }
        if ( strIntMsg != strIntBuf ) {
            printk( "%s\n", strIntBuf );
        }
        kfree( strDebug );
    }
#endif

#ifdef CONFIG_PEGASUS
    free_mdio_irq( NetDevice[ 0 ]);
#endif

    platform_exit();
}  /* cleanup_module */


#ifndef MODULE
module_init(netdev_init_module);
module_exit(netdev_cleanup_module);
#endif

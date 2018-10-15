/* ---------------------------------------------------------------------------
          Copyright (c) 2006-2008 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    interrupt.c - Linux network device driver interrupt processing.

    Author      Date        Description
    THa         12/05/06    Created file.
    THa         11/01/07    Add delay enable receive interrupt code for
                            testing purpose.
    THa         11/20/07    Allocate socket buffers close to MTU so that
                            skb->truesize is used for correct TCP window
                            calculation.
   ---------------------------------------------------------------------------
*/


#include <linux/pci.h>
#include "target.h"
#include "hardware.h"
#include "device.h"
#include <linux/etherdevice.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/if_vlan.h>


#if 0
#define RCV_UNTIL_NONE
#endif

#ifdef ALIGN_IP_HEADER
#if 0
#define SKB_COPY_MAX  64
#endif
#endif

#if 1
#define DELAY_TX_INTERRUPT
#endif


#ifdef CONFIG_KSZ_NAPI
#define ksz_rx_skb                    netif_receive_skb
#define ksz_rx_quota( count, quota )  min( count, quota )
#else
#define ksz_rx_skb                    netif_rx
#define ksz_rx_quota( count, quota )  count
#endif

/* -------------------------------------------------------------------------- */

#ifdef USE_MULTIPLE_RX_DESC
static struct sk_buff* last_skb = NULL;
static int skb_index = 0;
static int skb_len = 0;
#endif

#ifdef TEST_RX_LOOPBACK
static int rx_loopback = 0;
#endif

static inline int dev_rcv_packets (
    struct net_device* dev )
{
    int              iNext;
    TDescStat        status;
    int              packet_len;
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    PTDescInfo       pInfo = &pHardware->m_RxDescInfo;

#ifndef RCV_UNTIL_NONE
#ifdef CONFIG_KSZ_NAPI
    int              cnLeft = dev->quota;

#else
    int              cnLeft = pInfo->cnAlloc;
#endif
#endif
    PTDesc           pDesc;
    PDMA_BUFFER      pDma;
    struct sk_buff*  skb;
    int              received = 0;
    int              rx_status;

#ifdef ALIGN_IP_HEADER
    int              aligned;
#endif

    iNext = pInfo->iNext;

#ifdef RCV_UNTIL_NONE
    while ( 1 ) {

#else
    while ( cnLeft-- ) {
#endif

        /* Get next descriptor which is not hardware owned. */
        GetReceivedPacket( pInfo, iNext, pDesc, status.ulData );
        if ( status.rx.fHWOwned )
            break;

#ifdef DEBUG_COUNTER
        pHardware->m_nGood[ COUNT_GOOD_NEXT_PACKET ]++;
#endif

#ifdef CHECK_OVERRUN
        if ( !( pDesc->pCheck->Control.ulData & CPU_TO_LE32( DESC_HW_OWNED )) )
        {

#ifdef DEBUG_OVERRUN
            pHardware->m_ulDropped++;
#endif
            ReleasePacket( pDesc );
            FreeReceivedPacket( pInfo, iNext );

#ifndef RCV_UNTIL_NONE
            cnLeft = pInfo->cnAlloc;
#endif
            continue;
        }
#endif

        /* From hardware.c:HardwareReceive() */
        /* Status valid only when last descriptor bit is set. */
#ifndef USE_MULTIPLE_RX_DESC
        if ( status.rx.fLastDesc  &&  status.rx.fFirstDesc )
#else
        if ( status.rx.fLastDesc )
#endif
        {
#if defined( CHECK_RCV_ERRORS )  ||  defined( RCV_HUGE_FRAME )
            /* Receive without error.  With receive errors disabled, packets
               with receive errors will be dropped, so no need to check the
               error bit.
            */
            if ( !status.rx.fError

#ifdef RCV_HUGE_FRAME
                    ||  ( status.ulData & (
                    DESC_RX_ERROR_CRC | DESC_RX_ERROR_RUNT |
                    DESC_RX_ERROR_TOO_LONG | DESC_RX_ERROR_PHY )) ==
                    DESC_RX_ERROR_TOO_LONG
#endif
                    )
#endif
            {

#ifdef ALIGN_IP_HEADER
                aligned = status.rx.bAligned;
#endif

#ifndef NO_STATS
                if ( status.rx.fMulticast )
                {
                    pHardware->m_cnCounter
                        [ OID_COUNTER_MULTICAST_FRAMES_RCV ]+=1;
#ifdef DEBUG_RX
                    DBG_PRINT( "M " );
#endif
                }
#endif

                /* received length includes 4-byte CRC */
#ifdef TEST_RX_LOOPBACK
                packet_len = status.rx.wFrameLen;
#else
                packet_len = status.rx.wFrameLen - 4;
#endif

#if defined( DEBUG_RX )  &&  !defined( DEBUG_RX_DATA )
        if ( status.rx.fError )
            DBG_PRINT( "E: %08x, %d"NEWLINE, status.ulData, packet_len );
#endif

#if 0
               if ( packet_len < 60 ) {
                   packet_len += 2048;
               }
#endif

        pDma = DMA_BUFFER( pDesc );
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
        dma_sync_single_for_cpu(( struct device* ) NULL, pDma->dma,
#else
        consistent_sync( dma_to_virt((( struct device* ) NULL ), pDma->dma ),
#endif
            packet_len + 4,
            DMA_FROM_DEVICE );

#ifdef ALIGN_IP_HEADER
#ifdef SKB_COPY_MAX
    if ( packet_len > SKB_COPY_MAX )
#endif
    {
        skb = pDma->skb;

        /* skb->data != skb->head */
        pDma->skb = dev_alloc_skb( pDma->len );
        if ( !pDma->skb ) {
            pDma->skb = skb;
            priv->stats.rx_dropped++;
            goto release_packet;
        }

        pDma->dma = virt_to_dma((( struct device* ) NULL ), pDma->skb->data );

        /* Set descriptor. */
        SetReceiveBuffer( pDesc, pDma->dma );

        if ( aligned ) {
            skb_reserve( skb, aligned );
        }
        skb_put( skb, packet_len );
        skb->truesize = packet_len + sizeof( struct sk_buff );
    }
#ifdef SKB_COPY_MAX
    else
#endif
#endif

#if !defined( ALIGN_IP_HEADER ) || defined( SKB_COPY_MAX )
    {
#ifdef USE_MULTIPLE_RX_DESC
        if ( last_skb ) {
            memcpy( &last_skb->data[ skb_index ], pDma->skb->data, skb_len );
            skb = last_skb;
            last_skb = NULL;
        }
        else
#endif
        {
        /* skb->data != skb->head */
        skb = dev_alloc_skb( packet_len + 2 );
        if ( !skb ) {
            priv->stats.rx_dropped++;
            goto release_packet;
        }

        /* Align socket buffer in 4-byte boundary for better performance. */
        skb_reserve( skb, 2 );

#ifdef ALIGN_IP_HEADER
        memcpy( skb_put( skb, packet_len ), &pDma->skb->data[ aligned ],
            packet_len );
#else
        memcpy( skb_put( skb, packet_len ), pDma->skb->data, packet_len );
#endif
	}
    }
#endif

#if 0
#ifndef NO_STATS
        /* Descriptor does not have broadcast indication. */
        if ( 0xFF == skb->data[ 0 ] ) {
            pHardware->m_cnCounter
                [ OID_COUNTER_BROADCAST_FRAMES_RCV ]++;
            pHardware->m_cnCounter
                [ OID_COUNTER_BROADCAST_BYTES_RCV ] +=
                    packet_len;
            pHardware->m_cnCounter
                [ OID_COUNTER_MULTICAST_FRAMES_RCV ]--;
        }
        else if ( pBuffer[ 0 ] != 0x01 )
        {
            pHardware->m_cnCounter
                [ OID_COUNTER_UNICAST_FRAMES_RCV ]++;

            pHardware->m_cnCounter
                [ OID_COUNTER_DIRECTED_FRAMES_RCV ]++;
            pHardware->m_cnCounter
                [ OID_COUNTER_DIRECTED_BYTES_RCV ] +=
                    packet_len;
        }
#endif
#endif

#ifdef DEBUG_RX_DATA
    {
        int    nLength;
        UCHAR* bData = ( PUCHAR ) skb->data;

#if !defined( VERIFY_LOOPBACK ) && !defined( DEBUG_RX_ALL )
        if ( 0xFF != bData[ 0 ] )
#endif
        {
            DBG_PRINT( "R: %08x, %d"NEWLINE, status.ulData, packet_len );
            pHardware->m_nPacketLen = packet_len;
#ifdef DEBUG_DATA_HEADER
            if ( pHardware->m_nPacketLen > 0x40 )
                pHardware->m_nPacketLen = 0x40;
#endif
            for ( nLength = 0; nLength < pHardware->m_nPacketLen; nLength++ )
            {
                DBG_PRINT( "%02X ", bData[ nLength ]);
                if ( ( nLength % 16 ) == 15 )
                {
                    DBG_PRINT( NEWLINE );
                }
            }
            if ( ( nLength % 16 ) )
                DBG_PRINT( NEWLINE );
        }
    }
#endif

        skb->dev = dev;

#ifdef TEST_RX_LOOPBACK
        rx_loopback = 1;
        if ( !rx_loopback )
#endif
        skb->protocol = eth_type_trans( skb, dev );

#if RXCHECKSUM_DEFAULT
#ifdef CONFIG_KSZ8692VA
if ( pHardware->m_dwReceiveConfig &
        ( DMA_RX_CTRL_CSUM_UDP | DMA_RX_CTRL_CSUM_TCP ) )
{
        unsigned short protocol;
        struct iphdr* iph;

        protocol = skb->protocol;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22))
        skb->nh.raw = skb->data;
        iph = skb->nh.iph;
#else
        skb_reset_network_header( skb );
        iph = ( struct iphdr* ) skb_network_header( skb );
#endif
        if ( protocol == htons( ETH_P_8021Q ) ) {
            protocol = iph->tot_len;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22))
            skb->nh.raw += VLAN_HLEN;
            iph = skb->nh.iph;
#else
            skb_set_network_header( skb, VLAN_HLEN );
            iph = ( struct iphdr* ) skb_network_header( skb );
#endif
        }
        if ( protocol == htons( ETH_P_IP ) ) {
            if ( iph->protocol == IPPROTO_TCP ) {

#ifdef RCV_HUGE_FRAME_
            if ( !status.rx.fCsumErrTCP )
#endif
                skb->ip_summed = CHECKSUM_UNNECESSARY;
            }
        }
}
#endif
#ifdef CONFIG_KSZ8692VB
        if ( 0 == status.rx.fCsumError  &&  0 == status.rx.fCsumNotDone ) {
            skb->ip_summed = CHECKSUM_UNNECESSARY;
        }
#endif
#endif

        /* Update receive statistics. */
        priv->stats.rx_packets++;
        priv->stats.rx_bytes += packet_len;

#ifdef TEST_RX_LOOPBACK
        if ( rx_loopback )
        {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22))
            skb->nh.raw = skb->data + ETH_HLEN;
#else
            skb_set_network_header( skb, ETH_HLEN );
#endif
            ks8692_dev_transmit( skb, dev );
            rx_loopback = 0;
        }
        else
#endif
        rx_status = ksz_rx_skb( skb );
        received++;

#ifdef DEBUG_COUNTER
        pHardware->m_nGood[ COUNT_GOOD_RCV_COMPLETE ]++;
#endif

#ifdef DEBUG_OVERRUN
        pHardware->m_ulReceived++;
#endif

            }

#ifdef CHECK_RCV_ERRORS
            /*
             *  Receive with error.
             */
            else {

                /* Update receive error statistics. */

                pHardware->m_cnCounter
                    [ OID_COUNTER_RCV_ERROR ]+=1;

#ifndef NO_STATS
                if ( status.rx.fErrCRC )
                    pHardware->m_cnCounter
                        [ OID_COUNTER_RCV_ERROR_CRC ]+=1;

                if ( status.rx.fErrRunt )
                    pHardware->m_cnCounter
                        [ OID_COUNTER_RCV_ERROR_RUNT ]+=1;

                if ( status.rx.fErrTooLong )
                    pHardware->m_cnCounter
                        [ OID_COUNTER_RCV_ERROR_TOOLONG ]+=1;

                if ( status.rx.fErrPHY )
                    pHardware->m_cnCounter
                        [ OID_COUNTER_RCV_ERROR_MII ]+=1;
#endif

#ifdef DEBUG_RX
                DBG_PRINT( "  RX: %08X"NEWLINE, status.ulData );
#endif

#ifdef DEBUG_COUNTER
                pHardware->m_nBad[ COUNT_BAD_RCV_FRAME ]+=1;
#endif

            }

/* Hardware checksum errors are not associated with receive errors. */
#if RXCHECKSUM_DEFAULT

/* Hardware cannot handle UDP packet in IP fragments. */
#if 0
            if ( status.rx.fCsumErrUDP )
                pHardware->m_cnCounter
                    [ OID_COUNTER_RCV_ERROR_UDP ]+=1;
#endif

            if ( status.rx.fCsumErrTCP )
                pHardware->m_cnCounter
                    [ OID_COUNTER_RCV_ERROR_TCP ]+=1;

            if ( status.rx.fCsumErrIP )
                pHardware->m_cnCounter
                    [ OID_COUNTER_RCV_ERROR_IP ]+=1;
#endif

#else
#ifdef RCV_HUGE_FRAME
            else {
#ifdef DEBUG_RX
                DBG_PRINT( "  RX: %08X"NEWLINE, status.ulData );
#endif
            }
#endif
#endif
        }

#ifdef USE_MULTIPLE_RX_DESC
        else if ( !status.rx.fError  ||  ( status.ulData & (
                DESC_RX_ERROR_CRC | DESC_RX_ERROR_RUNT |
                DESC_RX_ERROR_TOO_LONG | DESC_RX_ERROR_PHY )) ==
                DESC_RX_ERROR_TOO_LONG ) {
            PDMA_BUFFER pDma = DMA_BUFFER( pDesc );

            /* received length includes 4-byte CRC */
            packet_len = status.rx.wFrameLen - 4;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
            dma_sync_single_for_cpu(( struct device* ) NULL, pDma->dma,
#else
            consistent_sync( dma_to_virt((( struct device* ) NULL ),
                pDma->dma ),
#endif
                pDma->len,
                DMA_FROM_DEVICE );

            if ( !last_skb ) {

                /* Allocate extra bytes for 32-bit transfer. */
                last_skb = dev_alloc_skb( packet_len + 6 );
                if ( !last_skb ) {
                    priv->stats.rx_dropped++;
                    goto release_packet;
                }

                skb_reserve( last_skb, 2 );
                skb_put( last_skb, packet_len );
                skb_index = 0;
                skb_len = packet_len + 4;
            }
            memcpy( &last_skb->data[ skb_index ], pDma->skb->data, pDma->len );
            skb_index += pDma->len;
            skb_len -= pDma->len;
        }
#endif

#ifdef DEBUG_RX
        else {
            DBG_PRINT( " ? %08x"NEWLINE, status.ulData );
        }
#endif

release_packet:
        ReleasePacket( pDesc );
        FreeReceivedPacket( pInfo, iNext );
    }
    pInfo->iNext = iNext;

#ifdef DEBUG_COUNTER
    if ( !received )
        pHardware->m_nGood[ COUNT_GOOD_NO_NEXT_PACKET ]++;
    else if ( 1 == received )
        pHardware->m_nGood[ COUNT_GOOD_RCV_CNT_1 ]++;
    else if ( 2 == received )
        pHardware->m_nGood[ COUNT_GOOD_RCV_CNT_2 ]++;
    else
        pHardware->m_nGood[ COUNT_GOOD_RCV_CNT_3 ]++;
#endif
    return( received );
}  /* dev_rcv_packets */

/* -------------------------------------------------------------------------- */

#ifdef CONFIG_DELAYED_TIMER
void DelayRxIntrTask (
    unsigned long data )
{
    struct net_device* dev = ( struct net_device* ) data;
    struct dev_priv*   priv = ( struct dev_priv* ) dev->priv;
    struct dev_info*   hw_priv = priv->pDevInfo;
    PHARDWARE          pHardware = &hw_priv->hw;

    /* tasklets are interruptible. */
    spin_lock_irq( &priv->lock );
    HardwareTurnOnInterrupt( pHardware, ( INT_RX | INT_RX_OVERRUN ),
        NULL );
    spin_unlock_irq( &priv->lock );
}  /* DelayRxIntrTask */
#endif


void RxProcessTask (
    unsigned long data )
{
    struct net_device* dev = ( struct net_device* ) data;
    struct dev_priv*   priv = netdev_priv(dev);
    struct dev_info*   hw_priv = priv->pDevInfo;
    PHARDWARE          pHardware = &hw_priv->hw;

    if ( !pHardware->m_bEnabled )
        return;
    if ( unlikely( !dev_rcv_packets( dev )) ) {

        /* tasklets are interruptible. */
        spin_lock_irq( &priv->lock );
        HardwareTurnOnInterrupt( pHardware, ( INT_RX | INT_RX_OVERRUN ),
            NULL );
        spin_unlock_irq( &priv->lock );
    }
    else {
        HardwareAcknowledgeInterrupt( pHardware, INT_RX );
        tasklet_schedule( &hw_priv->rx_tasklet );
    }
}  /* RxProcessTask */


void TxProcessTask (
    unsigned long data )
{
    struct net_device* dev = ( struct net_device* ) data;
    struct dev_priv*   priv = netdev_priv(dev);
    struct dev_info*   hw_priv = priv->pDevInfo;
    PHARDWARE          pHardware = &hw_priv->hw;

    HardwareAcknowledgeInterrupt( pHardware, INT_TX | INT_TX_EMPTY );

    TRANSMIT_DONE( dev );

    /* tasklets are interruptible. */
    spin_lock_irq( &priv->lock );
    HardwareTurnOnInterrupt( pHardware, INT_TX, NULL );
    spin_unlock_irq( &priv->lock );
}  /* TxProcessTask */


#ifdef CONFIG_KSZ_NAPI
/*
    dev_poll

    Description:
        This function is used to poll the device for received buffers.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        int* budget
            Remaining number of packets allowed to report to the stack.

    Return (int):
        Zero if all packets reported; one to indicate more.
*/

int
ks8692_\
dev_poll (
    struct net_device* dev,
    int*               budget )
{
    struct dev_priv* priv = ( struct dev_priv* ) dev->priv;
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              received;
    int              left = min( *budget, dev->quota );

    HardwareAcknowledgeInterrupt( pHardware, INT_RX );

    received = dev_rcv_packets( dev );

    dev->quota -= received;
    *budget -= received;

    if ( received < left ) {
        netif_rx_complete( dev );

        /* tasklets are interruptible. */
        spin_lock_irq( &priv->lock );
        HardwareTurnOnInterrupt( pHardware, ( INT_RX | INT_RX_OVERRUN ),
            NULL );
        spin_unlock_irq( &priv->lock );
    }

    return( received >= left );
}  /* dev_poll */
#endif


/*
    dev_interrupt

    Description:
        This routine is called by upper network layer to signal interrupt.

    Parameters:
        int irq
            Interrupt number.

        void* dev_id
            Pointer to network device.

        struct pt_regs regs
            Pointer to registers.

    Return (None):
*/

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
void

#else
irqreturn_t
#endif
ks8692_\
dev_interrupt (
    int             irq,
    void*           dev_id
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))
    ,
    struct pt_regs* regs
#endif
    )
{
    UINT               IntEnable = 0;
    struct net_device* dev = ( struct net_device* ) dev_id;
    struct dev_priv*   priv = netdev_priv(dev);
    struct dev_info*   hw_priv = priv->pDevInfo;
    PHARDWARE          pHardware = &hw_priv->hw;

    HardwareReadInterrupt( pHardware, &IntEnable );

#ifdef DEBUG_COUNTER
    pHardware->m_nGood[ COUNT_GOOD_INT ]++;
#endif
    do {

#ifdef DEBUG_COUNTER
        pHardware->m_nGood[ COUNT_GOOD_INT_LOOP ]++;
#endif

#ifdef DEBUG_INTERRUPT
        DBG_PRINT( "I: %08X"NEWLINE, IntEnable );
#endif
        HardwareAcknowledgeInterrupt( pHardware, IntEnable );
        IntEnable &= pHardware->m_ulInterruptMask;

        /* All packets have been sent. */

#ifdef DELAY_TX_INTERRUPT
        if ( unlikely( IntEnable & ( INT_TX | INT_TX_EMPTY )) ) {
            HardwareTurnOffInterrupt( pHardware, ( INT_TX | INT_TX_EMPTY ));
            tasklet_schedule( &hw_priv->tx_tasklet );
        }

#else
        if ( ( IntEnable & INT_TX_EMPTY ) ) {

            /* Also acknowledge transmit complete interrupt. */
            HardwareAcknowledgeTransmit( pHardware );

            TRANSMIT_DONE( dev );
        }

        /* Do not need to process transmit complete as all transmitted
           descriptors are freed.
        */
        else
        if ( ( IntEnable & INT_TX ) ) {
            TRANSMIT_DONE( dev );
        }
#endif

        if ( likely( IntEnable & INT_RX ) ) {

#ifdef DEBUG_COUNTER
            pHardware->m_nGood[ COUNT_GOOD_INT_RX ]++;
#endif

#ifdef CONFIG_KSZ_NAPI
            HardwareTurnOffInterrupt( pHardware,
                ( INT_RX | INT_RX_OVERRUN ));
            if ( likely( netif_rx_schedule_prep( dev )) ) {

                /* tell system we have work to be done. */
                __netif_rx_schedule( dev );
            }

#else
            HardwareTurnOffInterrupt( pHardware,
                ( INT_RX | INT_RX_OVERRUN ));
            tasklet_schedule( &hw_priv->rx_tasklet );
#endif
        }

#if 1
        if ( unlikely( IntEnable & INT_RX_OVERRUN ) ) {

#ifdef DBG_
            DBG_PRINT( "Rx overrun"NEWLINE );
#endif

            pHardware->m_cnCounter[ OID_COUNTER_RCV_NO_BUFFER ]++;
            priv->stats.rx_fifo_errors++;

            HardwareResumeReceive( pHardware );
        }
#endif

#if 0
        if ( ( IntEnable & INT_RX_STOPPED ) ) {

#ifdef DBG
            DBG_PRINT( "Rx stopped"NEWLINE );
#endif

            /* Receive just has been stopped. */
            if ( 0 == pHardware->m_bReceiveStop ) {
                pHardware->m_ulInterruptMask &= ~INT_RX_STOPPED;
            }
            else if ( pHardware->m_bReceiveStop > 1 ) {
                if ( pHardware->m_bEnabled  &&
                        ( pHardware->m_dwReceiveConfig &
                        DMA_RX_CTRL_ENABLE ) ) {

#ifdef DBG
                    DBG_PRINT( "Rx disabled"NEWLINE );
#endif
                    HardwareStartReceive( pHardware );
                }
                else {

#ifdef DBG
                    DBG_PRINT( "Rx stop disabled:%d"NEWLINE,
                        pHardware->m_bReceiveStop );
#endif
                    pHardware->m_ulInterruptMask &= ~INT_RX_STOPPED;
                    pHardware->m_bReceiveStop = 0;
                }
            }

            /* Receive just has been started. */
            else {
                pHardware->m_bReceiveStop++;
            }
            HardwareAcknowledgeInterrupt( pHardware, INT_RX_STOPPED );
            break;
        }

#if 0
        if ( ( IntEnable & INT_TX_STOPPED ) ) {
            DWORD dwData;

            pHardware->m_ulInterruptMask &= ~INT_TX_STOPPED;
            DBG_PRINT( "Tx stopped"NEWLINE );
            HW_READ_DWORD( pHardware, REG_DMA_TX_CTRL, &dwData );
            if ( !( dwData & DMA_TX_CTRL_ENABLE ) ) {
                DBG_PRINT( "Tx disabled"NEWLINE );
            }
            HardwareAcknowledgeInterrupt( pHardware, INT_TX_STOPPED );
            break;
        }
#endif
#endif
    } while ( 0 );

    IntEnable = KS_READ_REG( KS_INT_ENABLE );
    IntEnable &= ~( INT_CHECK << pHardware->m_nShift );
    IntEnable |= pHardware->m_ulInterruptMask << pHardware->m_nShift;
    KS_WRITE_REG( KS_INT_ENABLE, IntEnable );

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
#ifdef HAVE_DELAY_SOFTIRQ
    delay_softirq = 1;
#endif
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
    return IRQ_HANDLED;
#endif
}  /* dev_interrupt */

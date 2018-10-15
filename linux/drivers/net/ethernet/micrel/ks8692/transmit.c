/* ---------------------------------------------------------------------------
          Copyright (c) 2006-2008 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    transmit.c - Linux network device driver transmit processing.

    Author      Date        Description
    THa         12/05/06    Created file.
    Ken         06/22/07    Updated for dma usage instead of PCI usage
    THa         08/29/07    Update for Linux 2.6.21.
    THa         07/24/08    Workaround transmit failure problem.
   ---------------------------------------------------------------------------
*/


#include <linux/pci.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <asm/irq.h>
#include "target.h"
#include "hardware.h"
#include "device.h"

#ifdef VERIFY_CSUM_GEN
#include <net/ip.h>
#include <net/ipv6.h>
#endif


#if 1
#define SKB_WAITING
#endif

/* -------------------------------------------------------------------------- */

static inline PDMA_BUFFER alloc_tx_buf (
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
        /* Remember current transmit buffer. */
        pAdapter->m_TxBufInfo.pCurrent = pDma;
#endif

        /* Link to current descriptor. */
        pCurrent->pReserved = pDma;
    }
    return( pDma );
}  /* alloc_tx_buf */


static inline void free_tx_buf (
    PBUFFER_INFO pBufInfo,
    PDMA_BUFFER  pDma )
{
    pDma->pNext = pBufInfo->pTop;
    pBufInfo->pTop = pDma;
    pBufInfo->cnAvail++;
}  /* free_tx_buf */

/* -------------------------------------------------------------------------- */

/*
    send_packet

    Description:
        This function is used to send a packet out to the network.

    Parameters:
        struct sk_buff* skb
            Pointer to socket buffer.

        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

static int send_packet (
    struct sk_buff*    skb,
    struct net_device* dev )
{
    PTDesc           pDesc;
    PTDesc           pFirstDesc;
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    PTDescInfo       pInfo = &pHardware->m_TxDescInfo;
    PDMA_BUFFER      pDma;
    int              len;

#ifdef SCATTER_GATHER
    int              last_frag = skb_shinfo( skb )->nr_frags;
#endif
    int              rc = 1;

#if ETH_ZLEN > 60
    len = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;
#else
    /* Hardware will pad the length to 60. */
    len = skb->len;
#endif

    /* Remember the very first descriptor. */
    pFirstDesc = pInfo->pCurrent;
    pDesc = pFirstDesc;

    pDma = alloc_tx_buf( &hw_priv->m_TxBufInfo, pDesc );

#ifdef SCATTER_GATHER
    if ( last_frag ) {
        int         frag;
        skb_frag_t* this_frag;

        pDma->len = skb->len - skb->data_len;

#ifdef HIGH_MEM_SUPPORT
        pDma->dma = dma_map_page(
            hw_priv->pdev, virt_to_page( skb->data ),
            (( unsigned long ) skb->data & ~PAGE_MASK ), pDma->len,
            DMA_TO_DEVICE );
#else
        pDma->dma = dma_map_single(
            hw_priv->pdev, skb->data, pDma->len,
            DMA_TO_DEVICE );
#endif
        SetTransmitBuffer( pDesc, pDma->dma );
        SetTransmitLength( pDesc, pDma->len );

        frag = 0;
        do {
            this_frag = &skb_shinfo( skb )->frags[ frag ];

            /* Get a new descriptor. */
            GetTxPacket( pInfo, pDesc );

#ifdef SKIP_TX_INT
            ++pHardware->m_TxIntCnt;
#endif

            pDma = alloc_tx_buf( &hw_priv->m_TxBufInfo, pDesc );
            pDma->len = this_frag->size;

#ifdef HIGH_MEM_SUPPORT
            pDma->dma = dma_map_page(
                hw_priv->pdev, this_frag->page,
                this_frag->page_offset, pDma->len,
                DMA_TO_DEVICE );
#else
            pDma->dma = dma_map_single(
                hw_priv->pdev,
                page_address( this_frag->page ) + this_frag->page_offset,
                pDma->len,
                DMA_TO_DEVICE );
#endif
            SetTransmitBuffer( pDesc, pDma->dma );
            SetTransmitLength( pDesc, pDma->len );

            frag++;
            if ( frag == last_frag )
                break;

            /* Do not release the last descriptor here. */
            ReleasePacket( pDesc );
        } while ( 1 );

        /* pCurrent points to the last descriptor. */
        pInfo->pCurrent = pDesc;

        /* Release the first descriptor. */
        ReleasePacket( pFirstDesc );
    }
    else
#endif
    {

#ifdef VERIFY_CSUM_GEN
        if ( skb->protocol == htons( ETH_P_IP ) ) {
            struct iphdr* iph;
            int           offset;
            USHORT*       checksum;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22))
            iph = skb->nh.iph;
#else
            iph = ( struct iphdr* ) skb_network_header( skb );
#endif
            offset = ntohs( iph->frag_off );
            checksum = ( USHORT* )(( char* ) iph + iph->ihl * 4 );

            /* Not an IP fragment. */
            if ( !( offset & ( IP_MF | IP_OFFSET )) ) {
                if ( iph->protocol == IPPROTO_TCP ) {

#ifndef VERIFY_NO_CSUM_GEN
                    ( pDesc )->sw.BufSize.tx.fCsumGenTCP = TRUE;
#endif
                    checksum += 8;
                    *checksum = CSUM_VAL;
                }
                else if ( iph->protocol == IPPROTO_UDP ) {

#ifndef VERIFY_NO_CSUM_GEN
                    ( pDesc )->sw.BufSize.tx.fCsumGenUDP = TRUE;
#endif
                    checksum += 3;
                    *checksum = CSUM_VAL;
                    checksum += 4;
                    *checksum = CSUM_VAL;
                }

#if defined( CONFIG_KSZ8692VB )
                else if ( iph->protocol == IPPROTO_ICMP ) {
                    checksum += 1;

#ifndef VERIFY_NO_CSUM_GEN
                    ( pDesc )->sw.BufSize.tx.fCsumGenICMP = TRUE;
/* THa  2008/10/07
   Enable ICMP by itself does not work.
*/
#if 1
                    ( pDesc )->sw.BufSize.tx.fCsumGenUDP = TRUE;
#endif
#endif
                    *checksum = CSUM_VAL;
                    checksum += 3;
                    *checksum = CSUM_VAL;
                }
#endif
            }

#ifdef VERIFY_NO_IP_CSUM_GEN
#ifndef VERIFY_NO_CSUM_GEN
            ( pDesc )->sw.BufSize.tx.fCsumGenIP = TRUE;
#endif
            checksum = &iph->check;
            *checksum = CSUM_VAL;
#endif
        }
        else if ( skb->protocol == htons( ETH_P_IPV6 ) ) {
            struct ipv6hdr* iph;
            USHORT*         checksum;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22))
            iph = ( struct ipv6hdr* ) skb->nh.iph;
#else
            iph = ( struct ipv6hdr* ) skb_network_header( skb );
#endif
            if ( iph->nexthdr != NEXTHDR_FRAGMENT ) {
                checksum = ( USHORT* )( iph + 1 );
                if ( iph->nexthdr == NEXTHDR_TCP ) {

#ifndef VERIFY_NO_CSUM_GEN
                    ( pDesc )->sw.BufSize.tx.fCsumGenTCP = TRUE;
#endif
                    checksum += 8;
                    *checksum = CSUM_VAL;
                }
                else if ( iph->nexthdr == NEXTHDR_UDP ) {

#ifndef VERIFY_NO_CSUM_GEN
                    ( pDesc )->sw.BufSize.tx.fCsumGenUDP = TRUE;
#endif
                    checksum += 3;
                    *checksum = CSUM_VAL;
                }
                else if ( iph->nexthdr == NEXTHDR_ICMP  &&
                        *checksum != ntohs( 0x8700 ) ) {

#ifndef VERIFY_NO_CSUM_GEN
                    ( pDesc )->sw.BufSize.tx.fCsumGenICMP = TRUE;
#endif
                    checksum += 1;
                    *checksum = CSUM_VAL;
                }
            }
        }
#endif

        pDma->len = len;

#ifdef HIGH_MEM_SUPPORT
        pDma->dma = dma_map_page(
            hw_priv->pdev, virt_to_page( skb->data ),
            (( unsigned long ) skb->data & ~PAGE_MASK ), pDma->len,
            DMA_TO_DEVICE );

#else
        pDma->dma = dma_map_single(
            hw_priv->pdev, skb->data, pDma->len,
            DMA_TO_DEVICE );
#endif
        SetTransmitBuffer( pDesc, pDma->dma );
        SetTransmitLength( pDesc, pDma->len );
    }

#if TXCHECKSUM_DEFAULT
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19))
    if ( skb->ip_summed == CHECKSUM_HW ) {
#else
    if ( skb->ip_summed == CHECKSUM_PARTIAL ) {
#endif
        struct iphdr* iph;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22))
        iph = skb->nh.iph;
#else
        iph = ( struct iphdr* ) skb_network_header( skb );
#endif
        if ( iph->protocol == IPPROTO_TCP ) {
            ( pDesc )->sw.BufSize.tx.fCsumGenTCP = TRUE;
        }
        else if ( iph->protocol == IPPROTO_UDP ) {
            ( pDesc )->sw.BufSize.tx.fCsumGenUDP = TRUE;
        }
    }
#endif

    /* The last descriptor holds the packet so that it can be returned to
       network subsystem after all descriptors are transmitted.
    */
    pDma->skb = skb;

#ifdef DEBUG_TX_DATA
    {
        int    nLength;
        UCHAR* bData = ( PUCHAR ) pDma->skb->data;

//        if ( 0xFF != bData[ 0 ] )
        {
            DBG_PRINT( "S: %d"NEWLINE, skb->len );
            pHardware->m_nPacketLen = pDma->skb->len;
#ifdef DEBUG_DATA_HEADER
            if ( pHardware->m_nPacketLen > 0x40 )
                pHardware->m_nPacketLen = 0x40;
#endif
            for ( nLength = 0; nLength < pHardware->m_nPacketLen; nLength++ )
            {
                DBG_PRINT( "%02x ", bData[ nLength ]);
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

    if ( HardwareSendPacket( pHardware ) ) {
        rc = 0;

        /* Update transmit statistics. */
#ifndef NO_STATS
        pHardware->m_cnCounter[ OID_COUNTER_DIRECTED_FRAMES_XMIT ]++;
        pHardware->m_cnCounter[ OID_COUNTER_DIRECTED_BYTES_XMIT ] +=
            len;
#endif
        priv->stats.tx_packets++;
        priv->stats.tx_bytes += len;

#if 0
        netif_trans_update(dev);
#endif
    }

#ifdef SKB_WAITING
    priv->skb_waiting = NULL;
#endif

    return( rc );
}  /* send_packet */


#ifdef SKB_WAITING
/*
    send_next_packet

    Description:
        This function is used to send the next packet waiting to be sent.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

static int send_next_packet (
    struct net_device* dev )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    int              rc = 1;

    if ( priv->skb_waiting ) {
        PHARDWARE pHardware = &hw_priv->hw;
        int       num = 1;

#ifdef SCATTER_GATHER
        num = skb_shinfo( priv->skb_waiting )->nr_frags + 1;
#endif
        if ( HardwareAllocPacket( pHardware, priv->skb_waiting->len, num )
                >= num ) {
            rc = send_packet( priv->skb_waiting, dev );
            netif_wake_queue( dev );
        }
    }
    return( rc );
}  /* send_next_packet */
#endif


/*
    transmit_done

    Description:
        This routine is called when the transmit interrupt is triggered,
        indicating either a packet is sent successfully or there are transmit
        errors.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (None):
*/

void
ks8692_\
transmit_done (
    struct net_device* dev )
{
    int              iLast;
    TDescStat        status;
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    PTDescInfo       pInfo = &pHardware->m_TxDescInfo;
    PTDesc           pDesc;
    PDMA_BUFFER      pDma;

    iLast = pInfo->iLast;

    while ( pInfo->cnAvail < pInfo->cnAlloc )
    {
        /* Get next descriptor which is not hardware owned. */
        GetTransmittedPacket( pInfo, iLast, pDesc, status.ulData );
        if ( status.tx.fHWOwned )
            break;

        pDma = DMA_BUFFER( pDesc );

        /* This descriptor contains a buffer. */
        if ( pDma )
        {

#ifdef HIGH_MEM_SUPPORT
            dma_unmap_page(
                hw_priv->pdev, pDma->dma, pDma->len,
                DMA_TO_DEVICE );
#else
            dma_unmap_single(
                hw_priv->pdev, pDma->dma, pDma->len,
                DMA_TO_DEVICE );
#endif

            /* This descriptor contains the last buffer in the packet. */
            if ( pDma->skb )
            {
                /* Notify the network subsystem that the packet has been sent.
                */

#ifdef DEBUG_TX_
    {
        int    nLength;
        UCHAR* bData = ( PUCHAR ) pDma->skb->data;

//        if ( 0xFF != bData[ 0 ] )
        {
            for ( nLength = 0; nLength < pDma->skb->len; nLength++ )
            {
                DBG_PRINT( "%02x ", bData[ nLength ]);
                if ( ( nLength % 16 ) == 15 )
                {
                    DBG_PRINT( NEWLINE );
                }
            }
            DBG_PRINT( NEWLINE );
        }
    }
#endif

#if 1
                netif_trans_update(pDma->skb->dev);
#endif

                /* Release the packet back to network subsystem. */
                dev_kfree_skb_irq( pDma->skb );
                pDma->skb = NULL;
            }

            /* Free the transmitted buffer. */
            free_tx_buf( &hw_priv->m_TxBufInfo, pDma );
            pDesc->pReserved = NULL;
        }

        /* Free the transmitted descriptor. */
        FreeTransmittedPacket( pInfo, iLast );
    }
    pInfo->iLast = iLast;

    if ( netif_queue_stopped( dev ) )

#ifdef SKB_WAITING
        send_next_packet( dev );

#else
        netif_wake_queue( dev );
#endif

}  /* transmit_done */


void transmit_reset (
    struct net_device* dev )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    PTDescInfo       pInfo = &pHardware->m_TxDescInfo;
    int              iLast;
    PTDesc           pDesc;
    PDMA_BUFFER      pDma;
    TDescStat        status;

    iLast = pInfo->iLast;

    while ( pInfo->cnAvail < pInfo->cnAlloc )
    {
        /* Get next descriptor which is hardware owned. */
        GetTransmittedPacket( pInfo, iLast, pDesc, status.ulData );
        if ( status.tx.fHWOwned )
        {
            ReleaseDescriptor( pDesc, status );
        }

        pDma = DMA_BUFFER( pDesc );

        /* This descriptor contains a buffer. */
        if ( pDma )
        {

#ifdef HIGH_MEM_SUPPORT
            dma_unmap_page(
                hw_priv->pdev, pDma->dma, pDma->len,
                DMA_TO_DEVICE );
#else
            dma_unmap_single(
                hw_priv->pdev, pDma->dma, pDma->len,
                DMA_TO_DEVICE );
#endif

            /* This descriptor contains the last buffer in the packet. */
            if ( pDma->skb )
            {
                /* Notify the network subsystem that the packet has not been
                   sent.
                */

                /* Release the packet back to network subsystem. */
                dev_kfree_skb( pDma->skb );
                pDma->skb = NULL;
            }

            /* Free the buffer. */
            free_tx_buf( &hw_priv->m_TxBufInfo, pDma );
            pDesc->pReserved = NULL;
        }

        /* Free the descriptor. */
        FreeTransmittedPacket( pInfo, iLast );
    }
    pInfo->iLast = iLast;
}  /* transmit_reset */

/* -------------------------------------------------------------------------- */

#if 0
static int insert_send = 0;
int netdev_change_mtu (
    struct net_device* dev,
    int                new_mtu );
#endif

/*
    dev_transmit

    Description:
        This function is used by the upper network layer to send out a packet.

    Parameters:
        struct sk_buff* skb
            Pointer to socket buffer.

        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code indicating failure.
*/

netdev_tx_t
ks8692_\
dev_transmit (
    struct sk_buff*    skb,
    struct net_device* dev )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              len;
    int              num = 1;
    int              rc, trc = NETDEV_TX_OK;

#ifndef CONFIG_PEGASUS
    struct sk_buff* org_skb = skb;

    if ( skb->len <= 48 ) {
        if ( skb->end - skb->data >= 50 ) {
            memset( &skb->data[ skb->len ], 0, 50 - skb->len );
            skb->len = 50;
        }
        else {
            skb = dev_alloc_skb( 50 );
            if ( !skb ) {
                return NETDEV_TX_BUSY;
            }
            memcpy( skb->data, org_skb->data, org_skb->len );
            memset( &skb->data[ org_skb->len ], 0, 50 - org_skb->len );
            skb->len = 50;
            skb->dev = org_skb->dev;
            skb->ip_summed = org_skb->ip_summed;
            skb->csum = org_skb->csum;

            skb->nh.raw = skb->data + ETH_HLEN;

            dev_kfree_skb( org_skb );
        }
    }
#endif

#if defined( DEBUG_TX ) && !defined( DEBUG_TX_DATA )
    DBG_PRINT( "S: %d"NEWLINE, skb->len );
#endif

#if 0
    if ( !insert_send  &&
            ( skb->protocol == htons( ETH_P_IP )  ||
            skb->protocol == htons( ETH_P_IPV6 )) ) {
        insert_send = 1;
        if ( skb->protocol == htons( ETH_P_IP ) ) {
            struct iphdr* iph;
            int           offset;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22))
            iph = skb->nh.iph;
#else
            iph = ( struct iphdr* ) skb_network_header( skb );
#endif
            offset = ntohs( iph->frag_off );

            /* Not an IP fragment. */
            if ( !( offset & ( IP_MF | IP_OFFSET )) )
                netdev_change_mtu( dev, 0 );
        }
#if 1
        else {
            struct ipv6hdr* iph;
            USHORT*         checksum;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22))
            iph = ( struct ipv6hdr* ) skb->nh.iph;
#else
            iph = ( struct ipv6hdr* ) skb_network_header( skb );
#endif
	    if ( iph->nexthdr != NEXTHDR_FRAGMENT ) {
                checksum = ( USHORT* )( iph + 1 );
                if ( iph->nexthdr == NEXTHDR_TCP  ||
                        iph->nexthdr == NEXTHDR_UDP  ||
                        ( iph->nexthdr == NEXTHDR_ICMP  &&
                        ( *checksum == ntohs( 0x8000 )  ||
                        *checksum == ntohs( 0x8100 ))) ) {
#if 0
                    netdev_change_mtu( dev, 1 );
#else
                    HW_WRITE_DWORD( pHardware, REG_DMA_TX_CTRL,
                        ( pHardware->m_dwTransmitConfig |
                        DMA_TX_CTRL_LOOPBACK ));
                    netdev_change_mtu( dev, 2 );
                    DelayMicrosec( 10 );
                    HW_WRITE_DWORD( pHardware, REG_DMA_TX_CTRL,
                        pHardware->m_dwTransmitConfig );
#endif
                }
            }
        }
#endif
        insert_send = 0;
    }
#endif

#ifdef TEST_MINIMUM_TX
    if ( skb->len >= 1600 ) {
        skb->len -= 1600;
        DBG_PRINT( "min tx: %d"NEWLINE, skb->len );
    }
#endif

#ifdef SKB_WAITING
    /* There is a packet waiting to be sent.  Transmit queue should already be
       stopped.
    */
    if ( priv->skb_waiting ) {

#ifdef DBG
        printk( "waiting to send!\n" );
#endif
        priv->stats.tx_dropped++;
        if ( !netif_queue_stopped( dev ) ) {
            DBG_PRINT( "queue not stopped!\n" );
            netif_stop_queue( dev );
        }
        return NETDEV_TX_BUSY;
    }
#endif

#if ETH_ZLEN > 60
    len = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;
#else
    /* Hardware will pad the length to 60. */
    len = skb->len;
#endif

#if 0
    if ( ( rc = AcquireHardware( hw_priv, FALSE, FALSE )) ) {
        return NETDEV_TX_BUSY;
    }
#endif

#if 1
    spin_lock_irq( &priv->lock );
#endif

#ifdef SKB_WAITING
    priv->skb_waiting = skb;
#endif

#ifdef SCATTER_GATHER
    num = skb_shinfo( skb )->nr_frags + 1;
#endif
    if ( HardwareAllocPacket( pHardware, len, num ) >= num ) {
        rc = send_packet( skb, dev );

        if ( !rc ) {

            /* Check whether there are space to hold more socket buffers. */
        }
        else {
            dev_kfree_skb( skb );
        }
    }
    else {

#ifdef DEBUG_TX
        printk( "wait to send\n" );
#endif

        /* Stop the transmit queue until packet is allocated. */
        netif_stop_queue( dev );

#ifndef SKB_WAITING
        trc = NETDEV_TX_BUSY;
#endif
    }

#if 1
    spin_unlock_irq( &priv->lock );
#endif
#if 0
    ReleaseHardware( hw_priv, FALSE );
#endif

    return( trc );
}  /* dev_transmit */

/* -------------------------------------------------------------------------- */

/*
    dev_transmit_timeout

    Description:
        This routine is called when the transmit timer expires.  That indicates
        the hardware is not running correctly because transmit interrupts are
        not triggered to free up resources so that the transmit routine can
        continue sending out packets.  The hardware is reset to correct the
        problem.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (None):
*/

void
ks8692_\
dev_transmit_timeout (
    struct net_device *dev )
{
    static unsigned long last_reset = 0;

    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;

#ifdef DBG
    printk( "transmit timeout:%lu\n", jiffies );
#endif

    last_reset = jiffies;
    if ( hw_priv ) {
        AcquireHardware( hw_priv, FALSE, FALSE );

        HardwareDisableInterrupt( pHardware );
        HardwareDisable( pHardware );

        /* Initialize to invalid value so that link detection is done. */
        pHardware->m_wLinkPartner = 0xFFFF;

        transmit_reset( dev );
        HardwareResetRxPackets( pHardware );
        HardwareResetTxPackets( pHardware );
        InitBuffers( &hw_priv->m_RxBufInfo );
        InitBuffers( &hw_priv->m_TxBufInfo );
        InitReceiveBuffers( hw_priv );

        HardwareReset( pHardware );
        DelayMicrosec( 10 );
        HardwareSetup( pHardware );
        HardwareSetDescriptorBase( pHardware, pHardware->m_TxDescInfo.ulRing,
            pHardware->m_RxDescInfo.ulRing );

        /* Need to program misc. configuration register. */
        HardwareGetLinkSpeed( pHardware );

        if ( pHardware->m_bMulticastListSize )
            HardwareSetGroupAddress( pHardware );
    }

    {
        /* This port device is opened for use. */
        if ( priv->opened ) {

#ifdef SKB_WAITING
            if ( priv->skb_waiting ) {
                priv->stats.tx_dropped++;
                dev_kfree_skb( priv->skb_waiting );
                priv->skb_waiting = NULL;
            }
#endif
        }
    }

    netif_trans_update(dev);
    netif_wake_queue( dev );

    if ( hw_priv ) {
        HardwareEnable( pHardware );

        ReleaseHardware( hw_priv, FALSE );
        HardwareEnableInterrupt( pHardware );
    }
}  /* dev_transmit_timeout */

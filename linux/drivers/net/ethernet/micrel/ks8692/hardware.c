/*---------------------------------------------------------------------------
          Copyright (c) 2006-2008 Micrel, Inc.  All rights reserved.
  ---------------------------------------------------------------------------

  hardware.c - Target independent hardware functions

  Author  Date      Version  Description
  THa     12/10/06  0.0.1    Created file.
  THa     07/17/07           Calculate multicast hash table CRC using only
                             first 5 bytes of data.
  THa     08/29/07           Update for Linux 2.6.21.
  THa     11/30/07           Correct MIB counter reporting.
  THa     07/17/08           Add Access Control List (ACL) support.
  THa     07/28/08           Use UINT instead of ULONG.
                             Workaround transmit failure problem.
  THa     10/31/08           Workaround DMA engine reset hang problem.
  ---------------------------------------------------------------------------
*/


#include <mach/platform.h>
#include "target.h"
#include "hardware.h"

#if 0
#define SET_MAC_ADDR
#endif

/* THa  2008/10/31
   KSZ8692 has a hardware bug that resetting the DMA engine hangs the whole
   system.  The position of the hardware reset code influences whether system
   hangs or not.  As this problem happens only with 166 MHz system bus clock
   and above, one workaround is to set the clock to 125 MHz before resetting
   and then set the clock back to original value.
*/
#if 1
#define AVOID_RESET_HANG
#endif

/* THa  2007/07/10
   Multicast hash table function does not work correctly in KSZ8692!
*/
#ifdef CONFIG_KSZ8692VA
#define MULTICAST_LEN  5

#else
#define MULTICAST_LEN  6
#endif

extern int rx_icmp_csum;
extern int rx_ip_csum;
extern int rx_tcp_csum;
extern int rx_udp_csum;
extern int rx_err;


#ifdef CONFIG_KSZ8692VA
#if 1
UCHAR DEFAULT_MAC_ADDRESS[] = { 0x00, 0x10, 0xA1, 0x86, 0x90, 0x01 };
#else
UCHAR DEFAULT_MAC_ADDRESS[] = { 0x00, 0x10, 0xA1, 0x86, 0x80, 0x01 };
#endif

#else
UCHAR DEFAULT_MAC_ADDRESS[] = { 0x00, 0x10, 0xA1, 0x96, 0x92, 0x01 };
#endif

/* -------------------------------------------------------------------------- */

/*
    HardwareReadPhy

    Description:
        This routine reads data from the PHY register.

    Parameters:
        int port
            Port to read.

        USHORT wPhyReg
            PHY register to read.

        PUSHORT pwData
            Pointer to word to store the read data.

    Return (None):
*/

int HardwareReadPhy (
    int       port,
    USHORT    wPhyReg,
    PUSHORT   pwData )
{
    int phy;
    int status;
    int timeout = STA_TIMEOUT;

    phy = ( wPhyReg << 6 ) | ( port << 1 ) | STA_READ;
    KS_WRITE_REG( KS8692_STA_COMM, phy );
    KS_WRITE_REG( KS8692_STA_CTRL, STA_START );
    do {
        DelayMicrosec( 2 );
        status = KS_READ_REG( KS8692_STA_STATUS ) & STA_STATUS_MASK;
    } while ( status != STA_READ_DONE  &&  --timeout );
    if ( STA_READ_DONE == status )
    {
        *pwData = ( USHORT ) KS_READ_REG( KS8692_STA_DATA0 );
        return TRUE;
    }
    return FALSE;
}  /* HardwareReadPhy */


/*
    HardwareWritePhy

    Description:
        This routine writes data to the PHY register.

    Parameters:
        int port
            Port to write.

        USHORT wPhyReg
            PHY register to write.

        USHORT wData
            Word data to write.

    Return (None):
*/

int HardwareWritePhy (
    int       port,
    USHORT    wPhyReg,
    USHORT    wRegData )
{
    int phy;
    int status;
    int timeout = STA_TIMEOUT;

    phy = ( wPhyReg << 6 ) | ( port << 1 ) | STA_WRITE;
    KS_WRITE_REG( KS8692_STA_DATA0, wRegData );
    KS_WRITE_REG( KS8692_STA_COMM, phy );
    KS_WRITE_REG( KS8692_STA_CTRL, STA_START );
    do {
        DelayMicrosec( 2 );
        status = KS_READ_REG( KS8692_STA_STATUS ) & STA_STATUS_MASK;
    } while ( status != STA_WRITE_DONE  &&  --timeout );
    return FALSE;
}  /* HardwareWritePhy */

/* -------------------------------------------------------------------------- */

#if 0
#define AT93C_CODE    0
#define AT93C_WR_OFF  0x00
#define AT93C_WR_ALL  0x10
#define AT93C_ER_ALL  0x20
#define AT93C_WR_ON   0x30

#define AT93C_WRITE   1
#define AT93C_READ    2
#define AT93C_ERASE   3

#define EEPROM_DELAY  4


static
#ifdef DEF_LINUX
inline
#endif
#ifdef _WIN32
__inline
#endif
void DropGPIO (
    PHARDWARE pHardware,
    UCHAR     gpio )
{
    USHORT data;

    HW_READ_WORD( pHardware, REG_EEPROM_CTRL_OFFSET, &data );
    data &= ~gpio;
    HW_WRITE_WORD( pHardware, REG_EEPROM_CTRL_OFFSET, data );
}  /* DropGPIO */


static
#ifdef DEF_LINUX
inline
#endif
#ifdef _WIN32
__inline
#endif
void RaiseGPIO (
    PHARDWARE pHardware,
    UCHAR     gpio )
{
    USHORT data;

    HW_READ_WORD( pHardware, REG_EEPROM_CTRL_OFFSET, &data );
    data |= gpio;
    HW_WRITE_WORD( pHardware, REG_EEPROM_CTRL_OFFSET, data );
}  /* RaiseGPIO */


static
#ifdef DEF_LINUX
inline
#endif
#ifdef _WIN32
__inline
#endif
UCHAR StateGPIO (
    PHARDWARE pHardware,
    UCHAR     gpio )
{
    USHORT data;

    HW_READ_WORD( pHardware, REG_EEPROM_CTRL_OFFSET, &data );
    return(( UCHAR )( data & gpio ));
}  /* StateGPIO */


#define EEPROM_CLOCK( pHardware )                                           \
{                                                                           \
    RaiseGPIO( pHardware, EEPROM_SERIAL_CLOCK );                            \
    DelayMicrosec( EEPROM_DELAY );                                          \
    DropGPIO( pHardware, EEPROM_SERIAL_CLOCK );                             \
    DelayMicrosec( EEPROM_DELAY );                                          \
}


static
USHORT SPIMRead (
    PHARDWARE pHardware )
{
    int    i;
    USHORT bTemp = 0;

    for ( i = 15; i >= 0; i-- ) {
        RaiseGPIO( pHardware, EEPROM_SERIAL_CLOCK );
        DelayMicrosec( EEPROM_DELAY );

        bTemp |= ( StateGPIO( pHardware, EEPROM_DATA_IN )) ? 1 << i : 0;

        DropGPIO( pHardware, EEPROM_SERIAL_CLOCK );
        DelayMicrosec( EEPROM_DELAY );
    }
    return bTemp;
}  /* SPIMRead */


static
void SPIMWrite (
    PHARDWARE pHardware,
    USHORT    Data )
{
    int i;

    for ( i = 15; i >= 0; i-- ) {
        ( Data & ( 0x01 << i ) ) ? RaiseGPIO( pHardware, EEPROM_DATA_OUT ) :
            DropGPIO( pHardware, EEPROM_DATA_OUT );
        EEPROM_CLOCK( pHardware );
    }
}  /* SPIMWrite */


static
void SPIMReg (
    PHARDWARE pHardware,
    UCHAR     Data,
    UCHAR     Reg )
{
    int i;

    /* Initial start bit */
    RaiseGPIO( pHardware, EEPROM_DATA_OUT );
    EEPROM_CLOCK( pHardware );

    /* AT93C operation */
    for ( i = 1; i >= 0; i-- ) {
        ( Data & ( 0x01 << i ) ) ? RaiseGPIO( pHardware, EEPROM_DATA_OUT ) :
            DropGPIO( pHardware, EEPROM_DATA_OUT );
        EEPROM_CLOCK( pHardware );
    }

    /* Address location */
    for ( i = 5; i >= 0; i-- ) {
        ( Reg & ( 0x01 << i ) ) ? RaiseGPIO( pHardware, EEPROM_DATA_OUT ) :
            DropGPIO( pHardware, EEPROM_DATA_OUT );
        EEPROM_CLOCK( pHardware );
    }
}  /* SPIMReg */


/*
    EepromReadWord

    Description:
        This function reads a word from the AT93C46 EEPROM.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR Reg
            The register offset.

    Return (USHORT):
        The data value.
*/

USHORT EepromReadWord (
    PHARDWARE pHardware,
    UCHAR     Reg )
{
    USHORT Data;

    RaiseGPIO( pHardware, EEPROM_ACCESS_ENABLE | EEPROM_CHIP_SELECT );

    SPIMReg( pHardware, AT93C_READ, Reg );
    Data = SPIMRead( pHardware );

    DropGPIO( pHardware, EEPROM_ACCESS_ENABLE | EEPROM_CHIP_SELECT );

    return( Data );
}  /* EepromReadWord */


/*
    EepromWriteWord

    Description:
        This procedure writes a word to the AT93C46 EEPROM.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR Reg
            The register offset.

        USHORT Data
            The data value.

    Return (None):
*/

void EepromWriteWord (
    PHARDWARE pHardware,
    UCHAR     Reg,
    USHORT    Data )
{
    int timeout;

    RaiseGPIO( pHardware, EEPROM_ACCESS_ENABLE | EEPROM_CHIP_SELECT );

    /* Enable write. */
    SPIMReg( pHardware, AT93C_CODE, AT93C_WR_ON );
    DropGPIO( pHardware, EEPROM_CHIP_SELECT );
    DelayMicrosec( 1 );

    /* Erase the register. */
    RaiseGPIO( pHardware, EEPROM_CHIP_SELECT );
    SPIMReg( pHardware, AT93C_ERASE, Reg );
    DropGPIO( pHardware, EEPROM_CHIP_SELECT );
    DelayMicrosec( 1 );

    /* Check operation complete. */
    RaiseGPIO( pHardware, EEPROM_CHIP_SELECT );
    timeout = 8;
    DelayMillisec( 2 );
    do {
        DelayMillisec( 1 );
    } while ( !StateGPIO( pHardware, EEPROM_DATA_IN )  &&  --timeout );
    DropGPIO( pHardware, EEPROM_CHIP_SELECT );
    DelayMicrosec( 1 );

    /* Write the register. */
    RaiseGPIO( pHardware, EEPROM_CHIP_SELECT );
    SPIMReg( pHardware, AT93C_WRITE, Reg );
    SPIMWrite( pHardware, Data );
    DropGPIO( pHardware, EEPROM_CHIP_SELECT );
    DelayMicrosec( 1 );

    /* Check operation complete. */
    RaiseGPIO( pHardware, EEPROM_CHIP_SELECT );
    timeout = 8;
    DelayMillisec( 2 );
    do {
        DelayMillisec( 1 );
    } while ( !StateGPIO( pHardware, EEPROM_DATA_IN )  &&  --timeout );
    DropGPIO( pHardware, EEPROM_CHIP_SELECT );
    DelayMicrosec( 1 );

    /* Disable write. */
    RaiseGPIO( pHardware, EEPROM_CHIP_SELECT );
    SPIMReg( pHardware, AT93C_CODE, AT93C_WR_OFF );

    DropGPIO( pHardware, EEPROM_ACCESS_ENABLE | EEPROM_CHIP_SELECT );
}  /* EepromWriteWord */
#endif

/* -------------------------------------------------------------------------- */

/*
    HardwareInitialize

    Description:
        This function checks the hardware is correct for this driver and resets
        the hardware for proper initialization.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE if successful; otherwise, FALSE.
*/

BOOLEAN HardwareInitialize
(
    PHARDWARE pHardware )
{
    UINT data;

    data = KS_READ_REG( KS8692_STA_PHY );
    pHardware->m_wPhyAddr = ( data >> pHardware->m_nPhyShift ) &
        STA_PHY0_ADD_MASK;
    HardwareSetupPhy( pHardware );

    /* Initialize to invalid value so that link detection is done. */
    pHardware->m_wLinkPartner = 0xFFFF;

#ifdef CONFIG_KSZ8692VB
    data = KS_READ_REG( KS8692_SPARE_REG );
    data |= IPV6_UDP_FRAG_PASS;
    KS_WRITE_REG( KS8692_SPARE_REG, data );
#endif
    return( TRUE );
}  /* HardwareInitialize */


/*
    HardwareReset

    Description:
        This function resets the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE if successful; otherwise, FALSE.
*/

BOOLEAN HardwareReset
(
    PHARDWARE pHardware )
{
#ifdef AVOID_RESET_HANG
    UINT data;

    data = KS_READ_REG( KS8692_SYSTEM_BUS_CLOCK );
    KS_WRITE_REG( KS8692_SYSTEM_BUS_CLOCK,
        ( data & ~SYSTEM_BUS_CLOCK_MASK ) | SYSTEM_BUS_CLOCK_125 );
#endif
    HW_WRITE_DWORD( pHardware, REG_DMA_TX_CTRL, DMA_TX_CTRL_RESET );

#ifdef AVOID_RESET_HANG
    KS_WRITE_REG( KS8692_SYSTEM_BUS_CLOCK, data );
#endif

    /* MAC address always got erased after engine reset. */
    HardwareSetAddress( pHardware );

#ifdef CONFIG_KSZ8692VB
    {
        int entry;

        if ( pHardware->m_fBroadcast )
            HardwareSetBroadcastTraffic( pHardware, TRUE,
                pHardware->m_bBroadcastCounter );
        if ( pHardware->m_fDiffServ )
            HardwareSetDiffServPriority( pHardware, TRUE,
                pHardware->m_dwDiffServ0, pHardware->m_dwDiffServ1 );
        if ( pHardware->m_f802_1p )
            HardwareSet802_1P_Priority( pHardware, TRUE,
                pHardware->m_b802_1p_mapping );
        if ( pHardware->m_fBlock )
            HardwareSet_ACL_Block( pHardware, TRUE );
        for ( entry = 0; entry < TOTAL_ACL_NUM; entry++ ) {
            PTAclInfo acl = &pHardware->AclInfo[ entry ];

            acl->changed = 1;
            if ( acl->enable )
                HardwareSetupACL( pHardware, acl, acl->mode, acl->mode );
        }
    }
#endif

#if 0
{
    UCHAR wakeup_mask[ 4 ];

    wakeup_mask[ 0 ] = 0x3F;
    HardwareSetWolFrame( pHardware, 0, 1, wakeup_mask, 6,
        pHardware->m_bOverrideAddress );
}
#endif

#ifdef DEBUG_COUNTER
    pHardware->m_nGood[ COUNT_GOOD_CMD_RESET ]+=1;
#endif
    return TRUE;
}  /* HardwareReset */


/*
    HardwareSetup

    Description:
        This routine setup the hardware for proper operation.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareSetup
(
    PHARDWARE pHardware )
{
    /* Setup Transmit Control */
    pHardware->m_dwTransmitConfig = ( DMA_TX_CTRL_PAD_ENABLE | DMA_TX_CTRL_CRC_ENABLE |
                                     (PBL_DEFAULT << 24) | DMA_TX_CTRL_ENABLE );
#if FLOWCONTROL_DEFAULT
    pHardware->m_dwTransmitConfig |= DMA_TX_CTRL_FLOW_ENABLE;
#else
    pHardware->m_dwTransmitConfig &= ~DMA_TX_CTRL_FLOW_ENABLE;
#endif

#ifndef DEF_LINUX
#if TXCHECKSUM_DEFAULT
    /* Hardware cannot handle UDP packet in IP fragments. */
    pHardware->m_dwTransmitConfig |= (DMA_TX_CTRL_CSUM_TCP | DMA_TX_CTRL_CSUM_IP);
#else
    pHardware->m_dwTransmitConfig &= ~(DMA_TX_CTRL_CSUM_UDP | DMA_TX_CTRL_CSUM_TCP | DMA_TX_CTRL_CSUM_IP);
#endif /* TXCHECKSUM_DEFAULT */
#endif

#ifdef VERIFY_LOOPBACK
    pHardware->m_dwTransmitConfig |= DMA_TX_CTRL_LOOPBACK;
#endif

#ifdef TEST_RX_LOOPBACK
    pHardware->m_dwTransmitConfig &=
        ~( DMA_TX_CTRL_PAD_ENABLE | DMA_TX_CTRL_CRC_ENABLE );
#endif

#ifdef VERIFY_IP_CSUM_GEN_BUG
    pHardware->m_dwTransmitConfig |= DMA_TX_CTRL_CSUM_IP;
#endif

    /* Setup Receive Control */

    pHardware->m_dwReceiveConfig = ( DMA_RX_CTRL_BROADCAST | DMA_RX_CTRL_UNICAST |
                                    (PBL_DEFAULT << 24) | DMA_RX_CTRL_ENABLE );

#if FLOWCONTROL_DEFAULT
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_FLOW_ENABLE;
#else
    pHardware->m_dwReceiveConfig &= ~DMA_RX_CTRL_FLOW_ENABLE;
#endif

#if RXCHECKSUM_DEFAULT
#ifndef CONFIG_PEGASUS
    /* Hardware cannot handle UDP packet in IP fragments. */
    pHardware->m_dwReceiveConfig |= (DMA_RX_CTRL_CSUM_TCP | DMA_RX_CTRL_CSUM_IP);
#endif

#ifdef CONFIG_KSZ8692VB
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_CSUM_TCP;
#endif

#else
    pHardware->m_dwReceiveConfig &= ~(DMA_RX_CTRL_CSUM_UDP | DMA_RX_CTRL_CSUM_TCP | DMA_RX_CTRL_CSUM_IP);
#endif

/* Only work for IP packets! */
#ifdef ALIGN_IP_HEADER
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_IP_ALIGN;
#endif

#if 1
    if ( rx_ip_csum )
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_CSUM_IP;
    if ( rx_tcp_csum )
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_CSUM_TCP;
    if ( rx_udp_csum )
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_CSUM_UDP;

#ifdef CONFIG_KSZ8692VB
    if ( rx_icmp_csum )
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_CSUM_ICMP;
#endif
#endif

#ifdef CONFIG_PEGASUS
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_MULTICAST;
#endif

/* THa  2008/10/02
   Fixed in next revision.
*/
#if 0
/* THa  2008/06/06
   Multicast hash table function does not work at all in KSZ8692 rev. B!
*/
#if defined( CONFIG_KSZ8692VB )  &&  !defined( VERIFY_MULTICAST )
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_ALL_MULTICAST;
#endif
#endif

    if ( pHardware->m_bAllMulticast )
        pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_ALL_MULTICAST;
    if ( pHardware->m_bPromiscuous )
        pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_PROMISCUOUS;

#if defined( TEST_RX_LOOPBACK ) || defined( DEBUG_RX_ALL )
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_PROMISCUOUS;
#endif

#if defined( CHECK_RCV_ERRORS ) || defined( RCV_HUGE_FRAME ) || defined( DEBUG_RX_ALL )
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_ERROR;
#endif

#if defined( RCV_HUGE_FRAME )
#if defined( CONFIG_KSZ8692VB )
    /* WAN port cannot handle receiving more than 2047-byte frames. */
    if ( 0 == pHardware->m_nPhyShift )
#else
    /* LAN port cannot handle receiving more than 2047-byte frames. */
    if ( 0 != pHardware->m_nPhyShift )
#endif
        pHardware->m_dwReceiveConfig &= ~DMA_RX_CTRL_ERROR;
#endif

#if 1
    if ( rx_err )
    pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_ERROR;
#endif

#ifdef DBG
    DBG_PRINT( "rx ctrl: %08x"NEWLINE, pHardware->m_dwReceiveConfig );
    DBG_PRINT( "tx ctrl: %08x"NEWLINE, pHardware->m_dwTransmitConfig );
#endif
}  /* HardwareSetup */


/*
    HardwareSetupInterrupt

    Description:
        This routine setup the interrupt mask for proper operation.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareSetupInterrupt
(
    PHARDWARE pHardware )
{
    pHardware->m_ulInterruptMask = INT_RX | INT_TX | INT_RX_OVERRUN;
}  /* HardwareSetupInterrupt */


/*
    HardwareClearCounters

    Description:
        This routine resets all hardware counters to zero.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareClearCounters (
    PHARDWARE pHardware )
{
    memset(( void* ) pHardware->m_cnCounter, 0,
        ( sizeof( ULONGLONG ) * OID_COUNTER_LAST ) );

    HardwareInitCounters( pHardware );
}  /* HardwareClearCounters */

/* -------------------------------------------------------------------------- */

#ifdef DBG
void CheckDescriptors (
    PTDescInfo pInfo )
{
    int    i;
    PTDesc pDesc = pInfo->pRing;

    DBG_PRINT( "  %p: %d, %d, %d"NEWLINE, pInfo->phwRing, pInfo->cnAvail,
        pInfo->iLast, pInfo->iNext );
    for ( i = 0; i < pInfo->cnAlloc; i++ )
    {
        DBG_PRINT( "%08x %08x %08x %08x; %08x %08x"NEWLINE,
            pDesc->phw->Control.ulData,
            pDesc->phw->BufSize.ulData,
            pDesc->phw->ulBufAddr,
            pDesc->phw->ulNextPtr,
            pDesc->sw.Control.ulData,
            pDesc->sw.BufSize.ulData );
        pDesc++;
    }
}  /* CheckDescriptors */
#endif


void CheckDescriptorNum (
    PTDescInfo pInfo )
{
    int cnAlloc = pInfo->cnAlloc;
    int nShift;

    nShift = 0;
    while ( !( cnAlloc & 1 ) )
    {
        nShift++;
        cnAlloc >>= 1;
    }
    if ( cnAlloc != 1  ||  nShift < 2 )
    {
        DBG_PRINT( "Hardware descriptor numbers not right!"NEWLINE );
        while ( cnAlloc )
        {
            nShift++;
            cnAlloc >>= 1;
        }
        if ( nShift < 2 )
        {
            nShift = 2;
        }
        cnAlloc = 1 << nShift;
        pInfo->cnAlloc = cnAlloc;
    }
    pInfo->iMax = pInfo->cnAlloc - 1;
}  /* CheckDescriptorNum */


void HardwareInitDescriptors (
    PTDescInfo pDescInfo,
    int        fTransmit )
{
    int       i;
    UINT32    ulPhysical = pDescInfo->ulRing;
    PTHw_Desc pDesc = pDescInfo->phwRing;
    PTDesc    pCurrent = pDescInfo->pRing;
    PTDesc    pPrevious = NULL;

#ifdef CHECK_OVERRUN
    int       check = ( pDescInfo->cnAlloc * 15 ) / 16;
#endif

    for ( i = 0; i < pDescInfo->cnAlloc; i++ )
    {
        pCurrent->phw = pDesc++;
        ulPhysical += pDescInfo->nSize;
#if 0
        if ( fTransmit )
        {
#if TXCHECKSUM_DEFAULT
            /* Hardware cannot handle UDP packet in IP fragments. */
            pCurrent->sw.BufSize.ulData =
                ( DESC_TX_CSUM_GEN_TCP | DESC_TX_CSUM_GEN_IP );
#endif
        }
#endif
        pPrevious = pCurrent++;
        pPrevious->phw->ulNextPtr = CPU_TO_LE32( ulPhysical );

#ifdef CHECK_OVERRUN
        pPrevious->pCheck = &pDescInfo->phwRing[(( i + check ) &
            pDescInfo->iMax )];
#endif
    }
    pPrevious->phw->ulNextPtr = CPU_TO_LE32( pDescInfo->ulRing );
    pPrevious->sw.BufSize.rx.fEndOfRing = TRUE;
    pPrevious->phw->BufSize.ulData =
        CPU_TO_LE32( pPrevious->sw.BufSize.ulData );

    pDescInfo->cnAvail = pDescInfo->cnAlloc;
    pDescInfo->iLast = pDescInfo->iNext = 0;

    pDescInfo->pCurrent = pDescInfo->pRing;
}  /* HardwareInitDescriptors */


/*
 * HardwareSetDescriptorBase
 *	This function sets MAC address to given type (WAN, LAN or HPHA)
 *
 * Argument(s)
 *  pHardware        Pointer to hardware instance.
 *  pTxDescBaseAddr	 pointer to base address of Tx descripotr.
 *  pRxDescBaseAddr	 pointer to base address of Rx descripotr.
 *
 * Return(s)
 *	NONE.
 */
void HardwareSetDescriptorBase
(
    PHARDWARE  pHardware,
    UINT32     TxDescBaseAddr,
    UINT32     RxDescBaseAddr
)
{
    /* Set base address of Tx/Rx descriptors */
    HW_WRITE_DWORD( pHardware, REG_DMA_TX_ADDR, TxDescBaseAddr );
    HW_WRITE_DWORD( pHardware, REG_DMA_RX_ADDR, RxDescBaseAddr );
}  /* HardwareSetDescriptorBase */


void HardwareResetPackets (
    PTDescInfo pInfo )
{
    pInfo->pCurrent = pInfo->pRing;
    pInfo->cnAvail = pInfo->cnAlloc;
    pInfo->iLast = pInfo->iNext = 0;
}  /* HardwareResetPackets */

/* -------------------------------------------------------------------------- */

/*
    Link processing primary routines
*/

#ifndef INLINE
/*
    HardwareAcknowledgeLink

    Description:
        This routine acknowledges the link change interrupt.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareAcknowledgeLink
(
    PHARDWARE pHardware )
{
}  /* HardwareAcknowledgeLink */
#endif


/*
    HardwareCheckLink

    Description:
        This routine checks the link status.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareCheckLink
(
    PHARDWARE pHardware )
{
    HardwareGetLinkSpeed( pHardware );
}  /* HardwareCheckLink */

/* -------------------------------------------------------------------------- */

/*
    Receive processing primary routines
*/

#ifndef INLINE
/*
    HardwareAcknowledgeReceive

    Description:
        This routine acknowledges the receive interrupt.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareAcknowledgeReceive
(
    PHARDWARE pHardware )
{
    KS_WRITE_REG( KS_INT_STATUS, INT_RX << pHardware->m_nShift );
}  /* HardwareAcknowledgeReceive */
#endif


/*
    HardwareStartReceive

    Description:
        This routine starts the receive function of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareStartReceive
(
    PHARDWARE pHardware )
{
    HW_WRITE_DWORD( pHardware, REG_DMA_RX_CTRL,
        pHardware->m_dwReceiveConfig );

#if 0
    /* Notify when the receive stops. */
    pHardware->m_ulInterruptMask |= INT_RX_STOPPED;
#endif
    HW_WRITE_DWORD( pHardware, REG_DMA_RX_START, DMA_START );
    HardwareAcknowledgeInterrupt( pHardware, INT_RX_STOPPED );
    pHardware->m_bReceiveStop++;

    /* Variable overflows. */
    if ( 0 == pHardware->m_bReceiveStop )
        pHardware->m_bReceiveStop = 2;
}  /* HardwareStartReceive */


/*
    HardwareStopReceive

    Description:
        This routine stops the receive function of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareStopReceive
(
    PHARDWARE pHardware )
{
#if 0
    pHardware->m_bReceiveStop = 0;
    HardwareTurnOffInterrupt( pHardware, INT_RX_STOPPED );
    HardwareDisableInterruptBit( pHardware, INT_RX_STOPPED );
#endif
    HW_WRITE_DWORD( pHardware, REG_DMA_RX_CTRL,
                    (pHardware->m_dwReceiveConfig & ~DMA_RX_CTRL_ENABLE ) );
}  /* HardwareStopReceive */

/* -------------------------------------------------------------------------- */

/*
    Transmit processing primary routines
*/

#ifndef INLINE
/*
    HardwareAcknowledgeTransmit

    Description:
        This routine acknowledges the trasnmit interrupt.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareAcknowledgeTransmit
(
    PHARDWARE pHardware )
{
    KS_WRITE_REG( KS_INT_STATUS, INT_TX << pHardware->m_nShift );
}  /* HardwareAcknowledgeTransmit */
#endif


/*
    HardwareStartTransmit

    Description:
        This routine starts the transmit function of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareStartTransmit
(
    PHARDWARE pHardware )
{
    HW_WRITE_DWORD( pHardware, REG_DMA_TX_CTRL,
        pHardware->m_dwTransmitConfig );

#ifdef DEVELOP
{
    PTDescInfo pDescInfo = &pHardware->m_TxDescInfo;
    UINT       ulIntStatus;
    int        i;
    int        timeout;
    PTDesc     pCurrent = pDescInfo->pRing;

    /* Find out the current descriptor pointed by hardware. */
    for ( i = 0; i < pDescInfo->cnAlloc - 1; i++ )
    {
        /* This descriptor will be skipped. */
        pCurrent->phw->BufSize.ulData = 0;
        pCurrent->sw.Control.tx.fHWOwned = TRUE;
        pCurrent->phw->Control.ulData =
            CPU_TO_LE32( pCurrent->sw.Control.ulData );
        pCurrent++;
    }

    /* Stop at the last descriptor. */
    pCurrent->sw.Control.tx.fHWOwned = FALSE;
    pCurrent->phw->Control.ulData =
        CPU_TO_LE32( pCurrent->sw.Control.ulData );

    /* Let the hardware goes through the descriptors. */
    HW_WRITE_DWORD( pHardware, REG_DMA_TX_START, DMA_START );

    timeout = 10;
    do {
        DelayMillisec( 10 );
        HardwareReadInterrupt( pHardware, &ulIntStatus );
    } while ( !( ulIntStatus & INT_TX_EMPTY )  &&  timeout-- );
    if ( !( ulIntStatus & INT_TX_EMPTY ) ) {
        DBG_PRINT( "Tx not working!  Reset the hardware!"NEWLINE );
    }

    /* Acknowledge the interrupt. */
    HardwareAcknowledgeEmpty( pHardware );

    /* Last descriptor */
    pCurrent->sw.BufSize.tx.wBufSize = 0;
    pCurrent->phw->BufSize.ulData =
        CPU_TO_LE32( pCurrent->sw.BufSize.ulData );
    pCurrent->sw.Control.tx.fHWOwned = TRUE;
    pCurrent->phw->Control.ulData = CPU_TO_LE32( pCurrent->sw.Control.ulData );

    /* First descriptor */
    pCurrent = pDescInfo->pRing;
    pCurrent->sw.Control.tx.fHWOwned = FALSE;
    pCurrent->phw->Control.ulData =
        CPU_TO_LE32( pCurrent->sw.Control.ulData );

    /* Let the hardware goes to the first descriptor. */
    HW_WRITE_DWORD( pHardware, REG_DMA_TX_START, DMA_START );

    timeout = 10;
    do {
        DelayMillisec( 10 );
        HardwareReadInterrupt( pHardware, &ulIntStatus );
    } while ( !( ulIntStatus & INT_TX_EMPTY )  &&  timeout-- );
    if ( !( ulIntStatus & INT_TX_EMPTY ) ) {
        DBG_PRINT( "Tx not working!  Reset the hardware!"NEWLINE );
    }

    /* Acknowledge the interrupt. */
    HardwareAcknowledgeEmpty( pHardware );

    /* Reset all the descriptors. */
    pCurrent = pDescInfo->pRing;
    for ( i = 0; i < pDescInfo->cnAlloc; i++ )
    {
        pCurrent->sw.Control.tx.fHWOwned = FALSE;
        pCurrent->phw->Control.ulData =
            CPU_TO_LE32( pCurrent->sw.Control.ulData );
        pCurrent++;
    }
    pDescInfo->pCurrent = pDescInfo->pRing;
}
#endif
}  /* HardwareStartTransmit */


/*
    HardwareStopTransmit

    Description:
        This routine stops the transmit function of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareStopTransmit
(
    PHARDWARE pHardware )
{
    HW_WRITE_DWORD( pHardware, REG_DMA_TX_CTRL,
                   (pHardware->m_dwTransmitConfig & ~DMA_TX_CTRL_ENABLE ) );
}  /* HardwareStopTransmit */

/* -------------------------------------------------------------------------- */

/*
    Interrupt processing primary routines
*/

#ifndef INLINE
/*
    HardwareAcknowledgeInterrupt

    Description:
        This routine acknowledges the specified interrupts.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UINT32 ulInterrupt
            The interrupt masks to be acknowledged.

    Return (None):
*/

void HardwareAcknowledgeInterrupt
(
    PHARDWARE pHardware,
    UINT32    ulInterrupt )
{
    KS_WRITE_REG( KS_INT_STATUS, ulInterrupt << pHardware->m_nShift );
}  /* HardwareAcknowledgeInterrupt */


/*
    HardwareDisableInterrupt

    Description:
        This routine disables the interrupts of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareDisableInterrupt
(
    PHARDWARE pHardware )
{
    UINT data;

    pHardware->m_InterruptBlocked = pHardware->m_ulInterruptMask;

    data = KS_READ_REG( KS_INT_ENABLE );
    data &= ~( INT_CHECK << pHardware->m_nShift );
    KS_WRITE_REG( KS_INT_ENABLE, data );
    data = KS_READ_REG( KS_INT_ENABLE );
    pHardware->m_ulInterruptSet = ( data >> pHardware->m_nShift ) &
        INT_CHECK;
}  /* HardwareDisableInterrupt */


/*
    HardwareEnableInterrupt

    Description:
        This routine enables the interrupts of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareEnableInterrupt
(
    PHARDWARE pHardware )
{
    pHardware->m_InterruptBlocked = 0;

    HardwareSetInterrupt( pHardware, pHardware->m_ulInterruptMask );
}  /* HardwareEnableInterrupt */


/*
    HardwareReadInterrupt

    Description:
        This routine reads the current interrupts of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UINT* pStatus
            Buffer to store the interrupt mask.

    Return (None):
*/

void HardwareReadInterrupt
(
    PHARDWARE pHardware,
    UINT*     pStatus )
{
    *pStatus = ( KS_READ_REG( KS_INT_STATUS ) >>
        pHardware->m_nShift ) & INT_CHECK;
}  /* HardwareReadInterrupt */


/*
    HardwareRestoreInterrupt

    Description:
        This routine restores the interrupts which are blocked before.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UINT Interrupt
            The interrupt mask that is blocked.

    Return (None):
*/

void HardwareRestoreInterrupt
(
    PHARDWARE pHardware,
    UINT      Interrupt )
{
    /* Need to restore interrupts. */
    if ( Interrupt )
    {
        pHardware->m_InterruptBlocked = 0;

        HardwareSetInterrupt( pHardware, pHardware->m_ulInterruptMask );
    }
}  /* HardwareRestoreInterrupt */


/*
    HardwareSetInterrupt

    Description:
        This routine enables the specified interrupts of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UINT ulInterrupt
            The interrupt mask to enable.

    Return (None):
*/

void HardwareSetInterrupt
(
    PHARDWARE pHardware,
    UINT      ulInterrupt )
{
    UINT data;

    pHardware->m_ulInterruptSet = ulInterrupt;
    data = KS_READ_REG( KS_INT_ENABLE );
    data &= ~( INT_CHECK << pHardware->m_nShift );
    data |= pHardware->m_ulInterruptMask << pHardware->m_nShift;
    KS_WRITE_REG( KS_INT_ENABLE, data );
}  /* HardwareSetInterrupt */
#endif


/*
    HardwareBlockInterrupt

    Description:
        This function blocks all interrupts of the hardware and returns the
        current interrupt enable mask so that interrupts can be restored later.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (USHORT):
    Return (UINT):
        The current interrupt enable mask.
*/

UINT HardwareBlockInterrupt
(
    PHARDWARE pHardware )
{
    UINT Interrupt = 0;

    if ( !pHardware->m_InterruptBlocked )
    {
        HardwareDisableInterruptSync( pHardware );
        Interrupt = pHardware->m_InterruptBlocked;
    }
    return( Interrupt );
}  /* HardwareBlockInterrupt */

/* -------------------------------------------------------------------------- */

/*
    Other interrupt primary routines
*/

#ifndef INLINE
/*
    HardwareTurnOffInterrupt

    Description:
        This routine turns off the specified interrupts in the interrupt mask
        so that those interrupts will not be enabled.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UINT32 ulInterruptBit
            The interrupt bits to be off.

    Return (None):
*/

void HardwareTurnOffInterrupt
(
    PHARDWARE pHardware,
    UINT32    ulInterruptBit )
{
    pHardware->m_ulInterruptMask &= ~ulInterruptBit;

    /* Interrupts are not blocked. */
    if ( !pHardware->m_InterruptBlocked )
    {
        HardwareSetInterrupt( pHardware, pHardware->m_ulInterruptMask );
    }
}  /* HardwareTurnOffInterrupt */
#endif


/*
    HardwareTurnOnInterrupt

    Description:
        This routine turns on the specified interrupts in the interrupt mask so
        that those interrupts will be enabled.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UINT32 ulInterruptBit
            The interrupt bits to be on.

    Return (None):
*/

void HardwareTurnOnInterrupt
(
    PHARDWARE pHardware,
    UINT32    ulInterruptBit,
    UINT32*   pulInterruptMask )
{
    pHardware->m_ulInterruptMask |= ulInterruptBit;

    /* An interrupt mask is previously retrieved to be set later. */
    if ( pulInterruptMask )
    {
        *pulInterruptMask |= ulInterruptBit;
    }
    else
    {
        if ( !pHardware->m_InterruptBlocked )
        {
            HardwareSetInterrupt( pHardware, pHardware->m_ulInterruptMask );
        }
    }
}  /* HardwareTurnOnInterrupt */

/* -------------------------------------------------------------------------- */

/*
    Register saving routines
*/

/* -------------------------------------------------------------------------- */

/*
    HardwareDisable

    Description:
        This routine disables the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareDisable
(
    PHARDWARE pHardware )
{
    HardwareStopReceive( pHardware );
    HardwareStopTransmit( pHardware );
    pHardware->m_bEnabled = FALSE;
}  /* HardwareDisable */


/*
    HardwareEnable

    Description:
        This routine enables the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareEnable
(
    PHARDWARE pHardware )
{
    HardwareStartTransmit( pHardware );
    HardwareStartReceive( pHardware );
    pHardware->m_bEnabled = TRUE;
}  /* HardwareEnable */

/* -------------------------------------------------------------------------- */

/*
    Receive processing routines
*/

#ifndef INLINE
/*
    HardwareFreeReceivedPacket

    Description:
        This routine frees the received packet and puts it back in the queue
        for use.
        This routine is used for debug purpose.  The inline version should be
        used.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

        int* piNext
            Pointer to descriptor index of received packet.

    Return (None):
*/

void HardwareFreeReceivedPacket (
    PTDescInfo pInfo,
    int*       piNext )
{
    ASSERT( pInfo->cnAvail > 0 );
    ( *piNext )++;
    *piNext &= pInfo->iMax;
    pInfo->cnAvail--;

#ifdef DEBUG_RX_DESC
    DBG_PRINT( "free rx: %d"NEWLINE, pInfo->cnAvail );
#endif
}  /* HardwareFreeReceivedPacket */


/*
    HardwareGetReceivedPacket

    Description:
        This function retrieves the received packet.
        This routine is used for debug purpose.  The inline version should be
        used.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

        int iNext
            Descriptor index of next packet.

        UINT32* pulData
            Pointer to descriptor status.

    Return (PTDesc):
        Pointer to descriptor if successful; otherwise, NULL.
*/

PTDesc HardwareGetReceivedPacket (
    PTDescInfo pInfo,
    int        iNext,
    UINT32*    pulData )
{
    PTDesc pDesc = &pInfo->pRing[ iNext ];

    *pulData = LE32_TO_CPU( pDesc->phw->Control.ulData );

#ifdef DEBUG_RX_DESC_CHECK
    DBG_PRINT( "rx? %d: %x=%08X"NEWLINE, pInfo->cnAvail,
        iNext, ( int ) *pulData );
#endif
    if ( !( *pulData & DESC_HW_OWNED ) )
    {
        pInfo->pCurrent = pDesc;
        pInfo->cnAvail++;

#ifdef DEBUG_RX_DESC
        DBG_PRINT( "got rx: %d: %p"NEWLINE, pInfo->cnAvail,
            pInfo->pCurrent );
#endif
        return( pInfo->pCurrent );
    }
    return NULL;
}  /* HardwareGetReceivedPacket */


/*
    HardwareGetRxPacket

    Description:
        This function retrieves the packet for receive setup.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

    Return (PTDesc):
        Pointer to descriptor if successful; otherwise, NULL.
*/

PTDesc HardwareGetRxPacket (
    PTDescInfo pInfo )
{
    pInfo->pCurrent = &pInfo->pRing[ pInfo->iLast ];
    pInfo->iLast++;
    pInfo->iLast &= pInfo->iMax;
    pInfo->cnAvail--;

    pInfo->pCurrent->sw.BufSize.ulData &= ~DESC_RX_MASK;

    return( pInfo->pCurrent );
}  /* HardwareGetRxPacket */
#endif


/*
    HardwareReceive

    Description:
        This routine handles the receive processing.  The packet is read into
        the m_Lookahead buffer and its length is indicated in m_nPacketLen.

        No matter switch is Direct mode or Lookup mode, when
        m_bPortRX is 1 : Receive packet from Port1,
        m_bPortRX is 2 : Receive packet from Port2.


    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareReceive
(
    PHARDWARE pHardware )
{
    TDescStat status;
    PTDesc    pDesc = pHardware->m_RxDescInfo.pCurrent;

    /* Assume error. */
    pHardware->m_nPacketLen = 0;

    status.ulData = pDesc->sw.Control.ulData;

    /* Status valid only when last descriptor bit is set. */
    if ( status.rx.fLastDesc  &&  status.rx.fFirstDesc )
    {

#if defined( CHECK_RCV_ERRORS ) || defined( RCV_HUGE_FRAME )
        /*
         *  Receive without error.  With receive errors disabled, packets with
         *  receive errors will be dropped, so no need to check the error bit.
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
            pHardware->m_nPacketLen = status.rx.wFrameLen - 4;
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
#endif
    }
}  /* HardwareReceive */

/* -------------------------------------------------------------------------- */

/*
    Transmit processing routines
*/


#define MIN_TX_BUFFER_SIZE    128
#define TX_PACKET_ARRAY_SIZE  8
#define RX_PACKET_ARRAY_SIZE  64


/*
    HardwareAllocPacket

    Description:
        This function allocates a packet for transmission.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int length
            The length of the packet.

        int physical
            Number of descriptors required in PCI version.

    Return (int):
        0 if not successful; 1 for buffer copy; otherwise, can use descriptors.
*/

int HardwareAllocPacket
(
    PHARDWARE pHardware,
    int       length,
    int       physical )
{

    /* Always leave one descriptor free. */
    if ( pHardware->m_TxDescInfo.cnAvail <= 1 )
    {
        /* Update transmit statistics. */
        pHardware->m_cnCounter[ OID_COUNTER_XMIT_ALLOC_FAIL ]+=1;
        return( 0 );
    }

    /* Allocate a descriptor for transmission and mark it current. */
    GetTxPacket( &pHardware->m_TxDescInfo, pHardware->m_TxDescInfo.pCurrent );
    pHardware->m_TxDescInfo.pCurrent->sw.BufSize.tx.fFirstSeg = TRUE;

#ifdef SKIP_TX_INT
    ++pHardware->m_TxIntCnt;
    pHardware->m_TxSize += length;

    /* Cannot hold on too much data. */
    if ( pHardware->m_TxSize >= MAX_TX_HELD_SIZE )
    {
        pHardware->m_TxIntCnt = pHardware->m_TxIntMask + 1;
    }
#endif

#ifdef _WIN32
    if ( length < MIN_TX_BUFFER_SIZE  ||
            physical > TX_PACKET_ARRAY_SIZE  ||
            physical + 32 >= pHardware->m_TxDescInfo.cnAvail )
#else
    if ( physical >= pHardware->m_TxDescInfo.cnAvail )
#endif
    {
        return( 1 );
    }

    return( physical + 1 );
}  /* HardwareAllocPacket */



#ifndef INLINE
/*
    HardwareFreeTransmittedPacket

    Description:
        This routine frees the transmitted packet and puts it back in the queue
        for use.
        This routine is used for debug purpose.  The inline version should be
        used.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

        int* piLast
            Pointer to descriptor index of transmitted packet.

    Return (None):
*/

void HardwareFreeTransmittedPacket (
    PTDescInfo pInfo,
    int*       piLast )
{
    ASSERT( pInfo->cnAvail < pInfo->cnAlloc );
    ( *piLast )++;
    *piLast &= pInfo->iMax;
    pInfo->cnAvail++;

#ifdef DEBUG_TX_DESC
    DBG_PRINT( "free tx: %d"NEWLINE, pInfo->cnAvail );
#endif
}  /* HardwareFreeTransmittedPacket */


/*
    HardwareFreeTxPacket

    Description:
        This routine frees the allocated packet for transmission and puts it
        back in queue for use.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

    Return (None):
*/

void HardwareFreeTxPacket (
    PTDescInfo pInfo )
{
    pInfo->iNext--;
    pInfo->iNext &= pInfo->iMax;
    pInfo->cnAvail++;

#ifdef DEBUG_TX_DESC
    DBG_PRINT( "putback: %d"NEWLINE, pInfo->cnAvail );
#endif
}  /* HardwareFreeTxPacket */


/*
    HardwareGetTransmittedPacket

    Description:
        This function retrieves the transmitted packet.
        This routine is used for debug purpose.  The inline version should be
        used.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

        int iLast
            Descriptor index of transmitted packet.

        UINT32* pulData
            Pointer to descriptor status.

    Return (PTDesc):
        Pointer to descriptor if successful; otherwise, NULL.
*/

PTDesc HardwareGetTransmittedPacket (
    PTDescInfo pInfo,
    int        iLast,
    UINT32*    pulData )
{
    PTDesc pDesc = &pInfo->pRing[ iLast ];

    *pulData = LE32_TO_CPU( pDesc->phw->Control.ulData );
    if ( !( *pulData & DESC_HW_OWNED ) )
    {

#ifdef DEBUG_TX_DESC
        DBG_PRINT( "got tx: %x"NEWLINE, iLast );
#endif
    }
    return( pDesc );
}  /* HardwareGetTransmittedPacket */


/*
    HardwareGetTxPacket

    Description:
        This function retrieves the packet for transmission.
        This routine is used for debug purpose.  The inline version should be
        used.

    Parameters:
        PDescInfo pInfo
            Pointer to descriptor information structure.

    Return (PTDesc):
        Pointer to descriptor if successful; otherwise, NULL.
*/

PTDesc HardwareGetTxPacket (
    PTDescInfo pInfo )
{
    ASSERT( pInfo->cnAvail > 0 );
    pInfo->pCurrent = &pInfo->pRing[ pInfo->iNext ];
    pInfo->iNext++;
    pInfo->iNext &= pInfo->iMax;
    pInfo->cnAvail--;

    pInfo->pCurrent->sw.BufSize.ulData &= ~DESC_TX_MASK;

#ifdef DEBUG_TX_DESC
    DBG_PRINT( "get tx: %d: %p"NEWLINE, pInfo->cnAvail,
        pInfo->pCurrent );
#endif

    return( pInfo->pCurrent );
}  /* HardwareGetTxPacket */
#endif


/*
    HardwareSendPacket

    Description:
        This function transmits the packet marked by m_bTransmitPacket.
        This function marks the packet for transmission in PCI version.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE if successful; otherwise, FALSE.
*/

BOOLEAN HardwareSendPacket
(
    PHARDWARE pHardware )
{
    PTDesc pCurrent = pHardware->m_TxDescInfo.pCurrent;

    pCurrent->sw.BufSize.tx.fLastSeg = TRUE;

#ifdef SKIP_TX_INT
    if ( pHardware->m_TxIntCnt > pHardware->m_TxIntMask )
    {
        pCurrent->sw.BufSize.tx.fInterrupt = TRUE;
        pHardware->m_TxIntCnt = 0;
        pHardware->m_TxSize = 0;
    }

#else
    pCurrent->sw.BufSize.tx.fInterrupt = TRUE;
#endif

    ReleasePacket( pCurrent );

#ifdef DEBUG_TX
    DBG_PRINT( ":%08x"NEWLINE, ( pCurrent )->phw->BufSize.ulData );
#endif

    HW_WRITE_DWORD( pHardware, REG_DMA_TX_START, 0 );
    return( TRUE );
}  /* HardwareSendPacket */


#ifndef INLINE
/*
    HardwareSetTransmitBuffer

    Description:
        This routine sets the transmit buffer address.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UINT32 ulBufAddr
            Buffer address.

    Return (None):
*/

void HardwareSetTransmitBuffer (
    PHARDWARE pHardware,
    UINT32    ulBufAddr )
{
    pHardware->m_TxDescInfo.pCurrent->phw->ulBufAddr = CPU_TO_LE32( ulBufAddr );
}  /* HardwareSetTransmitBuffer */
#endif

/* -------------------------------------------------------------------------- */

/*
    HardwareSetAddress

    Description:
        This routine programs the MAC address of the hardware when the address
        is overrided.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareSetAddress
(
    PHARDWARE pHardware )
{
    UCHAR i;

    for ( i = 0; i < MAC_ADDRESS_LENGTH; i++ )
    {
        HW_WRITE_BYTE( pHardware, ( UINT )( REG_DMA_MAC_ADDR_LO + i ),
            ( UCHAR ) pHardware->m_bOverrideAddress[ MAC_ADDR_ORDER( i )]);
    }

#ifdef DBG
    DBG_PRINT( "set addr: " );
    PrintMacAddress( pHardware->m_bOverrideAddress );
    DBG_PRINT( NEWLINE );
#endif
}  /* HardwareSetAddress */


/*
    HardwareReadAddress

    Description:
        This function retrieves the MAC address of the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE to indicate successful.
*/

BOOLEAN HardwareReadAddress
(
    PHARDWARE pHardware )
{
    UCHAR i;

    for ( i = 0; i < MAC_ADDRESS_LENGTH; i++ )
    {
        HW_READ_BYTE( pHardware, ( UINT )( REG_DMA_MAC_ADDR_LO + i ),
            ( PUCHAR ) &pHardware->m_bPermanentAddress[ MAC_ADDR_ORDER( i )]);
    }

#ifdef DBG
    DBG_PRINT( "get addr: " );
    PrintMacAddress( pHardware->m_bPermanentAddress );
    DBG_PRINT( NEWLINE );
#endif

    if ( !pHardware->m_bMacOverrideAddr )
    {
        MOVE_MEM( pHardware->m_bOverrideAddress,
            pHardware->m_bPermanentAddress, MAC_ADDRESS_LENGTH );
#ifdef SET_MAC_ADDR
memset( pHardware->m_bOverrideAddress, 0, MAC_ADDRESS_LENGTH );
#endif
        if ( 0 == pHardware->m_bOverrideAddress[ 0 ]  &&
                0 == pHardware->m_bOverrideAddress[ 1 ]  &&
                0 == pHardware->m_bOverrideAddress[ 2 ]  &&
                0 == pHardware->m_bOverrideAddress[ 3 ]  &&
                0 == pHardware->m_bOverrideAddress[ 4 ]  &&
                0 == pHardware->m_bOverrideAddress[ 5 ] )
        {
            MOVE_MEM( pHardware->m_bPermanentAddress,
                DEFAULT_MAC_ADDRESS, MAC_ADDRESS_LENGTH );
            MOVE_MEM( pHardware->m_bOverrideAddress,
                DEFAULT_MAC_ADDRESS, MAC_ADDRESS_LENGTH );
            pHardware->m_bPermanentAddress[ 5 ] = pHardware->m_wPhyAddr;
            pHardware->m_bOverrideAddress[ 5 ] = pHardware->m_wPhyAddr;
            HardwareSetAddress( pHardware );
        }
    }
    else
    {
        HardwareSetAddress( pHardware );
    }

    return TRUE;
}  /* HardwareReadAddress */


/*
    HardwareClearMulticast

    Description:
        This routine removes all multicast addresses set in the hardware.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareClearMulticast
(
    PHARDWARE pHardware )
{
    UCHAR i;

    for ( i = 0; i < HW_MULTICAST_SIZE; i++ )
    {
        pHardware->m_bMulticastBits[ i ] = 0;
        HW_WRITE_BYTE( pHardware, ( UINT )( REG_MULTICAST_0_OFFSET + i ), 0 );
    }
}  /* HardwareClearMulticast */


#if 0
/* The little-endian AUTODIN II ethernet CRC calculation.
   N.B. Do not use for bulk data, use a table-based routine instead.
   This is common code and should be moved to net/core/crc.c */
static unsigned int const ethernet_polynomial_le = 0xedb88320U;
static unsigned int ether_crc_le (
    int            length,
    unsigned char *data )
{
    unsigned int crc = 0xffffffff;  /* Initial value. */
    while ( --length >= 0 ) {
        unsigned char current_octet = *data++;
        int bit;

        for ( bit = 8; --bit >= 0; current_octet >>= 1 ) {
            if ( ( crc ^ current_octet ) & 1 ) {
                crc >>= 1;
                crc ^= ethernet_polynomial_le;
            }
            else
                crc >>= 1;
        }
    }
    return crc;
}  /* ether_crc_le */
#endif


/*
    ether_crc

    Description:
        This function generates a CRC-32 from the data block.
        It is used to calculate values for the hardware multicast filter hash table,
        or wake up frame CRC value.

    Parameters:
        int             length,
            the length of the block data

        unsigned char * data
            Pointer to the block data.

    Return:
        The computed CRC of the data.
*/
static UINT32 const ethernet_polynomial = 0x04c11db7U;
UINT32 ether_crc
(
    int  length,
    unsigned char *data
)
{
    long crc = -1;
    while ( --length >= 0 )
    {
        unsigned char current_octet = *data++;
        int bit;

        for ( bit = 0; bit < 8; bit++, current_octet >>= 1 )
        {
            crc = ( crc << 1 ) ^
                (( crc < 0 ) ^ ( current_octet & 1 ) ?
                ethernet_polynomial : 0 );
        }
    }
    return(( UINT32 ) crc );
}  /* ether_crc */


/*
    HardwareSetGroupAddress

    Description:
        This function programs multicast addresses for the hardware to accept
        those addresses.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE to indicate success.
*/

BOOLEAN HardwareSetGroupAddress
(
    PHARDWARE pHardware )
{
    UCHAR i;
    int   index;
    int   position;
    int   value;

    memset( pHardware->m_bMulticastBits, 0,
        sizeof( UCHAR ) * HW_MULTICAST_SIZE );

#ifdef VERIFY_MULTICAST
    DBG_PRINT( "set multicast:"NEWLINE );
    {
#if 0
        UCHAR spanning_tree[] = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x00 };
#else
        UCHAR spanning_tree[] = { 0x01, 0x00, 0x5E, 0x00, 0xFB, 0x00 };
#endif
        UCHAR service_loc[] = { 0x01, 0x00, 0x5E, 0x00, 0x01, 0x3C };
        UCHAR ipv6_fe80[] = { 0x33, 0x33, 0xFF, 0x86, 0x90, 0x03 };
        UCHAR ipv6_fec0[] = { 0x33, 0x33, 0xFF, 0x00, 0x00, 0x03 };

        position = ( ether_crc( MULTICAST_LEN, spanning_tree ) >> 26 ) & 0x3f;
        index = position >> 3;
        value = 1 << ( position & 7 );
        pHardware->m_bMulticastBits[ index ] |= ( UCHAR ) value;
        position = ( ether_crc( MULTICAST_LEN, service_loc ) >> 26 ) & 0x3f;
        index = position >> 3;
        value = 1 << ( position & 7 );
        pHardware->m_bMulticastBits[ index ] |= ( UCHAR ) value;
        position = ( ether_crc( MULTICAST_LEN, ipv6_fe80 ) >> 26 ) & 0x3f;
        index = position >> 3;
        value = 1 << ( position & 7 );
        pHardware->m_bMulticastBits[ index ] |= ( UCHAR ) value;
        position = ( ether_crc( MULTICAST_LEN, ipv6_fec0 ) >> 26 ) & 0x3f;
        index = position >> 3;
        value = 1 << ( position & 7 );
        pHardware->m_bMulticastBits[ index ] |= ( UCHAR ) value;
    }
#endif
    for ( i = 0; i < pHardware->m_bMulticastListSize; i++ )
    {

#ifdef DBG
        PrintMacAddress( pHardware->m_bMulticastList[ i ]);
        DBG_PRINT( NEWLINE );
#endif
        position = ( ether_crc( MULTICAST_LEN,
            pHardware->m_bMulticastList[ i ]) >> 26 ) & 0x3f;
        index = position >> 3;
        value = 1 << ( position & 7 );
        pHardware->m_bMulticastBits[ index ] |= ( UCHAR ) value;
    }

    for ( i = 0; i < HW_MULTICAST_SIZE; i++ )
    {
        HW_WRITE_BYTE( pHardware, ( UINT )( REG_MULTICAST_0_OFFSET + i ),
            pHardware->m_bMulticastBits[ i ]);
    }
    return TRUE;
}  /* HardwareSetGroupAddress */


/*
    HardwareSetMulticast

    Description:
        This function enables/disables the hardware to accept all multicast
        packets.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bMulticast
            To turn on or off the all multicast feature.

    Return (BOOLEAN):
        TRUE to indicate success.
*/

BOOLEAN HardwareSetMulticast
(
    PHARDWARE pHardware,
    UCHAR     bMulticast )
{
    pHardware->m_bAllMulticast = bMulticast;

    HardwareStopReceive( pHardware );  /* Stop receiving for reconfiguration */

    if ( bMulticast )
        pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_ALL_MULTICAST;
    else
        pHardware->m_dwReceiveConfig &= ~DMA_RX_CTRL_ALL_MULTICAST;

    if ( pHardware->m_bEnabled )
        HardwareStartReceive( pHardware );
    return TRUE;
}  /* HardwareSetMulticast */


/*
    HardwareSetPromiscuous

    Description:
        This function enables/disables the hardware to accept all packets.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bMulticast
            To turn on or off the promiscuous feature.

    Return (BOOLEAN):
        TRUE to indicate success.
*/

BOOLEAN HardwareSetPromiscuous
(
    PHARDWARE pHardware,
    UCHAR     bPromiscuous )
{
    pHardware->m_bPromiscuous = bPromiscuous;

#if !defined( TEST_RX_LOOPBACK ) && !defined( DEBUG_RX_ALL )
    HardwareStopReceive( pHardware );  /* Stop receiving for reconfiguration */

    if ( bPromiscuous )
        pHardware->m_dwReceiveConfig |= DMA_RX_CTRL_PROMISCUOUS;
    else
        pHardware->m_dwReceiveConfig &= ~DMA_RX_CTRL_PROMISCUOUS;

    if ( pHardware->m_bEnabled )
        HardwareStartReceive( pHardware );
#endif
    return TRUE;
}  /* HardwareSetPromiscuous */

/* -------------------------------------------------------------------------- */

/*
    HardwareReadMIBCounter

    Description:
        This routine reads a MIB counter of the hardware port.
        Hardware interrupts are disabled to minimize corruption of read data.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT wAddr
            The address of the counter.

        PULONGLONG pqData
            Buffer to store the counter.

    Return (None):
*/

void HardwareReadMIBCounter
(
    PHARDWARE  pHardware,
    USHORT     wAddr,
    PULONGLONG pqData )
{
    UINT dwData;

    dwData = KS_READ_REG( pHardware->m_dwRegCounter + wAddr * 4 );
    *pqData += dwData;
}  /* HardwareReadMIBCounter */


/*
    HardwareReadCounters

    Description:
        This routine is used to read the counters of the port periodically to
        avoid counter overflow.  The hardware should be acquired first before
        calling this routine.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (int):
        non-zero when not all counters not read.
*/

int HardwareReadCounters (
    PHARDWARE pHardware )
{
    UINT data;

    ASSERT( pHardware->m_bAcquire );

    while ( pHardware->m_bCurrentCounter < pHardware->m_nRegCounter )
    {
        HardwareReadMIBCounter( pHardware, pHardware->m_bCurrentCounter,
            &pHardware->m_cnMIB[ pHardware->m_bCurrentCounter ]);
        ++pHardware->m_bCurrentCounter;
    }
    HW_READ_DWORD( pHardware, REG_PKT_DROP, &data );
    pHardware->m_cnMIB[ pHardware->m_bCurrentCounter++ ] += data;
    HW_READ_DWORD( pHardware, REG_CHECKSUM_DROP, &data );
    pHardware->m_cnMIB[ pHardware->m_bCurrentCounter++ ] += data;
    pHardware->m_bCurrentCounter = 0;
    return 0;
}  /* HardwareReadCounters */


/*
    HardwareInitCounters

    Description:
        This routine is used to initialize all counters to zero if the hardware
        cannot do it after reset.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

void HardwareInitCounters (
    PHARDWARE pHardware )
{
    UINT data;

    ASSERT( pHardware->m_bAcquire );

    pHardware->m_bCurrentCounter = 0;
    do
    {
        HardwareReadMIBCounter( pHardware, pHardware->m_bCurrentCounter,
            &pHardware->m_cnMIB[ pHardware->m_bCurrentCounter ]);
        ++pHardware->m_bCurrentCounter;
    } while ( pHardware->m_bCurrentCounter < pHardware->m_nRegCounter );
    HW_READ_DWORD( pHardware, REG_PKT_DROP, &data );
    pHardware->m_cnMIB[ pHardware->m_bCurrentCounter++ ] += data;
    HW_READ_DWORD( pHardware, REG_CHECKSUM_DROP, &data );
    pHardware->m_cnMIB[ pHardware->m_bCurrentCounter++ ] += data;
    memset(( void* ) pHardware->m_cnMIB, 0,
        sizeof( ULONGLONG ) * TOTAL_PORT_COUNTER_NUM );
    pHardware->m_bCurrentCounter = 0;
}  /* HardwareInitCounters */

/* -------------------------------------------------------------------------- */

/*
    HardwareSetWolEnable

    Description:
        This routine turns on or off the Wake-on-LAN features.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        USHORT frame
            The mask bit to change in the register.

        int enable
            TRUE to enable bit.

    Return (None):
*/

void HardwareSetWolEnable (
    PHARDWARE pHardware,
    USHORT    frame,
    int       enable )
{
    UINT data;

    HW_READ_DWORD( pHardware, REG_WOL_CTRL, &data );
    if ( enable )
        data |= frame;
    else
        data &= ~frame;
    HW_WRITE_DWORD( pHardware, REG_WOL_CTRL, data );
}  /* HardwareSetWolEnable */


/*
    HardwareSetWolFrame

    Description:
        This routine programs the Wake-on-LAN pattern entry to detect
            specified pattern.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int index
            The Wake-on-LAN pattern entry.

        ULONG masksize
            The mask data size.

        PUCHAR mask
            The mask data.

        ULONG patternsize
            The pattern data size.

        PUCHAR pattern
            The pattern data.

    Return (None):
*/

void HardwareSetWolFrame (
    PHARDWARE pHardware,
    int       index,
    ULONG     masksize,
    PUCHAR    mask,
    ULONG     patternsize,
    PUCHAR    pattern )
{
    int    bits;
    int    from;
    int    len;
    int    to;
    UINT32 crc;
    UCHAR  data[ 64 ];
    UCHAR  bData = 0;

    if ( patternsize > 64 )
    {
        patternsize = 64;
    }

    index *= REG_FRAME1_CRC - REG_FRAME0_CRC;
    bits = len = from = to = 0;
    do
    {
        if ( bits )
        {
            if ( ( bData & 1 ) )
            {
                data[ to++ ] = pattern[ from ];
            }
            bData >>= 1;
            ++from;
            --bits;
        }
        else
        {
            bData = mask[ len ];
            HW_WRITE_BYTE( pHardware, REG_FRAME0_MASK0 + index + len,
                bData );
            ++len;
            if ( bData )
                bits = 8;
            else
                from += 8;
        }
    } while ( from < ( int ) patternsize );
    crc = ether_crc( to, data );
    HW_WRITE_DWORD( pHardware, REG_FRAME0_CRC + index, crc );

#ifdef DBG
    DBG_PRINT( "%08X: %d=", crc, to );
    for ( from = 0; from < to; from++ )
    {
        DBG_PRINT( "%02X ", data[ from ]);
    }
    DBG_PRINT( NEWLINE );
    DBG_PRINT( "%08X: ", REG_FRAME0_CRC + index );
    for ( len = 0; len < 0xC; len += 4 )
    {
        HW_READ_DWORD( pHardware, REG_FRAME0_CRC + index + len, &crc );
        DBG_PRINT( "%08X ", crc );
    }
    DBG_PRINT( NEWLINE );
#endif
}  /* HardwareSetWolFrame */

/* -------------------------------------------------------------------------- */

#ifdef CONFIG_KSZ8692VB
/*
    ACL_Reg

    Description:
        This helper function returns the correct ACL register to access.

    Parameters:
        int acl
            The ACL index.

        UINT reg
            The ACL offset register.

    Return (UINT):
        The ACL register.
*/

static inline UINT ACL_Reg (
    int  acl,
    UINT reg )
{
    int offset = 0;

    if ( acl >= 16 )
    {
        acl -= 16;
        offset = 4;
    }
    acl *= 0x10;
    return( reg + acl + offset );
}  /* ACL_Reg */


/*
    HardwareSetACL

    Description:
        This routine programs the ACL configuration register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int enable
            TRUE to enable bit.

        UINT mask
            The mask bit to change in the register.

    Return (None):
*/

void HardwareSetACL (
    PHARDWARE  pHardware,
    int        acl,
    int        enable,
    UINT       mask )
{
    UINT dwData;
    UINT dwReg = ACL_Reg( acl, REG_ACL_CONF1 );

    HW_READ_DWORD( pHardware, dwReg, &dwData );
    if ( enable )
        dwData |= mask;
    else
        dwData &= ~mask;
    HW_WRITE_DWORD( pHardware, dwReg, dwData );
#ifdef DBG
    DBG_PRINT( " cfg %04X = %08x"NEWLINE, dwReg, dwData );
#endif
}  /* HardwareSetACL */


/*
    HardwareSet_ACL_Block

    Description:
        This routine turns on or off the ACL block feature.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int enable
            TRUE to enable feature.

    Return (None):
*/

void HardwareSet_ACL_Block (
    PHARDWARE  pHardware,
    int        enable )
{
    UINT dwData;

    HW_READ_DWORD( pHardware, REG_DMA_MISC_CFG, &dwData );
    if ( enable )
        dwData |= DMA_BLOCK_PACKET;
    else
        dwData &= ~DMA_BLOCK_PACKET;
    HW_WRITE_DWORD( pHardware, REG_DMA_MISC_CFG, dwData );
}  /* HardwareSet_ACL_Block */


/*
    HardwareSet_ACL_Config

    Description:
        This helper routine programs the ACL registers.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int acl
            The ACL index.

        int filter
            The filter enable bit.

        int priority
            The receive high priority bit.

        UINT dwData
            The ACL data value.

        UINT dwMask
            The ACL mask value.

        UINT dwCfg
            The ACL configuration value.

    Return (None):
*/

static void HardwareSet_ACL_Config (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    UINT       dwData,
    UINT       dwMask,
    UINT       dwCfg )
{
    UINT dwReg;

    dwReg = ACL_Reg( acl, REG_ACL_DATA1 );
    HW_WRITE_DWORD( pHardware, dwReg, dwData );
#ifdef DBG
    DBG_PRINT( " data %04X = %08x, ", dwReg, dwData );
#endif
    dwReg = ACL_Reg( acl, REG_ACL_MASK1 );
    HW_WRITE_DWORD( pHardware, dwReg, dwMask );
#ifdef DBG
    DBG_PRINT( "mask %04X = %08x, ", dwReg, dwMask );
#endif
    dwReg = ACL_Reg( acl, REG_ACL_CONF1 );
    if ( filter )
        dwCfg |= ACL_FILTER_EN;
    if ( priority )
        dwCfg |= ACL_HI_PRIO;
    HW_WRITE_DWORD( pHardware, dwReg, dwCfg );
#ifdef DBG
    DBG_PRINT( "cfg %04X = %08x"NEWLINE, dwReg, dwCfg );
#endif
}  /* HardwareSet_ACL_Config */


/*
    HardwareSet_ACL_Mac

    Description:
        This routine programs the ACL entry to filter MAC address.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int acl
            The ACL entry.

        int filter
            The filter enable bit.

        int priority
            The receive high priority bit.

        UCHAR* addr
            The MAC address.

    Return (None):
*/

void HardwareSet_ACL_Mac (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    UCHAR*     addr )
{
    UINT dwCfg;
    UINT data;
    UINT mask;

    data = (( UINT ) addr[ 2 ] << 24 ) | (( UINT ) addr[ 3 ] << 16 ) |
        (( UINT ) addr[ 4 ] << 8 ) | addr[ 5 ];
    mask = (( UINT ) addr[ 0 ] << 8 ) | addr[ 1 ];
    dwCfg = ACL_ENABLE | ACL_MAC_SA;
    HardwareSet_ACL_Config( pHardware, acl, filter, priority, data, mask,
        dwCfg );
}  /* HardwareSet_ACL_Mac */


/*
    HardwareSet_ACL_Offset

    Description:
        This routine programs the ACL entry to filter data pattern.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int acl
            The ACL entry.

        int filter
            The filter enable bit.

        int priority
            The receive high priority bit.

        int offset
            The offset value.

        UINT data
            The pattern data value.

        UINT mask
            The pattern mask.

    Return (None):
*/

void HardwareSet_ACL_Offset (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    int        offset,
    UINT       data,
    UINT       mask )
{
    UINT dwCfg;

    ASSERT( offset <= ACL_DATA_OFFSET_MASK );

    dwCfg = ACL_ENABLE | ACL_OFFSET_SCHEME | offset;
    HardwareSet_ACL_Config( pHardware, acl, filter, priority, data, mask,
        dwCfg );
}  /* HardwareSet_ACL_Offset */


/*
    HardwareSet_ACL_Protocol

    Description:
        This routine programs the ACL entry to filter packet protocol.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int acl
            The ACL entry.

        int filter
            The filter enable bit.

        int priority
            The receive high priority bit.

        UCHAR data
            The protocol number.

        UCHAR mask
            The protocol mask.

    Return (None):
*/

void HardwareSet_ACL_Protocol (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    UCHAR      data,
    UCHAR      mask )
{
    UINT dwCfg;

    dwCfg = ACL_ENABLE | ACL_PROTOCOL;
    HardwareSet_ACL_Config( pHardware, acl, filter, priority, data, mask,
        dwCfg );
}  /* HardwareSet_ACL_Protocol */


/*
    HardwareSet_ACL_Port

    Description:
        This routine programs the ACL entry to filter port number.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int acl
            The ACL entry.

        int filter
            The filter enable bit.

        int priority
            The receive high priority bit.

        int cfg
            The filter mode.

        USHORT data
            The port number.

        USHORT mask
            The port mask.

    Return (None):
*/

void HardwareSet_ACL_Port (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    int        cfg,
    USHORT     data,
    USHORT     mask )
{
    UINT dwCfg;

    ASSERT( 1 <= cfg  &&  cfg <= 3 );

    dwCfg = ACL_ENABLE;
    if ( ( cfg & 2 ) )
        dwCfg |= ACL_PORT_DA;
    if ( ( cfg & 1 ) )
        dwCfg |= ACL_PORT_SA;
    HardwareSet_ACL_Config( pHardware, acl, filter, priority, data, mask,
        dwCfg );
}  /* HardwareSet_ACL_Port */


/*
    HardwareSet_ACL_IPv4

    Description:
        This routine programs the ACL entry to filter IPv4 address.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int acl
            The ACL entry.

        int filter
            The filter enable bit.

        int priority
            The receive high priority bit.

        int cfg
            The filter mode.

        UCHAR* addr
            The IPv4 address.

        UCHAR* mask
            The IPv4 mask.

    Return (None):
*/

void HardwareSet_ACL_IPv4 (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    int        cfg,
    UCHAR*     addr,
    UCHAR*     mask )
{
    UINT dwCfg;
    UINT data;
    UINT dwMask;

    ASSERT( 1 <= cfg  &&  cfg <= 3 );

    data = (( UINT ) addr[ 0 ] << 24 ) | (( UINT ) addr[ 1 ] << 16 ) |
        (( UINT ) addr[ 2 ] << 8 ) | addr[ 3 ];
    dwMask = (( UINT ) mask[ 0 ] << 24 ) | (( UINT ) mask[ 1 ] << 16 ) |
        (( UINT ) mask[ 2 ] << 8 ) | mask[ 3 ];
    dwCfg = ACL_ENABLE;
    if ( ( cfg & 1 ) )
        dwCfg |= ACL_IPV4_DA;
    if ( ( cfg & 2 ) )
        dwCfg |= ACL_IPV4_SA;
    HardwareSet_ACL_Config( pHardware, acl, filter, priority, data, dwMask,
        dwCfg );
}  /* HardwareSet_ACL_IPv4 */


/*
    HardwareSet_ACL_IPv6

    Description:
        This routine programs the ACL entry to filter IPv6 address.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        int acl
            The ACL entry.

        int filter
            The filter enable bit.

        int priority
            The receive high priority bit.

        int cfg
            The filter mode.

        UCHAR* addr
            The IPv6 address.

        UCHAR* mask
            The IPv6 mask.

    Return (None):
*/

void HardwareSet_ACL_IPv6 (
    PHARDWARE  pHardware,
    int        acl,
    int        filter,
    int        priority,
    int        cfg,
    UCHAR*     addr,
    UCHAR*     mask )
{
    UINT dwCfg;
    UINT dwReg;
    UINT dwData;
    int  i;

    ASSERT( !( acl % 4 ));
    ASSERT( 1 <= cfg  &&  cfg <= 3 );

    dwReg = ACL_Reg( acl, REG_ACL_DATA1 );
    dwReg += 0x30;
    for ( i = 0; i < 4; i++ )
    {
        dwData = (( UINT ) addr[ 0 ] << 24 ) | (( UINT ) addr[ 1 ] << 16 ) |
            (( UINT ) addr[ 2 ] << 8 ) | addr[ 3 ];
        HW_WRITE_DWORD( pHardware, dwReg, dwData );
#ifdef DBG
        DBG_PRINT( " %04X = %08x,", dwReg, dwData );
#endif
        dwReg -= 0x10;
        addr += 4;
    }
#ifdef DBG
    DBG_PRINT( NEWLINE );
#endif
    dwReg = ACL_Reg( acl, REG_ACL_MASK1 );
    dwReg += 0x30;
    for ( i = 0; i < 4; i++ )
    {
        dwData = (( UINT ) mask[ 0 ] << 24 ) | (( UINT ) mask[ 1 ] << 16 ) |
            (( UINT ) mask[ 2 ] << 8 ) | mask[ 3 ];
        HW_WRITE_DWORD( pHardware, dwReg, dwData );
#ifdef DBG
        DBG_PRINT( " %04X = %08x,", dwReg, dwData );
#endif
        dwReg -= 0x10;
        mask += 4;
    }
#ifdef DBG
    DBG_PRINT( NEWLINE );
#endif
    dwReg = ACL_Reg( acl, REG_ACL_CONF1 );
    dwCfg = ACL_ENABLE | ACL_IPV6;
    if ( ( cfg & 1 ) )
        dwCfg |= ACL_IPV6_DA;
    if ( ( cfg & 2 ) )
        dwCfg |= ACL_IPV6_SA;
    if ( filter )
        dwCfg |= ACL_FILTER_EN;
    if ( priority )
        dwCfg |= ACL_HI_PRIO;
    HW_WRITE_DWORD( pHardware, dwReg, dwCfg );
#ifdef DBG
    DBG_PRINT( " %04X = %08x"NEWLINE, dwReg, dwCfg );
#endif
}  /* HardwareSet_ACL_IPv6 */


void HardwareSetupACL (
    PHARDWARE pHardware,
    PTAclInfo acl,
    int       first,
    int       last )
{
    acl->changed = 1;
    if ( acl->enable  &&  first <= acl->mode  &&  acl->mode <= last ) {
        acl->changed = 0;
        switch ( acl->mode ) {
            case ACL_MODE_MAC:
                HardwareSet_ACL_Mac( pHardware, acl->acl,
                    acl->filter, acl->priority,
                    acl->mac );
                break;
            case ACL_MODE_OFFSET:
                HardwareSet_ACL_Offset( pHardware, acl->acl,
                    acl->filter, acl->priority,
                    acl->offset, acl->data, acl->mask );
                break;
            case ACL_MODE_PROTOCOL:
                HardwareSet_ACL_Protocol( pHardware, acl->acl,
                    acl->filter, acl->priority,
                    acl->protocol, acl->mask );
                break;
            case ACL_MODE_PORT_DST:
            case ACL_MODE_PORT_SRC:
            case ACL_MODE_PORT_BOTH:
                HardwareSet_ACL_Port( pHardware, acl->acl,
                    acl->filter, acl->priority,
                    acl->mode - ( ACL_MODE_PORT_DST - 1 ),
                    acl->port, acl->mask );
                break;
            case ACL_MODE_IPV4_DST:
            case ACL_MODE_IPV4_SRC:
            case ACL_MODE_IPV4_BOTH:
                HardwareSet_ACL_IPv4( pHardware, acl->acl,
                    acl->filter, acl->priority,
                    acl->mode - ( ACL_MODE_IPV4_DST - 1 ),
                    acl->ip4_addr, acl->ip4_mask );
                break;
            case ACL_MODE_IPV6_DST:
            case ACL_MODE_IPV6_SRC:
            case ACL_MODE_IPV6_BOTH:
            {
                int i;

                HardwareSet_ACL_IPv6( pHardware, acl->acl,
                    acl->filter, acl->priority,
                    acl->mode - ( ACL_MODE_IPV6_DST - 1 ),
                    acl->ip6_addr, acl->ip6_mask );
                for ( i = 0; i < 3; i++ ) {
                    acl++;
                    if ( acl->enable ) {
                        acl->enable = 0;
                        acl->changed = 1;
                        HardwareSet_ACL_Enable( pHardware, acl->acl, 0 );
                    }
                }
                break;
            }
        }
    }
}  /* HardwareSetupACL */

/* -------------------------------------------------------------------------- */

/*
    HardwareSetBroadcastTraffic

    Description:
        This routine programs the broadcast traffic control preload value into
        the hardware register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bEnable
            TRUE to enable feature.

        UCHAR bValue
            The preload value.

    Return (None):
*/

void HardwareSetBroadcastTraffic (
    PHARDWARE pHardware,
    UCHAR     bEnable,
    UCHAR     bValue )
{
    UINT data = 0;

    if ( bEnable )
    {
        data = bValue | DMA_BROADCAST_CNTL_EN;
    }
    HW_WRITE_DWORD( pHardware, REG_BROADCAST_CNTL, data );
}  /* HardwareSetBroadcastTraffic */

/* -------------------------------------------------------------------------- */

/*
    HardwareSetDiffServPriority

    Description:
        This routine programs the DiffServ priority into the hardware
            registers.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bEnable
            TRUE to enable feature.

        UINT Mapping0
        UINT Mapping1
            The DiffServ priority 64-bit mapping value.

    Return (None):
*/

void HardwareSetDiffServPriority (
    PHARDWARE pHardware,
    UCHAR     bEnable,
    UINT32    dwMapping0,
    UINT32    dwMapping1 )
{
    if ( bEnable )
    {
        HW_WRITE_DWORD( pHardware, REG_QOS_TOS0, dwMapping0 );
        HW_WRITE_DWORD( pHardware, REG_QOS_TOS1, dwMapping1 );
        HW_WRITE_DWORD( pHardware, REG_QOS_TOS2, QOS_TOS_ENABLE );
    }
    else
        HW_WRITE_DWORD( pHardware, REG_QOS_TOS2, 0 );
}  /* HardwareSetDiffServPriority */


/*
    HardwareSet802_1P_Priority

    Description:
        This routine programs the 802.1p priority into the hardware register.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UCHAR bEnable
            TRUE to enable feature.

        UCHAR bMapping
            The 802.1p priority mapping value.

    Return (None):
*/

void HardwareSet802_1P_Priority (
    PHARDWARE pHardware,
    UCHAR     bEnable,
    UCHAR     bMapping )
{
    UINT data = 0;

    if ( bEnable )
    {
        data = bMapping | QOS_802_1P_ENABLE;
    }
    HW_WRITE_DWORD( pHardware, REG_QOS_TAG, data );
}  /* HardwareSet802_1P_Priority */
#endif

/* -------------------------------------------------------------------------- */

UCHAR TestPacket[] =
{
/*  0 */    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* ff:ff:ff:ff:ff:ff (DA) */
/*  6 */    0x08, 0x00, 0x70, 0x22, 0x44, 0x55,     /* 08:00:70:22:44:55 (SA) */
/* 12 */    0x08, 0x06,                             /* ARP */
/* 14 */    0x00, 0x01,                             /* Ethernet */
/* 16 */    0x08, 0x00,                             /* IP */
/* 18 */    0x06, 0x04,
/* 20 */    0x00, 0x01,                             /* Request */
/* 22 */    0x08, 0x00, 0x70, 0x22, 0x44, 0x55,     /* 08:00:70:22:44:55 (SA) */
/* 28 */    0xC0, 0xA8, 0x01, 0x01,                 /* 192.168.1.1  (Source IP) */
/* 32 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 00:00:00:00:00:00 (DA) */
/* 38 */    0xC0, 0xA8, 0x01, 0x1E,                 /* 192.168.1.30 (Dest IP) */
/* 42 */    0x5A, 0xA5,                             /* Data */
};

/*
    TestLoopback

    Description:
        This function is used to test the loopback function.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (BOOLEAN):
        TRUE if loopback is working; otherwise, FALSE.
*/

BOOLEAN TestLoopback
(
    PHARDWARE pHardware )
{
#if 0
    PTDescInfo pRx = &pHardware->m_RxDescInfo;
#endif
    PTDescInfo pTx = &pHardware->m_TxDescInfo;
    UINT       IntEnable;
    UINT       IntStatus;
    UINT       IntMask = INT_TX | INT_RX;
    int        nLength = 42;
    int        cnTimeOut = 100;
    BOOLEAN    rc = TRUE;
    USHORT     i;

    pHardware->m_dwTransmitConfig |= DMA_TX_CTRL_LOOPBACK;
    HardwareEnable( pHardware );

    if ( HardwareAllocPacket( pHardware, nLength, 1 ) )
    {
        HardwareSetTransmitBuffer( pHardware, 0 );
        HardwareSetTransmitLength( pHardware, nLength );
        HardwareSendPacket( pHardware );

        do {
            HardwareReadInterrupt( pHardware, &IntStatus );
            IntEnable = IntStatus & IntMask;
        } while ( !IntEnable  &&  cnTimeOut-- );
        if ( ( IntEnable & INT_TX ) )
        {
            /* Acknowledge the interrupt. */
            HardwareAcknowledgeTransmit( pHardware );
            IntEnable &= ~INT_TX;
        }
        else
        {
            HardwareFreeTxPacket( pTx );
            rc = FALSE;
        }

        cnTimeOut = 100;
        while ( !IntEnable  &&  cnTimeOut-- )
        {
            HardwareReadInterrupt( pHardware, &IntStatus );
            IntEnable = IntStatus & IntMask;
        }
        if ( ( IntEnable & INT_RX ) )
        {
            /* Receive the packet. */
            HardwareReceive( pHardware );

            /* Acknowledge the interrupt. */
            HardwareAcknowledgeReceive( pHardware );

            for ( i = 0; i < nLength; i++ )
            {
                if ( pHardware->m_bLookahead[ i ] != TestPacket[ i ] )
                {
                    rc = FALSE;
                    break;
                }
            }
        }
        else
            rc = FALSE;
    }
    else
        rc = FALSE;
    pHardware->m_dwTransmitConfig &= ~DMA_TX_CTRL_LOOPBACK;
    HardwareDisable( pHardware );
    return( rc );
}  /* TestLoopback */

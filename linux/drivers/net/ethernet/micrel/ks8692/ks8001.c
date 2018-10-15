/* ---------------------------------------------------------------------------
          Copyright (c) 2007-2008 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    ks8001.c - Micrel KS8001 PHY functions.

    Author      Date        Description
    THa         02/08/07    Created file.
    THa         07/28/08    Use UINT instead of ULONG.
   ---------------------------------------------------------------------------
*/


#include "target.h"
#include "hardware.h"
#include "ks_phy.h"


/* -------------------------------------------------------------------------- */

/* Control Register */
#define PHY_SPEED_100MBIT           0x2000
#define PHY_TRANSMIT_DISABLE        0x0001

#define CTRL_RESERVED               0x007E

/* Status Register */
#define STATUS_RESERVED             0x0780

/* PHY Identifier Registers */
#define PHY_8001_ID_1               0x0022
#define PHY_8001_ID_2               0x161A

/* Receiver Error Counter Register */
#define PHY_REG_RXER_COUNTER        21

#define PHY_REG_INT_CTRL_STATUS     27
#define PHY_INT_JABBER              0x8000
#define PHY_INT_RCV_ERROR           0x4000
#define PHY_INT_PAGE_RCV            0x2000
#define PHY_INT_PAR_DETECT_FAULT    0x1000
#define PHY_INT_LINK_PARTNER_ACK    0x0800
#define PHY_INT_LINK_DOWN           0x0400
#define PHY_INT_REMOTE_FAULT        0x0200
#define PHY_INT_LINK_UP             0x0100
#define PHY_STAT_JABBER             0x0080
#define PHY_STAT_RCV_ERROR          0x0040
#define PHY_STAT_PAGE_RCV           0x0020
#define PHY_STAT_PAR_DETECT_FAULT   0x0010
#define PHY_STAT_LINK_PARTNER_ACK   0x0008
#define PHY_STAT_LINK_DOWN          0x0004
#define PHY_STAT_REMOTE_FAULT       0x0002
#define PHY_STAT_LINK_UP            0x0001

#define PHY_REG_LINK_MD             29
#define PHY_START_CABLE_DIAG        0x8000

#define PHY_CABLE_DIAG_RESULT       0x6000
#define PHY_CABLE_STAT_NORMAL       0x0000
#define PHY_CABLE_STAT_OPEN         0x2000
#define PHY_CABLE_STAT_SHORT        0x4000
#define PHY_CABLE_STAT_FAILED       0x6000

#define PHY_CABLE_FAULT_COUNTER     0x01FF

#define PHY_REG_PHY_CTRL            30
#define PHY_CTRL_LED_MODE           0xC000
#define PHY_LED_COL_FD_SP_LINK      0x0000
#define PHY_LED_ACT_FD_SP_LINK      0x4000
#define PHY_LED_ACT_FD_100_10       0x8000
#define PHY_STAT_REVERSED_POLARITY  0x2000
#define PHY_REMOTE_FAULT_DETECTED   0x1000
#define PHY_STAT_MDIX               0x0800
#define PHY_STAT_REMOTE_LOOPBACK    0x0080

#define PHY_REG_100TX_PHY_CTRL      31
#define PHY_FORCE_MDIX              0x4000
#define PHY_AUTO_MDIX_DISABLE       0x2000
#define PHY_ENERY_DETECT            0x1000
#define PHY_FORCE_LINK              0x0800
#define PHY_POWER_SAVING            0x0400
#define PHY_INT_LEVEL               0x0200
#define PHY_JABBER_COUNTER_ENABLE   0x0100
#define PHY_AUTO_NEG_COMPLETE       0x0080
#define PHY_FLOW_CTRL_CAPABLE       0x0040
#define PHY_ISOLATED                0x0020
#define PHY_OPERATION_MODE          0x001C
#define PHY_IN_AUTO_NEG             0x0000
#define PHY_IN_10BT                 0x0004
#define PHY_IN_100BTX               0x0008
#define PHY_IN_DEFAULT              0x000C
#define PHY_IN_10BT_FD              0x0014
#define PHY_IN_100BTX_FD            0x0018
#define PHY_IN_ISOLATE              0x001C
#define PHY_SQE_TEST_ENABLE         0x0002
#define PHY_SCRAMBLING_DISABLE      0x0001

/* -------------------------------------------------------------------------- */

#define PHY_LINKMD_TIMEOUT  10
#define PHY_RESET_TIMEOUT   10


#define HW_READ_PHY_CROSSOVER( phwi, data )                                 \
    HardwareReadPhy(( phwi )->m_wPhyAddr, PHY_REG_100TX_PHY_CTRL, &data )

#define HW_WRITE_PHY_CROSSOVER( phwi, data )                                \
    HardwareWritePhy(( phwi )->m_wPhyAddr, PHY_REG_100TX_PHY_CTRL, data )

#define HW_READ_PHY_POLARITY( phwi, data )                                  \
    HardwareReadPhy(( phwi )->m_wPhyAddr, PHY_REG_PHY_CTRL, &data )

#define HW_READ_PHY_LINK_MD( phwi, data )                                   \
    HardwareReadPhy(( phwi )->m_wPhyAddr, PHY_REG_LINK_MD, &data )

#define HW_WRITE_PHY_LINK_MD( phwi, data )                                  \
    HardwareWritePhy(( phwi )->m_wPhyAddr, PHY_REG_LINK_MD, data )

/* -------------------------------------------------------------------------- */

#define CABLE_LEN_MAXIMUM     15000
#define CABLE_LEN_MULTIPLIER  41


/*
    HardwareGetCableStatus

    Description:
        This routine is used to get the cable status.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        void* pBuffer
            Buffer to store the cable status.

    Return (None):
*/

static void ks8001_GetCableStatus (
    PHARDWARE pHardware,
    void*     pBuffer )
{
    UINT*  pulData = ( UINT* ) pBuffer;

    USHORT Control;
    USHORT Crossover;
    USHORT Data;
    USHORT LowSpeed;
    UINT   ulStatus;
    UINT   ulLength[ 5 ];
    UCHAR  bStatus[ 5 ];
    int    i;
    int    cnTimeOut;

    HW_READ_PHY_LINK_STATUS( pHardware, Data );
    bStatus[ 4 ] = CABLE_UNKNOWN;
    if ( ( Data & PHY_LINK_STATUS ) )
    {
        bStatus[ 4 ] = CABLE_GOOD;
        ulLength[ 4 ] = 1;
        bStatus[ 0 ] = CABLE_GOOD;
        ulLength[ 0 ] = 1;
        bStatus[ 1 ] = CABLE_GOOD;
        ulLength[ 1 ] = 1;

        HW_READ_PHY_POLARITY( pHardware, Data );
        ulStatus = 0;
        if ( ( Data & PHY_STAT_MDIX ) )
            ulStatus |= STATUS_CROSSOVER;
        if ( ( Data & PHY_STAT_REVERSED_POLARITY ) )
            ulStatus |= STATUS_REVERSED;
        if ( ( ulStatus & ( STATUS_CROSSOVER | STATUS_REVERSED )) ==
                ( STATUS_CROSSOVER | STATUS_REVERSED ) )
            bStatus[ 4 ] = CABLE_CROSSED_REVERSED;
        else if ( ( ulStatus & STATUS_CROSSOVER ) == STATUS_CROSSOVER )
            bStatus[ 4 ] = CABLE_CROSSED;
        else if ( ( ulStatus & STATUS_CROSSOVER ) == STATUS_REVERSED )
            bStatus[ 4 ] = CABLE_REVERSED;
        goto GetCableStatusDone;
    }

    /* Put in 10 Mbps mode. */
    HW_READ_PHY_CTRL( pHardware, Control );
    Data = Control;
    Data &= ~( PHY_AUTO_NEG_ENABLE | PHY_SPEED_100MBIT | PHY_FULL_DUPLEX );
    LowSpeed = Data;
    HW_WRITE_PHY_CTRL( pHardware, Data );

    HW_READ_PHY_CROSSOVER( pHardware, Crossover );

    for ( i = 0; i < 2; i++ )
    {
        Data = Crossover;

        /* Disable auto MDIX. */
        Data |= PHY_AUTO_MDIX_DISABLE;
        if ( 0 == i )
            Data &= ~PHY_FORCE_MDIX;
        else
            Data |= PHY_FORCE_MDIX;
        HW_WRITE_PHY_CROSSOVER( pHardware, Data );

        /* Disable transmitter. */
        Data = LowSpeed;
        Data |= PHY_TRANSMIT_DISABLE;
        HW_WRITE_PHY_CTRL( pHardware, Data );

        /* Wait at most 1 second.*/
        DelayMillisec( 100 );

        /* Enable transmitter. */
        Data &= ~PHY_TRANSMIT_DISABLE;
        HW_WRITE_PHY_CTRL( pHardware, Data );

        /* Start cable diagnostic test. */
        HW_READ_PHY_LINK_MD( pHardware, Data );
        Data |= PHY_START_CABLE_DIAG;
        HW_WRITE_PHY_LINK_MD( pHardware, Data );
        cnTimeOut = PHY_RESET_TIMEOUT;
        do
        {
            if ( !--cnTimeOut )
            {
                break;
            }
            DelayMillisec( 10 );
            HW_READ_PHY_LINK_MD( pHardware, Data );
        } while ( ( Data & PHY_START_CABLE_DIAG ) );

        ulLength[ i ] = 0;
        bStatus[ i ] = CABLE_UNKNOWN;

        if ( !( Data & PHY_START_CABLE_DIAG ) )
        {
            ulLength[ i ] = ( Data & PHY_CABLE_FAULT_COUNTER ) *
                CABLE_LEN_MULTIPLIER;
            Data &= PHY_CABLE_DIAG_RESULT;
            switch ( Data )
            {
                case PHY_CABLE_STAT_NORMAL:
                    bStatus[ i ] = CABLE_GOOD;
                    break;
                case PHY_CABLE_STAT_OPEN:
                    bStatus[ i ] = CABLE_OPEN;
                    break;
                case PHY_CABLE_STAT_SHORT:
                    bStatus[ i ] = CABLE_SHORT;
                    break;
            }
        }
        if ( CABLE_GOOD == bStatus[ i ] )
        {
            ulLength[ i ] = 1;
        }
    }

    HW_WRITE_PHY_CROSSOVER( pHardware, Crossover );
    HW_WRITE_PHY_CTRL( pHardware, Control );
    Control |= PHY_AUTO_NEG_RESTART;
    HW_WRITE_PHY_CTRL( pHardware, Control );

    ulLength[ 4 ] = ulLength[ 0 ];
    bStatus[ 4 ] = bStatus[ 0 ];
    for ( i = 1; i < 2; i++ )
    {
        if ( CABLE_GOOD == bStatus[ 4 ] )
        {
            if ( bStatus[ i ] != CABLE_GOOD )
            {
                bStatus[ 4 ] = bStatus[ i ];
                ulLength[ 4 ] = ulLength[ i ];
                break;
            }
        }
    }

GetCableStatusDone:
    /* Overall status */
    *pulData++ = ulLength[ 4 ];
    *pulData++ = bStatus[ 4 ];

    /* Pair 1-2 */
    *pulData++ = ulLength[ 0 ];
    *pulData++ = bStatus[ 0 ];

    /* Pair 3-6 */
    *pulData++ = ulLength[ 1 ];
    *pulData++ = bStatus[ 1 ];

    /* Pair 4-5 */
    *pulData++ = 0;
    *pulData++ = CABLE_UNKNOWN;

    /* Pair 7-8 */
    *pulData++ = 0;
    *pulData++ = CABLE_UNKNOWN;

}  /* HardwareGetCableStatus */


/*
    HardwareGetLinkStatus

    Description:
        This routine is used to get the link status.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        void* pBuffer
            Buffer to store the link status.

    Return (None):
*/

static void ks8001_GetLinkStatus (
    PHARDWARE pHardware,
    void*     pBuffer )
{
    UINT*  pulData = ( UINT* ) pBuffer;
    UINT   ulStatus;
    USHORT Capable;
    USHORT Crossover;
    USHORT Data;
    USHORT Polarity;
    USHORT Status;

    HW_READ_PHY_LINK_STATUS( pHardware, Capable );
    ulStatus = 0;
    if ( ( Capable & PHY_LINK_STATUS ) )
        ulStatus = 1;

    /* Link status */
    *pulData++ = ulStatus;

    HW_READ_PHY_AUTO_NEG( pHardware, Data );
    HW_READ_PHY_REM_CAP( pHardware, Status );

    HW_READ_PHY_CROSSOVER( pHardware, Crossover );
    HW_READ_PHY_POLARITY( pHardware, Polarity );

    ulStatus = 100000;
    if ( ( Crossover & PHY_OPERATION_MODE ) == PHY_IN_100BTX  ||
            ( Crossover & PHY_OPERATION_MODE ) == PHY_IN_100BTX_FD )
        ulStatus = 1000000;

    /* Link speed */
    *pulData++ = ulStatus;

    ulStatus = 0;

    if ( ( Crossover & PHY_OPERATION_MODE ) == PHY_IN_10BT_FD  ||
            ( Crossover & PHY_OPERATION_MODE ) == PHY_IN_100BTX_FD )
        ulStatus |= STATUS_FULL_DUPLEX;

    if ( ( Polarity & PHY_STAT_MDIX ) )
        ulStatus |= STATUS_CROSSOVER;

    if ( ( Polarity & PHY_STAT_REVERSED_POLARITY ) )
        ulStatus |= STATUS_REVERSED;

    /* Duplex mode with crossover and reversed polarity */
    *pulData++ = ulStatus;

    ulStatus = 0;
    if ( ( Capable & PHY_100BTX_FD_CAPABLE ) )
        ulStatus |= LINK_100MBPS_FULL;
    if ( ( Capable & PHY_100BTX_CAPABLE ) )
        ulStatus |= LINK_100MBPS_HALF;
    if ( ( Capable & PHY_10BT_FD_CAPABLE ) )
        ulStatus |= LINK_10MBPS_FULL;
    if ( ( Capable & PHY_10BT_CAPABLE ) )
        ulStatus |= LINK_10MBPS_HALF;

    ulStatus |= LINK_SYM_PAUSE;

    /* Capability */
    *pulData++ = ulStatus;

    ulStatus = 0;
#if 0
    if ( ( Data & PHY_AUTO_NEG_ASYM_PAUSE ) )
        ulStatus |= LINK_ASYM_PAUSE;
#endif
    if ( ( Data & PHY_AUTO_NEG_SYM_PAUSE ) )
        ulStatus |= LINK_SYM_PAUSE;
    if ( ( Data & PHY_AUTO_NEG_100BTX_FD ) )
        ulStatus |= LINK_100MBPS_FULL;
    if ( ( Data & PHY_AUTO_NEG_100BTX ) )
        ulStatus |= LINK_100MBPS_HALF;
    if ( ( Data & PHY_AUTO_NEG_10BT_FD ) )
        ulStatus |= LINK_10MBPS_FULL;
    if ( ( Data & PHY_AUTO_NEG_10BT ) )
        ulStatus |= LINK_10MBPS_HALF;

    if ( !( Crossover & PHY_AUTO_MDIX_DISABLE ) )
        ulStatus |= LINK_AUTO_MDIX;
    else if ( ( Crossover & PHY_FORCE_MDIX ) )
        ulStatus |= LINK_MDIX;
    ulStatus |= LINK_AUTO_POLARITY;

    /* Auto-Negotiation advertisement */
    *pulData++ = ulStatus;

    ulStatus = 0;
#if 0
    if ( ( Status & PHY_AUTO_NEG_ASYM_PAUSE ) )
        ulStatus |= LINK_ASYM_PAUSE;
#endif
    if ( ( Status & PHY_AUTO_NEG_SYM_PAUSE ) )
        ulStatus |= LINK_SYM_PAUSE;
    if ( ( Status & PHY_AUTO_NEG_100BTX_FD ) )
        ulStatus |= LINK_100MBPS_FULL;
    if ( ( Status & PHY_AUTO_NEG_100BTX ) )
        ulStatus |= LINK_100MBPS_HALF;
    if ( ( Status & PHY_AUTO_NEG_10BT_FD ) )
        ulStatus |= LINK_10MBPS_FULL;
    if ( ( Status & PHY_AUTO_NEG_10BT ) )
        ulStatus |= LINK_10MBPS_HALF;

    /* Link parnter capabilities */
    *pulData++ = ulStatus;
}  /* HardwareGetLinkStatus */


/*
    HardwareSetCapabilities

    Description:
        This routine is used to set the link capabilities.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

        UINT ulCapabilities
            A set of flags indicating different capabilities.

    Return (None):
*/

static void ks8001_SetCapabilities (
    PHARDWARE pHardware,
    UINT      ulCapabilities )
{
    UINT   InterruptMask;
    USHORT Data;
    int    cnTimeOut = PHY_RESET_TIMEOUT;

    InterruptMask = HardwareBlockInterrupt( pHardware );
    HardwareDisable( pHardware );

    HW_READ_PHY_AUTO_NEG( pHardware, Data );

#if 0
    if ( ( ulCapabilities & LINK_ASYM_PAUSE ) )
        Data |= PHY_AUTO_NEG_ASYM_PAUSE;
    else
        Data &= ~PHY_AUTO_NEG_ASYM_PAUSE;
#endif
    if ( ( ulCapabilities & LINK_SYM_PAUSE ) )
        Data |= PHY_AUTO_NEG_SYM_PAUSE;
    else
        Data &= ~PHY_AUTO_NEG_SYM_PAUSE;
    if ( ( ulCapabilities & LINK_100MBPS_FULL ) )
        Data |= PHY_AUTO_NEG_100BTX_FD;
    else
        Data &= ~PHY_AUTO_NEG_100BTX_FD;
    if ( ( ulCapabilities & LINK_100MBPS_HALF ) )
        Data |= PHY_AUTO_NEG_100BTX;
    else
        Data &= ~PHY_AUTO_NEG_100BTX;
    if ( ( ulCapabilities & LINK_10MBPS_FULL ) )
        Data |= PHY_AUTO_NEG_10BT_FD;
    else
        Data &= ~PHY_AUTO_NEG_10BT_FD;
    if ( ( ulCapabilities & LINK_10MBPS_HALF ) )
        Data |= PHY_AUTO_NEG_10BT;
    else
        Data &= ~PHY_AUTO_NEG_10BT;
    HW_WRITE_PHY_AUTO_NEG( pHardware, Data );

    HW_READ_PHY_CROSSOVER( pHardware, Data );
    if ( ( ulCapabilities & LINK_AUTO_MDIX ) )
        Data &= ~( PHY_AUTO_MDIX_DISABLE | PHY_FORCE_MDIX );
    else
    {
        Data |= PHY_AUTO_MDIX_DISABLE;
        if ( ( ulCapabilities & LINK_MDIX ) )
            Data |= PHY_FORCE_MDIX;
        else
            Data &= ~PHY_FORCE_MDIX;
    }
    HW_WRITE_PHY_CROSSOVER( pHardware, Data );

    /* Wait longer if cable is connected. */
    HW_READ_PHY_LINK_STATUS( pHardware, Data );
    if ( ( Data & PHY_LINK_STATUS ) )
        cnTimeOut *= 2;

    HW_READ_PHY_CTRL( pHardware, Data );
    Data &= ~PHY_MII_DISABLE;
    Data |= PHY_AUTO_NEG_ENABLE;
    Data |= PHY_AUTO_NEG_RESTART;
    HW_WRITE_PHY_CTRL( pHardware, Data );

    /* Wait for auto negotiation to complete. */
    DelayMillisec( 1500 );
    HW_READ_PHY_LINK_STATUS( pHardware, Data );
    while ( !( Data & PHY_LINK_STATUS ) )
    {
        if ( !--cnTimeOut )
        {
            break;
        }
        DelayMillisec( 100 );
        HW_READ_PHY_LINK_STATUS( pHardware, Data );
    }

#ifdef DEBUG_TIMEOUT
    cnTimeOut = PHY_RESET_TIMEOUT - cnTimeOut;
    if ( pHardware->m_nWaitDelay[ WAIT_DELAY_AUTO_NEG ] < cnTimeOut )
    {
        DBG_PRINT( "phy auto neg: %d = %x"NEWLINE, cnTimeOut, ( int ) Data );
        pHardware->m_nWaitDelay[ WAIT_DELAY_AUTO_NEG ] = cnTimeOut;
    }
#endif

    /* Check for disconnect. */
    HardwareCheckLink( pHardware );

    /* Check for connect. */
    HardwareCheckLink( pHardware );

    HardwareEnable( pHardware );
    HardwareRestoreInterrupt( pHardware, InterruptMask );
}  /* HardwareSetCapabilities */


/*
    HardwareGetLinkSpeed

    Description:
        This routine reads PHY registers to determine the current link status
        of the switch ports.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

static void ks8001_GetLinkSpeed (
    PHARDWARE pHardware )
{
    USHORT wLinkStatus;
    USHORT wLocal;
    USHORT wRemote;
    UINT   data;
    int    change = FALSE;

    HW_READ_PHY_LINK_STATUS( pHardware, wLinkStatus );
    HW_READ_PHY_AUTO_NEG( pHardware, wLocal );
    HW_READ_PHY_REM_CAP( pHardware, wRemote );

    if ( ( wLinkStatus & PHY_LINK_STATUS ) )
    {
        if ( MediaStateConnected != pHardware->m_ulHardwareState )
            change = TRUE;
        else if ( pHardware->m_wAdvertised != ( wLocal & PHY_LINK_SUPPORT )  ||
                pHardware->m_wLinkPartner != ( wRemote & PHY_LINK_SUPPORT ) )
            change = TRUE;
        pHardware->m_ulHardwareState = MediaStateConnected;
    }
    else
    {
        if ( MediaStateDisconnected != pHardware->m_ulHardwareState )
            change = TRUE;
        pHardware->m_ulHardwareState = MediaStateDisconnected;
    }

    if ( change )
    {
        if ( MediaStateConnected == pHardware->m_ulHardwareState )
        {
            pHardware->m_ulTransmitRate = 100000;
            if ( (( wLocal & PHY_AUTO_NEG_100BTX_FD )  &&
                    ( wRemote & PHY_AUTO_NEG_100BTX_FD ))  ||
                    (( wLocal & PHY_AUTO_NEG_100BTX )  &&
                    ( wRemote & PHY_AUTO_NEG_100BTX )) )
                pHardware->m_ulTransmitRate = 1000000;

            pHardware->m_ulDuplex = 1;
            if ( (( wLocal & PHY_AUTO_NEG_100BTX_FD )  &&
                    ( wRemote & PHY_AUTO_NEG_100BTX_FD ))  ||
                    (( wLocal & PHY_AUTO_NEG_10BT_FD )  &&
                    ( wRemote & PHY_AUTO_NEG_10BT_FD )  &&
                    ( !( wLocal & PHY_AUTO_NEG_100BTX )  ||
                    !( wRemote & PHY_AUTO_NEG_100BTX ))) )
                pHardware->m_ulDuplex = 2;

            pHardware->m_wAdvertised = wLocal & PHY_LINK_SUPPORT;
            pHardware->m_wLinkPartner = wRemote & PHY_LINK_SUPPORT;

            HW_READ_DWORD( pHardware, REG_DMA_MISC_CFG, &data );
            data &= ~( MISC_PORT_1000M | MISC_PORT_100M | MISC_PORT_FD );
            data &= ~MISC_RGMII_SELECT;
            data &= ~DMA_JUMBO_FRAME_SUPPORT;
            if ( 10000000 == pHardware->m_ulTransmitRate )
                data |= MISC_PORT_1000M;
            else if ( 1000000 == pHardware->m_ulTransmitRate )
                data |= MISC_PORT_100M;
            if ( 2 == pHardware->m_ulDuplex )
                data |= MISC_PORT_FD;
            data |= ( MISC_EXC_DEFER_ENABLE | MISC_RECEIVE_ENABLE );

#ifdef RCV_JUMBO_FRAME
            if ( 1 == pHardware->m_wPhyAddr )
                data |= DMA_JUMBO_FRAME_SUPPORT;
#endif
            HW_WRITE_DWORD( pHardware, REG_DMA_MISC_CFG, data );

#ifdef DBG
            DBG_PRINT( "link %d: %d, %d"NEWLINE, pHardware->m_wPhyAddr,
                ( int ) pHardware->m_ulTransmitRate,
                ( int ) pHardware->m_ulDuplex );
#endif
        }
        else
        {
#ifdef DBG
            DBG_PRINT( "link %d disconnected"NEWLINE, pHardware->m_wPhyAddr );
#endif
        }
    }
}  /* HardwareGetLinkSpeed */


/*
    HardwareSetLinkSpeed

    Description:
        This routine sets the link speed of the switch ports.

    Parameters:
        PHARDWARE pHardware
            Pointer to hardware instance.

    Return (None):
*/

static void ks8001_SetLinkSpeed (
    PHARDWARE pHardware )
{
    USHORT wConfig;
    USHORT wData;
    USHORT wLinkStatus;

    HW_READ_PHY_AUTO_NEG( pHardware, wData );
    HW_READ_PHY_LINK_STATUS( pHardware, wLinkStatus );

    wConfig = 0;
    if ( ( wLinkStatus & PHY_LINK_STATUS ) )
    {
        wConfig = wData;
    }

    wData |= PHY_AUTO_NEG_100BTX_FD | PHY_AUTO_NEG_100BTX |
        PHY_AUTO_NEG_10BT_FD | PHY_AUTO_NEG_10BT;

    /* Check if manual configuration is specified by the user. */
    if ( pHardware->m_wSpeed  ||  pHardware->m_bDuplex )
    {
        if ( 10 == pHardware->m_wSpeed )
        {
            wData &= ~( PHY_AUTO_NEG_100BTX_FD | PHY_AUTO_NEG_100BTX );
        }
        else if ( 100 == pHardware->m_wSpeed )
        {
            wData &= ~( PHY_AUTO_NEG_10BT_FD | PHY_AUTO_NEG_10BT );
        }
        if ( 1 == pHardware->m_bDuplex )
        {
            wData &= ~( PHY_AUTO_NEG_100BTX_FD | PHY_AUTO_NEG_10BT_FD );
        }
        else if ( 2 == pHardware->m_bDuplex )
        {
            wData &= ~( PHY_AUTO_NEG_100BTX | PHY_AUTO_NEG_10BT );
        }
    }
    if ( wData != wConfig )
    {
        HW_WRITE_PHY_AUTO_NEG( pHardware, wData );

        /* Restart auto-negotiation */
        HW_READ_PHY_CTRL( pHardware, wData );
        wData |= PHY_AUTO_NEG_RESTART;
        HW_WRITE_PHY_CTRL( pHardware, wData );
    }
}  /* HardwareSetLinkSpeed */


static void ks8001_SetupPhy (
    PHARDWARE pHardware )
{
#if 0
    USHORT wData;

    HardwareReadPhy( pHardware->m_wPhyAddr, PHY_REG_INT_CTRL_STATUS, &wData );
    wData |= PHY_INT_LINK_DOWN | PHY_INT_LINK_UP;
    HardwareWritePhy( pHardware->m_wPhyAddr, PHY_REG_INT_CTRL_STATUS, wData );
#endif
    pHardware->m_bLinkIntWorking = TRUE;
}  /* HardwareSetupPhy */


static struct hw_fn ks8001_hw = {
    .fnHardwareGetLinkSpeed = ks8001_GetLinkSpeed,
    .fnHardwareSetLinkSpeed = ks8001_SetLinkSpeed,
    .fnHardwareSetupPhy = ks8001_SetupPhy,

    .fnHardwareGetCableStatus = ks8001_GetCableStatus,
    .fnHardwareGetLinkStatus = ks8001_GetLinkStatus,
    .fnHardwareSetCapabilities = ks8001_SetCapabilities,
};


int ks8001_InitPhy ( void )
{
    USHORT wPhyId1, wPhyId2;
    UINT   data;
    UINT   installed = 0;
    int    i;

    data = KS_READ_REG( KS8692_STA_PHY );
    data &= ~( STA_PHY1_VALID | STA_PHY1_ADD_MASK |
        STA_PHY0_VALID | STA_PHY0_ADD_MASK );
    for ( i = 31; i >= 0; i-- ) {
        if ( HardwareReadPhy( i, PHY_REG_ID_1, &wPhyId1 ) ) {
            if ( HardwareReadPhy( i, PHY_REG_ID_2, &wPhyId2 ) ) {
                if ( wPhyId1 != PHY_8001_ID_1  ||  wPhyId2 != PHY_8001_ID_2 )
                    return FALSE;
                installed <<= 8;
                installed |= STA_PHY0_VALID | i;
            }
        }
    }
    data |= installed;
    KS_WRITE_REG( KS8692_STA_PHY, data );
    ks8692_fn = &ks8001_hw;
    return TRUE;
}  /* ks8001_InitPhy */

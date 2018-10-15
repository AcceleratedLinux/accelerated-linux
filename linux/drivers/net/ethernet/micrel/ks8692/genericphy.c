/* ---------------------------------------------------------------------------
          Copyright (c) 2007-2008 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    genericphy.c - Generic PHY functions.

    Author      Date        Description
    THa         02/13/07    Created file.
    THa         07/28/08    Use UINT instead of ULONG.
   ---------------------------------------------------------------------------
*/


#include "target.h"
#include "hardware.h"
#include "ks_phy.h"


/* -------------------------------------------------------------------------- */

#define PHY_RESET_TIMEOUT   10

/* -------------------------------------------------------------------------- */

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

static void Generic_GetCableStatus (
    PHARDWARE pHardware,
    void*     pBuffer )
{
    UINT* pulData = ( UINT* ) pBuffer;

    USHORT Data;
    UINT   ulLength[ 5 ];
    UCHAR  bStatus[ 5 ];
    int    i;

    memset( ulLength, 0, sizeof( ulLength ));
    memset( bStatus, 0, sizeof( bStatus ));
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

        goto GetCableStatusDone;
    }

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

static void Generic_GetLinkStatus (
    PHARDWARE pHardware,
    void*     pBuffer )
{
    UINT*  pulData = ( UINT* ) pBuffer;
    UINT   ulStatus;
    USHORT Capable;
    USHORT Data;
    USHORT Status;

    HW_READ_PHY_LINK_STATUS( pHardware, Capable );
    ulStatus = 0;
    if ( ( Capable & PHY_LINK_STATUS ) )
        ulStatus = 1;

    /* Link status */
    *pulData++ = ulStatus;

    HW_READ_PHY_AUTO_NEG( pHardware, Data );
    HW_READ_PHY_REM_CAP( pHardware, Status );

    ulStatus = 100000;
    if ( (( Data & PHY_AUTO_NEG_100BTX_FD )  &&
            ( Status & PHY_AUTO_NEG_100BTX_FD ))  ||
            (( Data & PHY_AUTO_NEG_100BTX )  &&
            ( Status & PHY_AUTO_NEG_100BTX )) )
        ulStatus = 1000000;

    /* Link speed */
    *pulData++ = ulStatus;

    ulStatus = 0;

    if ( (( Data & PHY_AUTO_NEG_100BTX_FD )  &&
            ( Status & PHY_AUTO_NEG_100BTX_FD ))  ||
            (( Data & PHY_AUTO_NEG_10BT_FD )  &&
            ( Status & PHY_AUTO_NEG_10BT_FD )  &&
            ( !( Data & PHY_AUTO_NEG_100BTX )  ||
            !( Status & PHY_AUTO_NEG_100BTX ))) )
        ulStatus |= STATUS_FULL_DUPLEX;

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

static void Generic_SetCapabilities (
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

static void Generic_GetLinkSpeed (
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
            if ( 10000000 == pHardware->m_ulTransmitRate )
                data |= MISC_PORT_1000M;
            else if ( 1000000 == pHardware->m_ulTransmitRate )
                data |= MISC_PORT_100M;
            if ( 2 == pHardware->m_ulDuplex )
                data |= MISC_PORT_FD;
            data |= 0x180;
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

static void Generic_SetLinkSpeed (
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


static void Generic_SetupPhy (
    PHARDWARE pHardware )
{
}  /* HardwareSetupPhy */


static struct hw_fn generic_hw = {
    .fnHardwareGetLinkSpeed = Generic_GetLinkSpeed,
    .fnHardwareSetLinkSpeed = Generic_SetLinkSpeed,
    .fnHardwareSetupPhy = Generic_SetupPhy,

    .fnHardwareGetCableStatus = Generic_GetCableStatus,
    .fnHardwareGetLinkStatus = Generic_GetLinkStatus,
    .fnHardwareSetCapabilities = Generic_SetCapabilities,
};


int Generic_InitPhy ( void )
{
    USHORT wPhyId1, wPhyId2;
    int    i;

    for ( i = 0; i < 32; i++ ) {
        if ( HardwareReadPhy( i, PHY_REG_ID_1, &wPhyId1 ) ) {
            if ( HardwareReadPhy( i, PHY_REG_ID_2, &wPhyId2 ) ) {
                printk( "%d = %04x %04x\n", i, wPhyId1, wPhyId2 );
#if 0
                if ( wPhyId1 != PHY_8001_ID_1  ||  wPhyId2 != PHY_8001_ID_2 )
                    return FALSE;
#endif
            }
        }
    }
    ks8692_fn = &generic_hw;
    return TRUE;
}  /* Generic_InitPhy */

/* ---------------------------------------------------------------------------
          Copyright (c) 2007-2008 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    vsc8201.c - Vitesse VSC8201 PHY function.

    Author      Date        Description
    THa         02/13/07    Created file.
    THa         05/30/08    Update for rev. B KSZ8692.
    THa         07/28/08    Use UINT instead of ULONG.
   ---------------------------------------------------------------------------
*/


#include "target.h"
#include "hardware.h"
#include "ks_phy.h"


#ifdef CONFIG_KSZ8692VA
#define JUMBO_FRAME_PORT  31
#define REGULAR_PORT      30

#else
#define JUMBO_FRAME_PORT  30
#define REGULAR_PORT      31
#endif

/* -------------------------------------------------------------------------- */

/* Mode Control Register */
#define PHY_SPEED_MASK              0x2040
#define PHY_SPEED_10MBIT            0x0000
#define PHY_SPEED_100MBIT           0x2000
#define PHY_SPEED_1000MBIT          0x0040

#define CTRL_RESERVED               0x003F

/* Mode Status Register */
#define PHY_100BT2_FD_CAPABLE       0x0400
#define PHY_100BT2_CAPABLE          0x0200
#define PHY_EXTENDED_STATUS         0x0100

#define STATUS_RESERVED             0x0080

/* PHY Identifier Registers */
#define PHY_VCS8201_ID_1            0x000F
#define PHY_VCS8201_ID_2            0xC413
#define PHY_VCS8201_ID_2_REV_A0     0x0001
#define PHY_VCS8201_ID_2_REV_A1     0x0002
#define PHY_VCS8201_ID_2_REV_D      0x0003

/* 1000BASE-T Control Register */
#define PHY_REG_1000BT_PHY_CTRL     9
#define PHY_TRANSMITTER_TEST_MODE   0xE000
#define PHY_MASTER_MANUAL_CFG       0x1000
#define PHY_MASTER_ENABLE           0x0800
#define PHY_MULTI_PORT_DEVICE       0x0400
#define PHY_AUTO_NEG_1000BT_FD      0x0200
#define PHY_AUTO_NEG_1000BT         0x0100

#define _1000BT_PHY_CTRL_RESERVED   0x00FF

/* 1000BASE-T Status Register */
#define PHY_REG_1000BT_PHY_STATUS   10
#define PHY_MASTER_CFG_FAULT        0x8000
#define PHY_LOCAL_MASTER            0x4000
#define PHY_LOCAL_RECEIVER_OK       0x2000
#define PHY_REMOTE_RECEIVER_OK      0x1000
#define PHY_REMOTE_1000BT_FD        0x0800
#define PHY_REMOTE_1000BT           0x0400
#define PHY_IDLE_ERROR_COUNT        0x00FF

#define _1000BT_PHY_STAT_RESERVED   0x0300

/* 1000BASE-T Status Extension Register #1 */
#define PHY_REG_1000BT_EXT_STAT_1   15
#define PHY_EXT_1000BTX_FD_CAPABLE  0x8000
#define PHY_EXT_1000BTX_CAPABLE     0x4000
#define PHY_EXT_1000BT_FD_CAPABLE   0x2000
#define PHY_EXT_1000BT_CAPABLE      0x1000

#define EXT_STAT_1_RESERVED         0x0FFF

/* 100BASE-TX Status Extension Register */
#define PHY_REG_100BTX_EXT_STATUS   16
#define PHY_EXT_DESCRAMBLER_LOCKED  0x8000
#define PHY_EXT_LOCK_ERROR          0x4000
#define PHY_EXT_LINK_DISCONNECTED   0x2000
#define PHY_EXT_LINK_ACTIVE         0x1000
#define PHY_EXT_RECEIVE_ERROR       0x0800
#define PHY_EXT_TRANSMIT_ERROR      0x0400
#define PHY_EXT_SSD_ERROR           0x0200
#define PHY_EXT_ESD_ERROR           0x0100

#define _100BTX_EXT_STAT_RESERVED   0x00FF

/* 1000BASE-T Status Extension Register #2 */
#define PHY_REG_1000BT_EXT_STAT_2   17
#define PHY_EXT_DESCRAMBLER_LOCKED  0x8000
#define PHY_EXT_LOCK_ERROR          0x4000
#define PHY_EXT_LINK_DISCONNECTED   0x2000
#define PHY_EXT_LINK_ACTIVE         0x1000
#define PHY_EXT_RECEIVE_ERROR       0x0800
#define PHY_EXT_TRANSMIT_ERROR      0x0400
#define PHY_EXT_SSD_ERROR           0x0200
#define PHY_EXT_ESD_ERROR           0x0100
#define PHY_EXT_CARRIER_ERROR       0x0080
#define PHY_EXT_NON_COMP_BCM5400    0x0040

#define EXT_STAT_2_RESERVED         0x003F

/* Bypass Control Register */
#define PHY_REG_BYPASS_CTRL         18
#define PHY_TRANSMIT_DISABLE        0x8000
#define PHY_BYPASS_4B5B             0x4000
#define PHY_BYPASS_SCRAMBLER        0x2000
#define PHY_BYPASS_DESCRAMBLER      0x1000
#define PHY_BYPASS_PCS_RECEIVE      0x0800
#define PHY_BYPASS_PCS_TRANSMIT     0x0400
#define PHY_BYPASS_LFI_TIMER        0x0200
#define PHY_TRANSMITTER_TEST_CLOCK  0x0100
#define PHY_FORCE_NON_COMP_BCM5400  0x0080
#define PHY_BYPASS_NON_COMP_BCM5400 0x0040
#define PHY_AUTO_PAIR_SWAP_DISABLE  0x0020
#define PHY_POLARITY_DISABLE        0x0010
#define PHY_PARALLEL_DETECT_CTRL    0x0008
#define PHY_PULSE_FILTER_DISABLE    0x0004
#define PHY_AUTO_NEXT_PAGE_DISABLE  0x0002
#define PHY_125MHZ_CLOCK_ENABLE     0x0001

/* Receive Error Counter Register */
#define PHY_REG_RXER_COUNTER        19
#define RXER_COUNTER_RESERVED       0xFF00

/* False Carrier Sense Counter Register */
#define PHY_REG_FCSER_COUNTER       20
#define FCSER_COUNTER_RESERVED      0xFF00

/* Disconnect Counter Register */
#define PHY_REG_DISC_COUNTER        21
#define DISC_COUNTER_RESERVED       0xFF00

/* 10BASE-T Control & Status Register */
#define PHY_REG_10BT_CTRL_STATUS    22
#define PHY_LINK_TEST_DISABLE       0x8000
#define PHY_JABBER_DETECT_DISABLE   0x4000
#define PHY_ECHO_MODE_DISABLE       0x2000
#define PHY_SQE_TX_DISABLE          0x1000

#define PHY_SQUELCH_CTRL            0x0C00
#define PHY_SQUELCH_300MV           0x0000
#define PHY_SQUELCH_197MV           0x0400
#define PHY_SQUELCH_450MV           0x0800

#define PHY_EOF_ERROR_DETECTED      0x0100
#define PHY_10BT_LINK_DISCONNETED   0x0080
#define PHY_10BT_LINK_ACTIVE        0x0040

#define PHY_CURRENT_REF_TRIM        0x0038
#define PHY_CURRENT_PLUS_6_         0x0018
#define PHY_CURRENT_PLUS_6          0x0010
#define PHY_CURRENT_PLUS_4          0x0008
#define PHY_CURRENT_PLUS_2          0x0000
#define PHY_CURRENT_ZERO            0x0038
#define PHY_CURRENT_MINUS_2         0x0030
#define PHY_CURRENT_MINUS_4         0x0028
#define PHY_CURRENT_MINUS_4_        0x0020

#define _10BT_CTRL_STATUS_RESERVED  0x0027

/* Extended PHY Control Register #1 */
#define PHY_REG_EXT_PHY_CTRL_1      23
#define PHY_MAC_INTERFACE_MODE      0xF000
#define PHY_GMII_MII_ENABLE         0x0000
#define PHY_RGMII_ENABLE            0x1000
#define PHY_TBI_ENABLE              0x2000
#define PHY_RTBI_ENABLE             0x3000

#define PHY_MAC_INTERFACE_POWER     0x0E00
#define PHY_POWER_3_3V              0x0000
#define PHY_POWER_2_5V              0x0200

#define PHY_RGMII_SKEW_TIMING_COMP  0x0100
#define PHY_EWRAP_ENABLE            0x0080
#define PHY_TBI_BIT_ORDER_REVERSAL  0x0040
#define PHY_ACTIPHY_ENABLE          0x0020
#define PHY_GMII_TRANSMIT_REVERSAL  0x0002

#define EXT_PHY_CTRL_1_RESERVED     0x0015

/* Extended PHY Control Register #2 */
#define PHY_REG_EXT_PHY_CTRL_2      24
#define PHY_EDGE_RATE_CTRL          0xE000
#define PHY_EDGE_RATE_PLUS_3        0x6000
#define PHY_EDGE_RATE_PLUS_2        0x4000
#define PHY_EDGE_RATE_PLUS_1        0x2000
#define PHY_EDGE_RATE_NOMINAL       0x0000
#define PHY_EDGE_RATE_MINUS_1       0xE000
#define PHY_EDGE_RATE_MINUS_2       0xC000
#define PHY_EDGE_RATE_MINUS_3       0xA000
#define PHY_EDGE_RATE_MINUS_4       0x8000

#define PHY_TRANSMIT_VOLT_REF_TRIM  0x1C00
#define PHY_TX_VOLT_PLUS_6_         0x0C00
#define PHY_TX_VOLT_PLUS_6          0x0800
#define PHY_TX_VOLT_PLUS_4          0x0400
#define PHY_TX_VOLT_PLUS_2          0x0000
#define PHY_TX_VOLT_ZERO            0x1C00
#define PHY_TX_VOLT_MINUS_2         0x1800
#define PHY_TX_VOLT_MINUS_4         0x1400
#define PHY_TX_VOLT_MINUS_4_        0x1000

#define PHY_TX_FIFO_DEPTH_CTRL      0x0380
#define PHY_TX_FIFO_5_SYMBOLS       0x0000
#define PHY_TX_FIFO_4_SYMBOLS       0x0080
#define PHY_TX_FIFO_3_SYMBOLS       0x0100
#define PHY_TX_FIFO_2_SYMBOLS       0x0180
#define PHY_TX_FIFO_1_SYMBOL        0x0200

#define PHY_RX_FIFO_DEPTH_CTRL      0x0070
#define PHY_RX_FIFO_5_SYMBOLS       0x0000
#define PHY_RX_FIFO_4_SYMBOLS       0x0010
#define PHY_RX_FIFO_3_SYMBOLS       0x0020
#define PHY_RX_FIFO_2_SYMBOLS       0x0030
#define PHY_RX_FIFO_1_SYMBOL        0x0040

#define PHY_CABLE_QUALITY_STATUS    0x000E
#define PHY_CABLE_LEN_LT_10M        0x0000
#define PHY_CABLE_LEN_LT_20M        0x0002
#define PHY_CABLE_LEN_LT_40M        0x0004
#define PHY_CABLE_LEN_LT_80M        0x0006
#define PHY_CABLE_LEN_LT_100M       0x0008
#define PHY_CABLE_LEN_LT_140M       0x000A
#define PHY_CABLE_LEN_LT_180M       0x000C
#define PHY_CABLE_LEN_GT_180M       0x000E

#define PHY_ANALOG_LOOPBACK_ENABLE  0x0001

/* Interrupt Mask Register */
#define PHY_REG_INT_CTRL            25
/* Interrupt Status Register */
#define PHY_REG_INT_STATUS          26
#define PHY_INT_PIN                 0x8000
#define PHY_INT_SPEED_CHANGE        0x4000
#define PHY_INT_LINK_CHANGE         0x2000
#define PHY_INT_DUPLEX_CHANGE       0x1000
#define PHY_INT_AUTO_NEG_ERROR      0x0800
#define PHY_INT_LINK_PARTNER_ACK    0x0400
#define PHY_INT_PAGE_RCV            0x0200
#define PHY_INT_SYMBOL_ERROR        0x0100
#define PHY_INT_DESCRAMBLER_LOST    0x0080
#define PHY_INT_CROSSOVER_CHANGE    0x0040
#define PHY_INT_POLARITY_CHANGE     0x0020
#define PHY_INT_JABBER              0x0010
#define PHY_INT_FALSE_CARRIER       0x0008
#define PHY_INT_PAR_DETECT_FAULT    0x0004
#define PHY_INT_MASTER_SLAVE        0x0002
#define PHY_INT_RCV_ERROR           0x0001

/* Parallel LED Control Register */
#define PHY_REG_PARALLEL_LED        27
#define PHY_LED_LINK10_ON           0x8000
#define PHY_LED_LINK10_DISABLE      0x4000
#define PHY_LED_LINK100_ON          0x2000
#define PHY_LED_LINK100_DISABLE     0x1000
#define PHY_LED_LINK1000_ON         0x0800
#define PHY_LED_LINK1000_DISABLE    0x0400
#define PHY_LED_DUPLEX_ON           0x0200
#define PHY_LED_DUPLEX_DISABLE      0x0100
#define PHY_LED_ACTIVITY_ON         0x0080
#define PHY_LED_ACTIVITY_DISABLE    0x0040
#define PHY_LED_QUALITY_ON          0x0020
#define PHY_LED_QUALITY_DISABLE     0x0010
#define PHY_LED_PULSE_ENABLE        0x0008
#define PHY_LED_BLINK_ENABLE        0x0004
#define PHY_LED_BLINK_RATE_10HZ     0x0002

#define PARALLEL_LED_RESERVED       0x0001

/* Auxiliary Control & Status Register */
#define PHY_REG_AUX_CTRL_STATUS     28
#define PHY_AUTO_NEG_COMPLETE       0x8000
#define PHY_AUTO_NEG_DISABALED      0x4000
#define PHY_STAT_MDIX               0x2000
#define PHY_CD_PAIR_SWAP            0x1000
#define PHY_STAT_INV_POLARITY_A     0x0800
#define PHY_STAT_INV_POLARITY_B     0x0400
#define PHY_STAT_INV_POLARITY_C     0x0200
#define PHY_STAT_INV_POLARITY_D     0x0100
#define PHY_STAT_FULL_DUPLEX        0x0020

#define PHY_STAT_SPEED_MASK         0x0018
#define PHY_IN_10BT                 0x0000
#define PHY_IN_100BTX               0x0008
#define PHY_IN_1000BT               0x0010

#define PHY_SMI_PRIORITY_SELECT     0x0004
#define PHY_RESET_CONTROL           0x0002

#define AUX_CTRL_STATUS_RESERVED    0x00C1

/* Delay Skew Status Register */
#define PHY_REG_DELAY_SKEW_STATUS   29
#define PHY_DELAY_SKEW_A            0x7000
#define PHY_DELAY_SKEW_B            0x0700
#define PHY_DELAY_SKEW_C            0x0070
#define PHY_DELAY_SKEW_D            0x0007

#define DELAY_SKEW_STATUS_RESERVED  0x8888


#define PHY_RESET_TIMEOUT   10


#define HW_READ_PHY_EXT_AUTO_NEG( phwi, data )                              \
    HardwareReadPhy(( phwi )->m_wPhyAddr, PHY_REG_1000BT_PHY_CTRL, &data )

#define HW_WRITE_PHY_EXT_AUTO_NEG( phwi, data )                             \
    HardwareWritePhy(( phwi )->m_wPhyAddr, PHY_REG_1000BT_PHY_CTRL, data )

#define HW_READ_PHY_POLARITY( phwi, data )                                  \
    HardwareReadPhy(( phwi )->m_wPhyAddr, PHY_REG_AUX_CTRL_STATUS, &data )

#define HW_READ_PHY_EXT_REM_CAP( phwi, data )                               \
    HardwareReadPhy(( phwi )->m_wPhyAddr, PHY_REG_1000BT_PHY_STATUS, &data )

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

static void vcs8201_GetCableStatus (
    PHARDWARE pHardware,
    void*     pBuffer )
{
    UINT*  pulData = ( UINT* ) pBuffer;

    USHORT Data;
    UINT   ulStatus;
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

        HW_READ_PHY_POLARITY( pHardware, Data );
        ulStatus = 0;
        if ( ( Data & PHY_STAT_MDIX ) )
            ulStatus |= STATUS_CROSSOVER;
        if ( ( Data & ( PHY_STAT_INV_POLARITY_A |
                PHY_STAT_INV_POLARITY_B |
                PHY_STAT_INV_POLARITY_C |
                PHY_STAT_INV_POLARITY_D )) )
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

static void vcs8201_GetLinkStatus (
    PHARDWARE pHardware,
    void*     pBuffer )
{
    UINT*  pulData = ( UINT* ) pBuffer;
    UINT   ulStatus;
    USHORT Capable;
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

    HW_READ_PHY_POLARITY( pHardware, Polarity );

    ulStatus = 100000;
    if ( ( Polarity & PHY_STAT_SPEED_MASK ) == PHY_IN_100BTX )
        ulStatus = 1000000;
    if ( ( Polarity & PHY_STAT_SPEED_MASK ) == PHY_IN_1000BT )
        ulStatus = 10000000;

    /* Link speed */
    *pulData++ = ulStatus;

    ulStatus = 0;

    if ( ( Polarity & PHY_STAT_FULL_DUPLEX ) )
        ulStatus |= STATUS_FULL_DUPLEX;

    if ( ( Polarity & PHY_STAT_MDIX ) )
        ulStatus |= STATUS_CROSSOVER;

    if ( ( Polarity & (
            PHY_STAT_INV_POLARITY_A |
            PHY_STAT_INV_POLARITY_B |
            PHY_STAT_INV_POLARITY_C |
            PHY_STAT_INV_POLARITY_D )) )
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

    HW_READ_PHY_EXT_REM_CAP( pHardware, Capable );
    if ( ( Capable & PHY_EXT_1000BT_FD_CAPABLE ) )
        ulStatus |= LINK_1GBPS_FULL;
    if ( ( Capable & PHY_EXT_1000BT_CAPABLE ) )
        ulStatus |= LINK_1GBPS_HALF;

    ulStatus |= LINK_SYM_PAUSE;
    ulStatus |= LINK_ASYM_PAUSE;

    /* Capability */
    *pulData++ = ulStatus;

    ulStatus = 0;
    if ( ( Data & PHY_AUTO_NEG_ASYM_PAUSE ) )
        ulStatus |= LINK_ASYM_PAUSE;
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

    HW_READ_PHY_EXT_AUTO_NEG( pHardware, Data );
    if ( ( Data & PHY_AUTO_NEG_1000BT_FD ) )
        ulStatus |= LINK_1GBPS_FULL;
    if ( ( Data & PHY_AUTO_NEG_1000BT ) )
        ulStatus |= LINK_1GBPS_HALF;

    ulStatus |= LINK_AUTO_MDIX;
    ulStatus |= LINK_AUTO_POLARITY;

    /* Auto-Negotiation advertisement */
    *pulData++ = ulStatus;

    ulStatus = 0;
    if ( ( Status & PHY_AUTO_NEG_ASYM_PAUSE ) )
        ulStatus |= LINK_ASYM_PAUSE;
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

    HW_READ_PHY_EXT_REM_CAP( pHardware, Status );
    if ( ( Status & PHY_REMOTE_1000BT_FD ) )
        ulStatus |= LINK_1GBPS_FULL;
    if ( ( Status & PHY_REMOTE_1000BT ) )
        ulStatus |= LINK_1GBPS_HALF;

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

static void vcs8201_SetCapabilities (
    PHARDWARE pHardware,
    UINT      ulCapabilities )
{
    UINT   InterruptMask;
    USHORT Data;
    int    cnTimeOut = PHY_RESET_TIMEOUT;

    InterruptMask = HardwareBlockInterrupt( pHardware );
    HardwareDisable( pHardware );

    HW_READ_PHY_AUTO_NEG( pHardware, Data );

    if ( ( ulCapabilities & LINK_ASYM_PAUSE ) )
        Data |= PHY_AUTO_NEG_ASYM_PAUSE;
    else
        Data &= ~PHY_AUTO_NEG_ASYM_PAUSE;
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

    HW_READ_PHY_EXT_AUTO_NEG( pHardware, Data );

    if ( ( ulCapabilities & LINK_1GBPS_FULL ) )
        Data |= PHY_AUTO_NEG_1000BT_FD;
    else
        Data &= ~PHY_AUTO_NEG_1000BT_FD;
    if ( ( ulCapabilities & LINK_1GBPS_HALF ) )
        Data |= PHY_AUTO_NEG_1000BT;
    else
        Data &= ~PHY_AUTO_NEG_1000BT;
    HW_WRITE_PHY_EXT_AUTO_NEG( pHardware, Data );

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

static void vcs8201_GetLinkSpeed (
    PHARDWARE pHardware )
{
    USHORT wLinkStatus;
    USHORT wLocal;
    USHORT wPolarity;
    USHORT wRemote;
    UINT   data;
    int    change = FALSE;

    HW_READ_PHY_LINK_STATUS( pHardware, wLinkStatus );
    HW_READ_PHY_POLARITY( pHardware, wPolarity );
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
            if ( ( wPolarity & PHY_STAT_SPEED_MASK ) == PHY_IN_100BTX )
                pHardware->m_ulTransmitRate = 1000000;
            if ( ( wPolarity & PHY_STAT_SPEED_MASK ) == PHY_IN_1000BT )
                pHardware->m_ulTransmitRate = 10000000;

            pHardware->m_ulDuplex = 1;
            if ( ( wPolarity & PHY_STAT_FULL_DUPLEX ) )
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

#ifndef CONFIG_KSZ8692_MII
#ifdef CONFIG_KSZ8692VA
            if ( JUMBO_FRAME_PORT == pHardware->m_wPhyAddr )
#endif
            {
                data |= MISC_RGMII_SELECT;

#ifdef CONFIG_KSZ8692VB
                data |= MISC_RGMII_TX_DELAY | MISC_RGMII_RX_DELAY;
#endif
            }
#endif

#ifdef RCV_JUMBO_FRAME
            if ( JUMBO_FRAME_PORT == pHardware->m_wPhyAddr )
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

static void vcs8201_SetLinkSpeed (
    PHARDWARE pHardware )
{
    USHORT wConfig;
    USHORT wData;
    USHORT wExt;
    USHORT wExtCfg;
    USHORT wLinkStatus;

    HW_READ_PHY_AUTO_NEG( pHardware, wData );
    HW_READ_PHY_EXT_AUTO_NEG( pHardware, wExt );
    HW_READ_PHY_LINK_STATUS( pHardware, wLinkStatus );

    wConfig = 0;
    wExtCfg = 0;
    if ( ( wLinkStatus & PHY_LINK_STATUS ) )
    {
        wConfig = wData;
        wExtCfg = wExt;
    }

    wData |= PHY_AUTO_NEG_100BTX_FD | PHY_AUTO_NEG_100BTX |
        PHY_AUTO_NEG_10BT_FD | PHY_AUTO_NEG_10BT;
    wExt |= PHY_AUTO_NEG_1000BT_FD | PHY_AUTO_NEG_1000BT;

#ifdef CONFIG_KSZ8692VA
/* LAN port gigabit connection not working. */
    if ( 30 == pHardware->m_wPhyAddr )
        wExt &= ~( PHY_AUTO_NEG_1000BT_FD | PHY_AUTO_NEG_1000BT );
#endif

    /* Check if manual configuration is specified by the user. */
    if ( pHardware->m_wSpeed  ||  pHardware->m_bDuplex )
    {
        if ( 10 == pHardware->m_wSpeed )
        {
            wData &= ~( PHY_AUTO_NEG_100BTX_FD | PHY_AUTO_NEG_100BTX );
            wExt &= ~( PHY_AUTO_NEG_1000BT_FD | PHY_AUTO_NEG_1000BT );
        }
        else if ( 100 == pHardware->m_wSpeed )
        {
            wData &= ~( PHY_AUTO_NEG_10BT_FD | PHY_AUTO_NEG_10BT );
            wExt &= ~( PHY_AUTO_NEG_1000BT_FD | PHY_AUTO_NEG_1000BT );
        }
        else if ( 1000 == pHardware->m_wSpeed )
        {
            wData &= ~( PHY_AUTO_NEG_10BT_FD | PHY_AUTO_NEG_10BT );
            wData &= ~( PHY_AUTO_NEG_100BTX_FD | PHY_AUTO_NEG_100BTX );
        }
        if ( 1 == pHardware->m_bDuplex )
        {
            wData &= ~( PHY_AUTO_NEG_100BTX_FD | PHY_AUTO_NEG_10BT_FD );
            wExt &= ~( PHY_AUTO_NEG_1000BT_FD );
        }
        else if ( 2 == pHardware->m_bDuplex )
        {
            wData &= ~( PHY_AUTO_NEG_100BTX | PHY_AUTO_NEG_10BT );
            wExt &= ~( PHY_AUTO_NEG_1000BT );
        }
    }
    if ( wData != wConfig  ||  wExt != wExtCfg )
    {
        HW_WRITE_PHY_AUTO_NEG( pHardware, wData );
        HW_WRITE_PHY_EXT_AUTO_NEG( pHardware, wExt );

        /* Restart auto-negotiation */
        HW_READ_PHY_CTRL( pHardware, wData );
        wData |= PHY_AUTO_NEG_RESTART;
        HW_WRITE_PHY_CTRL( pHardware, wData );
    }
}  /* HardwareSetLinkSpeed */


static void vcs8201_SetupPhy (
    PHARDWARE pHardware )
{
    USHORT wData;

#if 0
    HardwareReadPhy( pHardware->m_wPhyAddr, PHY_REG_INT_CTRL,
        &wData );
#if 0
    wData |= PHY_INT_PIN | PHY_INT_LINK_CHANGE | PHY_INT_SPEED_CHANGE |
        PHY_INT_DUPLEX_CHANGE;
#else
    wData |= PHY_INT_PIN | PHY_INT_LINK_CHANGE;
#endif
    HardwareWritePhy( pHardware->m_wPhyAddr, PHY_REG_INT_CTRL,
        wData );
#endif
    HardwareReadPhy( pHardware->m_wPhyAddr, PHY_REG_AUX_CTRL_STATUS, &wData );
    wData |= PHY_SMI_PRIORITY_SELECT;
    HardwareWritePhy( pHardware->m_wPhyAddr, PHY_REG_AUX_CTRL_STATUS, wData );

#ifndef CONFIG_KSZ8692_MII
#ifdef CONFIG_KSZ8692VA
    if ( JUMBO_FRAME_PORT == pHardware->m_wPhyAddr )
#endif
    {
        HardwareReadPhy( pHardware->m_wPhyAddr, PHY_REG_EXT_PHY_CTRL_1,
            &wData );
        wData &= ~PHY_MAC_INTERFACE_MODE;
        wData |= PHY_RGMII_ENABLE;
        HardwareWritePhy( pHardware->m_wPhyAddr, PHY_REG_EXT_PHY_CTRL_1,
            wData );
    }
#endif
    pHardware->m_bLinkIntWorking = TRUE;
}  /* HardwareSetupPhy */


static struct hw_fn vsc8201_hw = {
    .fnHardwareGetLinkSpeed = vcs8201_GetLinkSpeed,
    .fnHardwareSetLinkSpeed = vcs8201_SetLinkSpeed,
    .fnHardwareSetupPhy = vcs8201_SetupPhy,

    .fnHardwareGetCableStatus = vcs8201_GetCableStatus,
    .fnHardwareGetLinkStatus = vcs8201_GetLinkStatus,
    .fnHardwareSetCapabilities = vcs8201_SetCapabilities,
};


int vsc8201_InitPhy ( void )
{
    USHORT wPhyId1, wPhyId2;
    UINT   data;
    UINT   installed = 0;
    int    i;

    data = KS_READ_REG( KS8692_STA_PHY );
    data &= ~( STA_PHY1_VALID | STA_PHY1_ADD_MASK |
        STA_PHY0_VALID | STA_PHY0_ADD_MASK );
    for ( i = 0; i < 32; i++ ) {
        if ( HardwareReadPhy( i, PHY_REG_ID_1, &wPhyId1 ) ) {
            if ( HardwareReadPhy( i, PHY_REG_ID_2, &wPhyId2 ) ) {
                if ( wPhyId1 != PHY_VCS8201_ID_1  ||
                        wPhyId2 != PHY_VCS8201_ID_2 ) {
                    if ( !installed )
                        return FALSE;
                    else
                        printk( " PHY %d not detected: %04X %04X\n",
                            i, wPhyId1, wPhyId2 );
                }
                installed <<= 8;
                installed |= STA_PHY0_VALID | i;
            }
        }
    }
    data |= installed;
    KS_WRITE_REG( KS8692_STA_PHY, data );
    ks8692_fn = &vsc8201_hw;
    return TRUE;
}  /* vsc8201_InitPhy */

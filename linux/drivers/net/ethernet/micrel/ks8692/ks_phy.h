/* ---------------------------------------------------------------------------
             Copyright (c) 2007 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    ks_phy.h - Generic PHY header.

    Author      Date        Description
    THa         02/13/07    Created file.
   ---------------------------------------------------------------------------
*/


#ifndef __KS_PHY_H
#define __KS_PHY_H


/* Mode Control Register */
#define PHY_REG_CTRL                0
#define PHY_RESET                   0x8000
#define PHY_LOOPBACK                0x4000
#define PHY_AUTO_NEG_ENABLE         0x1000
#define PHY_POWER_DOWN              0x0800
#define PHY_MII_DISABLE             0x0400
#define PHY_AUTO_NEG_RESTART        0x0200
#define PHY_FULL_DUPLEX             0x0100
#define PHY_COLLISION_TEST          0x0080

/* Mode Status Register */
#define PHY_REG_STATUS              1
#define PHY_100BT4_CAPABLE          0x8000
#define PHY_100BTX_FD_CAPABLE       0x4000
#define PHY_100BTX_CAPABLE          0x2000
#define PHY_10BT_FD_CAPABLE         0x1000
#define PHY_10BT_CAPABLE            0x0800
#define PHY_MII_SUPPRESS_CAPABLE    0x0040
#define PHY_AUTO_NEG_ACKNOWLEDGE    0x0020
#define PHY_REMOTE_FAULT            0x0010
#define PHY_AUTO_NEG_CAPABLE        0x0008
#define PHY_LINK_STATUS             0x0004
#define PHY_JABBER_DETECT           0x0002
#define PHY_EXTENDED_CAPABILITY     0x0001

/* PHY Identifier Registers */
#define PHY_REG_ID_1                2
#define PHY_REG_ID_2                3

/* Auto-Negotiation Advertisement Register */
#define PHY_REG_AUTO_NEGOTIATION    4
/* Auto-Negotiation Link Partner Ability Register */
#define PHY_REG_REMOTE_CAPABILITY   5

#define PHY_AUTO_NEG_NEXT_PAGE      0x8000
#define PHY_ACKNOWLEDGE             0x4000
#define PHY_AUTO_NEG_REMOTE_FAULT   0x2000
#define PHY_AUTO_NEG_ASYM_PAUSE     0x0800
#define PHY_AUTO_NEG_SYM_PAUSE      0x0400
#define PHY_AUTO_NEG_100BT4         0x0200
#define PHY_AUTO_NEG_100BTX_FD      0x0100
#define PHY_AUTO_NEG_100BTX         0x0080
#define PHY_AUTO_NEG_10BT_FD        0x0040
#define PHY_AUTO_NEG_10BT           0x0020

#define PHY_AUTO_NEG_SELECTOR       0x001F
#define PHY_AUTO_NEG_802_3          0x0001

#define AUTO_NEGOTIATION_RESERVED   0x5000
#define REMOTE_CAPABILITY_RESERVED  0x1000

#define PHY_LINK_SUPPORT  (     \
    PHY_AUTO_NEG_ASYM_PAUSE |   \
    PHY_AUTO_NEG_SYM_PAUSE |    \
    PHY_AUTO_NEG_100BT4 |       \
    PHY_AUTO_NEG_100BTX_FD |    \
    PHY_AUTO_NEG_100BTX |       \
    PHY_AUTO_NEG_10BT_FD |      \
    PHY_AUTO_NEG_10BT )

/* Auto-Negotiation Expansion Register */
#define PHY_REG_AUTO_NEG_EXPANSION  6
#define PHY_PARALLEL_DETECT_FAULT   0x0010
#define PHY_REMOTE_NEXT_PAGE_ABLE   0x0008
#define PHY_LOCAL_NEXT_PAGE_ABLE    0x0004
#define PHY_PAGE_RECEIVED           0x0002
#define PHY_REMOTE_AUTO_NEG_ABLE    0x0001

#define AUTO_NEG_EXP_RESERVED       0xFFE0

/* Auto-Negotiation Next-Page Transmit Register */
#define PHY_REG_AUTO_NEG_NEXT_PAGE  7
/* Auto-Negotiation Link Partner Next-Page Receive Register */
#define PHY_REG_REMOTE_NEXT_PAGE    8

#define PHY_NEXT_PAGE               0x8000
#define PHY_ACKNOWLEDGE             0x4000
#define PHY_MESSAGE_PAGE            0x2000
#define PHY_ACKNOWLEDGE_2           0x1000
#define PHY_TOGGLE                  0x0800
#define PHY_MESSAGE_FIELD           0x07FF

#define AUTO_NEG_NEXT_PAGE_RESERVED 0x4000


#define HW_READ_PHY_CTRL( phwi, data )                                      \
    HardwareReadPhy(( phwi )->m_wPhyAddr, PHY_REG_CTRL, &data )

#define HW_WRITE_PHY_CTRL( phwi, data )                                     \
    HardwareWritePhy(( phwi )->m_wPhyAddr, PHY_REG_CTRL, data )

#define HW_READ_PHY_LINK_STATUS( phwi, data )                               \
    HardwareReadPhy(( phwi )->m_wPhyAddr, PHY_REG_STATUS, &data )

#define HW_READ_PHY_AUTO_NEG( phwi, data )                                  \
    HardwareReadPhy(( phwi )->m_wPhyAddr, PHY_REG_AUTO_NEGOTIATION, &data )

#define HW_WRITE_PHY_AUTO_NEG( phwi, data )                                 \
    HardwareWritePhy(( phwi )->m_wPhyAddr, PHY_REG_AUTO_NEGOTIATION, data )

#define HW_READ_PHY_REM_CAP( phwi, data )                                   \
    HardwareReadPhy(( phwi )->m_wPhyAddr, PHY_REG_REMOTE_CAPABILITY, &data )


enum {
    CABLE_UNKNOWN,
    CABLE_GOOD,
    CABLE_CROSSED,
    CABLE_REVERSED,
    CABLE_CROSSED_REVERSED,
    CABLE_OPEN,
    CABLE_SHORT
};


#define STATUS_FULL_DUPLEX  0x01
#define STATUS_CROSSOVER    0x02
#define STATUS_REVERSED     0x04

#define LINK_10MBPS_FULL    0x00000001
#define LINK_10MBPS_HALF    0x00000002
#define LINK_100MBPS_FULL   0x00000004
#define LINK_100MBPS_HALF   0x00000008
#define LINK_1GBPS_FULL     0x00000010
#define LINK_1GBPS_HALF     0x00000020
#define LINK_10GBPS_FULL    0x00000040
#define LINK_10GBPS_HALF    0x00000080
#define LINK_SYM_PAUSE      0x00000100
#define LINK_ASYM_PAUSE     0x00000200

#define LINK_AUTO_MDIX      0x00010000
#define LINK_MDIX           0x00020000
#define LINK_AUTO_POLARITY  0x00040000


#endif

/* ---------------------------------------------------------------------------
          Copyright (c) 2007-2008 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

    device_ethtool.c - Linux network device driver ethtool support.

    Author      Date        Description
    THa         02/19/07    Created file.
    THa         03/09/07    Add ethtool hardware checksum functions.
    THa         11/30/07    Correct MIB counter reporting.
    THa         07/17/08    Add Access Control List (ACL) support.
   ---------------------------------------------------------------------------
*/


#define EEPROM_SIZE  0x40

static UINT eeprom_data[ EEPROM_SIZE ] = { 0 };


#define ADVERTISED_ALL  (       \
    ADVERTISED_1000baseT_Half | \
    ADVERTISED_1000baseT_Full | \
    ADVERTISED_10baseT_Half |   \
    ADVERTISED_10baseT_Full |   \
    ADVERTISED_100baseT_Half |  \
    ADVERTISED_100baseT_Full )


/* These functions use the MII functions in mii.c. */
#ifdef MII
/*
    netdev_get_settings

    Description:
        This function queries the PHY and returns its state in the ethtool
        command.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_cmd* cmd
            Pointer to ethtool command.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_get_settings (
    struct net_device  *dev,
    struct ethtool_cmd *cmd )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    int              rc;

    if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return rc;
    }
    mii_ethtool_gset( &priv->mii_if, cmd );
    ReleaseHardware( hw_priv, FALSE );

    /* Save advertised settings for workaround in next function. */
    priv->advertising = cmd->advertising;
    return 0;
}  /* netdev_get_settings */


/*
    netdev_set_settings

    Description:
        This function sets the PHY according to the ethtool command.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_cmd* cmd
            Pointer to ethtool command.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_set_settings (
    struct net_device  *dev,
    struct ethtool_cmd *cmd )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              rc;

    /* ethtool utility does not change advertised setting if auto negotiation
       is not specified explicitly.
    */
    if ( cmd->autoneg  &&  priv->advertising == cmd->advertising ) {
        cmd->advertising |= ADVERTISED_ALL;
        if ( 10 == cmd->speed )
            cmd->advertising &=
                ~( ADVERTISED_100baseT_Full | ADVERTISED_100baseT_Half |
                ADVERTISED_1000baseT_Full | ADVERTISED_1000baseT_Half );
        else if ( 100 == cmd->speed )
            cmd->advertising &=
                ~( ADVERTISED_10baseT_Full | ADVERTISED_10baseT_Half |
                ADVERTISED_1000baseT_Full | ADVERTISED_1000baseT_Half );
        else if ( 1000 == cmd->speed )
            cmd->advertising &=
                ~( ADVERTISED_10baseT_Full | ADVERTISED_10baseT_Half |
                ADVERTISED_100baseT_Full | ADVERTISED_100baseT_Half );
        if ( 0 == cmd->duplex )
            cmd->advertising &=
                ~( ADVERTISED_100baseT_Full | ADVERTISED_10baseT_Full |
                ADVERTISED_1000baseT_Full );
        else if ( 1 == cmd->duplex )
            cmd->advertising &=
                ~( ADVERTISED_100baseT_Half | ADVERTISED_10baseT_Half |
                ADVERTISED_1000baseT_Half );
    }
    if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return rc;
    }
    if ( ( cmd->advertising & ADVERTISED_ALL ) == ADVERTISED_ALL ) {
        pHardware->m_bDuplex = 0;
        pHardware->m_wSpeed = 0;
    }
    else {
        pHardware->m_bDuplex = cmd->duplex + 1;
        pHardware->m_wSpeed = cmd->speed;
    }
    rc = mii_ethtool_sset( &priv->mii_if, cmd );
    ReleaseHardware( hw_priv, FALSE );
    return rc;
}  /* netdev_set_settings */


/*
    netdev_nway_reset

    Description:
        This function restarts the PHY for auto-negotiation.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_nway_reset (
    struct net_device *dev )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    int              rc;

    if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return rc;
    }
    rc = mii_nway_restart( &priv->mii_if );
    ReleaseHardware( hw_priv, FALSE );
    return rc;
}  /* netdev_nway_reset */


/*
    netdev_get_link

    Description:
        This function gets the link status from the PHY.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        True if PHY is linked and false otherwise.
*/

static u32 netdev_get_link (
    struct net_device *dev )
{
    struct dev_priv* priv = netdev_priv(dev);
    int              rc;

    rc = mii_link_ok( &priv->mii_if );
    return rc;
}  /* netdev_get_link */
#endif


/*
    netdev_get_drvinfo

    Description:
        This procedure returns the driver information.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_drvinfo* info
            Pointer to ethtool driver info data structure.

    Return (None):
*/

static void netdev_get_drvinfo (
    struct net_device      *dev,
    struct ethtool_drvinfo *info )
{
#if 0
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
#endif

    strcpy( info->driver, DRV_NAME );
    strcpy( info->version, DRV_VERSION );
    strcpy( info->bus_info, "AMBA" );
}  /* netdev_get_drvinfo */


/*
    netdev_get_regs_len

    Description:
        This function returns the length of the register dump.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Length of the register dump.
*/

static struct hw_regs {
    int start;
    int end;
} hw_regs_range[] = {
    { 0x0, 0x68 },
    { 0xE0, 0xEC },
    { 0x100, 0x10C },
    { 0x200, 0x204 },
    { 0x300, 0x310 },
    { 0x10000, 0x10058 },
    { 0x12000, 0x12058 },
    { 0x12100, 0x12228 },
    { 0x14000, 0x14044 },
    { 0x16000, 0x16040 },
    { 0x18000, 0x18040 },
    { 0x1E000, 0x1E024 },
    { 0x1E080, 0x1E0A4 },
    { 0x1E100, 0x1E124 },
    { 0x1E180, 0x1E1A4 },
    { 0x1E200, 0x1E270 },
    { 0x1E400, 0x1E410 },
    { 0x1E600, 0x1E610 },
    { 0x1E800, 0x1E824 },
    { 0x1E900, 0x1E920 },
    { 0x1EA00, 0x1EA60 },
    { 0x1EB00, 0x1EB10 },
    { 0, 0 }
};

static int netdev_get_regs_len (
    struct net_device *dev )
{
    struct hw_regs* pRange = hw_regs_range;
    int             len;
    int             num;
    int             regs_len;

    regs_len = 0;
    while ( pRange->end > pRange->start ) {
	num = ( pRange->end - pRange->start + 3 ) / 4;
	num = ( num + 3 ) / 4;
	len = ( 4 * 9 + 6 ) * num;
	regs_len += len;
        pRange++;
    }
    return( regs_len );
}  /* netdev_get_regs_len */


/*
    netdev_get_regs

    Description:
        This procedure dumps the register values in the provided buffer.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_regs* regs
            Pointer to ethtool registers data structure.

        void* ptr
            Pointer to buffer to store the register values.

    Return (None):
*/

static void netdev_get_regs (
    struct net_device   *dev,
    struct ethtool_regs *regs,
    void                *ptr )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    char*            buf = ( char* ) ptr;
    struct hw_regs*  pRange = hw_regs_range;
    int              base;
    int              i;
    int              len;
    int              rc;
    int              val;

    if ( ( rc = AcquireHardware( hw_priv, FALSE, TRUE )) ) {
        return;
    }
    regs->version = 0;
    while ( pRange->end > pRange->start ) {
	base = ( int ) hw_priv->hw.m_pVirtualMemory - BASE_IO_ADDR;
	len = pRange->start;
	if ( len >= 0x10000 )
	    base = -0x10000;
	while ( len < pRange->end ) {
	    buf += sprintf( buf, "%04x:", base + len );
	    for ( i = 0; i < 4; i++, len += 4 ) {
		if ( base < 0 )
		    val = KS_READ_REG( base + len );
		else
		    HW_READ_DWORD( &hw_priv->hw, len, &val );
		buf += sprintf( buf, " %08x", val );
	    }
	    buf += sprintf( buf, "\n" );
	}
        pRange++;
    }
    ReleaseHardware( hw_priv, FALSE );
}  /* netdev_get_regs */


/*
    netdev_get_wol

    Description:
        This procedure returns Wake-on-LAN support.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_wolinfo* wol
            Pointer to ethtool Wake-on-LAN data structure.

    Return (None):
*/

static void netdev_get_wol (
    struct net_device      *dev,
    struct ethtool_wolinfo *wol )
{

    wol->supported = WAKE_PHY | WAKE_MAGIC;
    wol->wolopts = WAKE_PHY | WAKE_MAGIC;
    memset( &wol->sopass, 0, sizeof( wol->sopass ));
}  /* netdev_get_wol */


/*
    netdev_set_wol

    Description:
        This function sets Wake-on-LAN support.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_wolinfo* wol
            Pointer to ethtool Wake-on-LAN data structure.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_set_wol (
    struct net_device      *dev,
    struct ethtool_wolinfo *wol )
{
    u32 support;

    support = WAKE_PHY | WAKE_MAGIC;
    if ( wol->wolopts & ~support )
        return -EINVAL;

    return 0;
}  /* netdev_set_wol */


/*
    netdev_get_msglevel

    Description:
        This function returns current debug message level.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (u32):
        Current debug message flags.
*/

static u32 netdev_get_msglevel (
    struct net_device *dev )
{
    struct dev_priv* priv = netdev_priv(dev);

    return priv->msg_enable;
}  /* netdev_get_msglevel */


/*
    netdev_set_msglevel

    Description:
        This procedure sets debug message level.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        u32 value
            Debug message flags.

    Return (None):
*/

static void netdev_set_msglevel (
    struct net_device *dev,
    u32               value )
{
    struct dev_priv* priv = netdev_priv(dev);

    priv->msg_enable = value;
}  /* netdev_set_msglevel */


/*
    netdev_get_eeprom_len

    Description:
        This function returns the length of the EEPROM.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Length of the EEPROM.
*/

static int netdev_get_eeprom_len (
    struct net_device *dev )
{
    return( EEPROM_SIZE * 4 );
}  /* netdev_get_eeprom_len */


/*
    netdev_get_eeprom

    Description:
        This function dumps the EEPROM data in the provided buffer.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_eeprom* eeprom
            Pointer to ethtool EEPROM data structure.

        u8* data
            Pointer to buffer store the EEPROM data.

    Return (int):
        Zero if successful; otherwise an error code.
*/

#define EEPROM_MAGIC  0x10A18692

static int netdev_get_eeprom (
    struct net_device     *dev,
    struct ethtool_eeprom *eeprom,
    u8                    *data )
{
#if 0
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
#endif
    UCHAR*           eeprom_byte = ( UCHAR* ) eeprom_data;
    UCHAR            i;
    UCHAR            len;

    len = ( eeprom->offset + eeprom->len + 3 ) / 4;
    for ( i = eeprom->offset / 4; i < len; i++ )
        eeprom_data[ i ] = KS_READ_REG( i * 4 );
    eeprom->magic = EEPROM_MAGIC;
    memcpy( data, &eeprom_byte[ eeprom->offset ], eeprom->len );

    return 0;
}  /* netdev_get_eeprom */


/*
    netdev_set_eeprom

    Description:
        This function modifies the EEPROM data one byte at a time.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_eeprom* eeprom
            Pointer to ethtool EEPROM data structure.

        u8* data
            Pointer to data buffer.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_set_eeprom (
    struct net_device     *dev,
    struct ethtool_eeprom *eeprom,
    u8                    *data )
{
#if 0
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
#endif
    UINT             eeprom_word[ EEPROM_SIZE ];
    UCHAR*           eeprom_byte = ( UCHAR* ) eeprom_word;
    UCHAR            i;
    UCHAR            len;

    if ( eeprom->magic != EEPROM_MAGIC )
        return 1;

    len = ( eeprom->offset + eeprom->len + 3 ) / 4;
    for ( i = eeprom->offset / 4; i < len; i++ )
        eeprom_data[ i ] = KS_READ_REG( i * 4 );
    memcpy( eeprom_word, eeprom_data, EEPROM_SIZE * 4 );
    memcpy( &eeprom_byte[ eeprom->offset ], data, eeprom->len );
    for ( i = 0; i < EEPROM_SIZE; i++ )
        if ( eeprom_word[ i ] != eeprom_data[ i ] ) {
            eeprom_data[ i ] = eeprom_word[ i ];
            KS_WRITE_REG( i * 4, eeprom_data[ i ]);
        }

    return 0;
}  /* netdev_set_eeprom */


/*
    netdev_get_pauseparam

    Description:
        This procedures returns the PAUSE control flow settings.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_pauseparam* pause
            Pointer to ethtool PAUSE settings data structure.

    Return (None):
*/

static void netdev_get_pauseparam (
    struct net_device         *dev,
    struct ethtool_pauseparam *pause )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;

    pause->autoneg = 1;
    pause->rx_pause =
        ( pHardware->m_dwReceiveConfig & DMA_RX_CTRL_FLOW_ENABLE ) ? 1 : 0;
    pause->tx_pause =
        ( pHardware->m_dwTransmitConfig & DMA_TX_CTRL_FLOW_ENABLE ) ? 1 : 0;
}  /* netdev_get_pauseparam */


/*
    netdev_set_pauseparam

    Description:
        This function sets the PAUSE control flow settings.
        Not implemented.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_pauseparam* pause
            Pointer to ethtool PAUSE settings data structure.

    Return (int):
        Zero if successful; otherwise an error code.
*/

static int netdev_set_pauseparam (
    struct net_device         *dev,
    struct ethtool_pauseparam *pause )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    ULONG            ulConfig;

    ulConfig = pHardware->m_dwReceiveConfig;
    if ( pause->rx_pause )
        ulConfig |= DMA_RX_CTRL_FLOW_ENABLE;
    else
        ulConfig &= ~DMA_RX_CTRL_FLOW_ENABLE;
    if ( ulConfig != pHardware->m_dwReceiveConfig ) {
        pHardware->m_dwReceiveConfig = ulConfig;
    }

    ulConfig = pHardware->m_dwTransmitConfig;
    if ( pause->tx_pause )
        ulConfig |= DMA_TX_CTRL_FLOW_ENABLE;
    else
        ulConfig &= ~DMA_TX_CTRL_FLOW_ENABLE;
    if ( ulConfig != pHardware->m_dwTransmitConfig ) {
        pHardware->m_dwTransmitConfig = ulConfig;
    }

    return 0;
}  /* netdev_set_pauseparam */


static struct {
    char string[ ETH_GSTRING_LEN ];
} ethtool_stats_rx_keys[ RX_MIB_COUNTER_NUM ] = {
    { "rx_octets" },
    { "rx_undersize_packets" },
    { "rx_fragments" },
    { "rx_oversize_packets" },
    { "rx_jabbers" },
    { "rx_symbol_errors" },
    { "rx_crc_errors" },
    { "rx_align_errors" },
    { "rx_mac_ctrl_packets" },
    { "rx_pause_packets" },
    { "rx_bcast_packets" },
    { "rx_mcast_packets" },
    { "rx_ucast_packets" },
    { "rx_64_or_less_octet_packets" },
    { "rx_65_to_127_octet_packets" },
    { "rx_128_to_255_octet_packets" },
    { "rx_256_to_511_octet_packets" },
    { "rx_512_to_1023_octet_packets" },
    { "rx_1024_to_1521_octet_packets" },
    { "rx_1522_to_2000_octet_packets" },

#if 0
    { "1234567890123456789012345678901" }
#endif
};

static struct {
    char string[ ETH_GSTRING_LEN ];
} ethtool_stats_lan_rx_keys[ 1 ] = {
    { "rx_2001_or_more_octet_packets" },

#if 0
    { "1234567890123456789012345678901" }
#endif
};

static struct {
    char string[ ETH_GSTRING_LEN ];
} ethtool_stats_wan_rx_keys[ 2 ] = {
    { "rx_2001_to_9216_octet_packets" },
    { "rx_9217_or_more_octet_packets" },

#if 0
    { "1234567890123456789012345678901" }
#endif
};

static struct {
    char string[ ETH_GSTRING_LEN ];
} ethtool_stats_tx_keys[ TX_MIB_COUNTER_NUM ] = {
    { "tx_octets" },
    { "tx_late_collisions" },
    { "tx_pause_packets" },
    { "tx_bcast_packets" },
    { "tx_mcast_packets" },
    { "tx_ucast_packets" },
    { "tx_deferred" },
    { "tx_total_collisions" },
    { "tx_excessive_collisions" },
    { "tx_single_collisions" },
    { "tx_mult_collisions" },

#if 0
    { "1234567890123456789012345678901" }
#endif
};

#ifdef CONFIG_KSZ8692VB
static struct {
    char string[ ETH_GSTRING_LEN ];
} ethtool_stats_drop_keys[ 2 ] = {
    { "packets_dropped" },
    { "checksum_errors_dropped" },

#if 0
    { "1234567890123456789012345678901" }
#endif
};
#endif


/*
    netdev_get_strings

    Description:
        This procedure returns the strings used to identify the statistics.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        u32 stringset
            String set identifier.

        u8* buf
            Pointer to buffer to store the strings.

    Return (None):
*/

static void netdev_get_strings (
    struct net_device *dev,
    u32               stringset,
    u8                *buf )
{

    if ( ETH_SS_STATS == stringset ) {
        struct dev_priv* priv = netdev_priv(dev);
        struct dev_info* hw_priv = priv->pDevInfo;
        PHARDWARE        pHardware = &hw_priv->hw;
        size_t           len = 0;
        int              rx_num = pHardware->m_nRegCounter -
            RX_MIB_COUNTER_NUM - TX_MIB_COUNTER_NUM;

        memcpy( buf, &ethtool_stats_rx_keys, sizeof( ethtool_stats_rx_keys ));
        len += sizeof( ethtool_stats_rx_keys );
        if ( 2 == rx_num ) {
            memcpy( &buf[ len ], &ethtool_stats_wan_rx_keys,
                sizeof( ethtool_stats_wan_rx_keys ));
            len += sizeof( ethtool_stats_wan_rx_keys );
        }
        else if ( 1 == rx_num ) {
            memcpy( &buf[ len ], &ethtool_stats_lan_rx_keys,
                sizeof( ethtool_stats_lan_rx_keys ));
            len += sizeof( ethtool_stats_lan_rx_keys );
        }
        memcpy( &buf[ len ], &ethtool_stats_tx_keys,
            sizeof( ethtool_stats_tx_keys ));

#ifdef CONFIG_KSZ8692VB
        len += sizeof( ethtool_stats_tx_keys );
        memcpy( &buf[ len ], &ethtool_stats_drop_keys,
            sizeof( ethtool_stats_drop_keys ));
#endif
    }
}  /* netdev_get_strings */


#if 0
/*
    netdev_get_stats_count

    Description:
        This function returns the size of the statistics to be reported.

    Parameters:
        struct net_device* dev
            Pointer to network device.

    Return (int):
        Size of the statistics to be reported.
*/

static int netdev_get_stats_count (
    struct net_device *dev )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              count = pHardware->m_nRegCounter;

#ifdef CONFIG_KSZ8692VB
    count += 2;
#endif
    return( count );
}  /* netdev_get_stats_count */
#endif


/*
    netdev_get_ethtool_stats

    Description:
        This procedure returns the statistics.

    Parameters:
        struct net_device* dev
            Pointer to network device.

        struct ethtool_stats* stats
            Pointer to ethtool statistics data structure.

        u64* data
            Pointer to buffer to store the statistics.

    Return (None):
*/

static void netdev_get_ethtool_stats (
    struct net_device    *dev,
    struct ethtool_stats *stats,
    u64                  *data )
{
    struct dev_priv* priv = netdev_priv(dev);
    struct dev_info* hw_priv = priv->pDevInfo;
    PHARDWARE        pHardware = &hw_priv->hw;
    int              n_stats = stats->n_stats;
    int              i;
    int              n;
    int              rc;

    hw_priv->Counter.fRead = 1;
    rc = interruptible_sleep_on_timeout(
        &hw_priv->Counter.wqhCounter, HZ * 2 );
    n = TOTAL_PORT_COUNTER_NUM;
    if ( n > n_stats )
        n = n_stats;
    n_stats -= n;
    for ( i = 0; i < n; i++ ) {
        *data++ = ( u64 ) pHardware->m_cnMIB[ i ];
    }
}  /* netdev_get_ethtool_stats */

static struct ethtool_ops netdev_ethtool_ops = {
#ifdef MII
    .get_settings       = netdev_get_settings,
    .set_settings       = netdev_set_settings,
    .nway_reset         = netdev_nway_reset,
    .get_link           = netdev_get_link,
#endif
    .get_drvinfo        = netdev_get_drvinfo,
    .get_regs_len       = netdev_get_regs_len,
    .get_regs           = netdev_get_regs,
    .get_wol            = netdev_get_wol,
    .set_wol            = netdev_set_wol,
    .get_msglevel       = netdev_get_msglevel,
    .set_msglevel       = netdev_set_msglevel,
    .get_eeprom_len     = netdev_get_eeprom_len,
    .get_eeprom         = netdev_get_eeprom,
    .set_eeprom         = netdev_set_eeprom,
    .get_pauseparam     = netdev_get_pauseparam,
    .set_pauseparam     = netdev_set_pauseparam,
    .get_strings        = netdev_get_strings,
    .get_ethtool_stats  = netdev_get_ethtool_stats,
};

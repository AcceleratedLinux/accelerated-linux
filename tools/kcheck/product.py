groups = {}
products = {}

groups['linux_common'] = dict(
    include = [
        'linux_crypto',
        'linux_net',
    ],
    device = dict(
        DEFAULTS_KERNEL_LINUX='y',
    ),
    linux = dict(
        DEBUG_INFO_NONE=['y','n'],
        DEFAULT_HOSTNAME='(none)',
        LOCALVERSION='',
        LOCALVERSION_AUTO='y',
        TICK_CPU_ACCOUNTING='y',
        BLK_DEV_INITRD='y',
        INITRAMFS_SOURCE='',
        RD_GZIP='y',
        EXPERT='y',
        MULTIUSER='y',
        PRINTK='y',
        BUG='y',
        BASE_FULL='y',
        EPOLL='y',
        SIGNALFD='y',
        TIMERFD='y',
        EVENTFD='y',
        SHMEM='y',
        ADVISE_SYSCALLS='y',
        MEMBARRIER='y',
        EMBEDDED='y',
        SLAB_MERGE_DEFAULT='y',
        SLUB_DEBUG='y',
        SLUB='y',
        MODULES='y',
        MODULE_UNLOAD='y',
        BLOCK='y',
        PARTITION_ADVANCED='y',
        PREEMPT_NONE='y',
        DEFAULT_MMAP_MIN_ADDR=4096,
        BINFMT_ELF='y',
        BINFMT_SCRIPT='y',
        COREDUMP='y',
        DEVTMPFS='y',
        DEVTMPFS_MOUNT='y',
        PREVENT_FIRMWARE_BUILD='y',
        MTD='y',
        MTD_BLOCK='y',
        BLK_DEV='y',
        BLK_DEV_RAM='y',
        BLK_DEV_RAM_COUNT=4,
        TTY='y',
        LDISC_AUTOLOAD='y',
        UNIX98_PTYS='y',
        LEDMAN='y',
        GPIOLIB='y',
        GPIO_SYSFS='y',
        FILE_LOCKING='y',
        INOTIFY_USER='y',
        PROC_FS='y',
        PROC_SYSCTL='y',
        PROC_PAGE_MONITOR='y',
        SYSFS='y',
        TMPFS='y',
        MISC_FILESYSTEMS='y',
        SQUASHFS='y',
        SQUASHFS_FILE_CACHE='y',
        SQUASHFS_DECOMP_SINGLE='y',
        SQUASHFS_XZ='y',
        SQUASHFS_XATTR='y',
        NLS='y',
        NLS_DEFAULT='iso8859-1',
        NLS_CODEPAGE_437='y',
        NLS_ISO8859_1='y',
        POSIX_TIMERS='y',
        COREDUMP_PRINTK='y',
        CONSOLE_LOGLEVEL_DEFAULT=7,
        MESSAGE_LOGLEVEL_DEFAULT=4,
        PRINTK_SAFE_LOG_BUF_SHIFT='13',
        DEBUG_KERNEL='y',
        PANIC_TIMEOUT=-1,
        SECTION_MISMATCH_WARN_ONLY='y',
        CRC_CCITT='y',
        CRC32='y',
        CRC32_SLICEBY8='y',
        XZ_DEC='y',
        )
    )

groups['linux_hugepages'] = dict(
    linux = dict(
        TRANSPARENT_HUGEPAGE='y',
        TRANSPARENT_HUGEPAGE_ALWAYS='y',
        HUGETLBFS='y',
        COMPACTION='y',
        MIGRATION='y',
        )
    )

groups['linux_smp'] = dict(
    linux = dict(
        SMP='y',
        RCU_CPU_STALL_TIMEOUT='21',
        ),
    )

groups['linux_crypto'] = dict(
    linux = dict(
        CRYPTO='y',
        CRYPTO_MANAGER='y',
        CRYPTO_MANAGER_DISABLE_TESTS='y',
        CRYPTO_NULL='y',
        CRYPTO_CBC='y',
        CRYPTO_ECB='y',
        CRYPTO_HMAC='y',
        CRYPTO_MD5='y',
        CRYPTO_SHA1='y',
        CRYPTO_SHA256='y',
        CRYPTO_AES='y',
        CRYPTO_DEFLATE='y',
        CRYPTO_DES='y',
        CRYPTO_SEQIV='y',
        CRYPTO_ECHAINIV='y',
        )
    )

groups['linux_crypto_arm'] = dict(
    linux = dict(
        ARM_CRYPTO='y',
        CRYPTO_AES_ARM='y',
        CRYPTO_SHA1_ARM='y',
        CRYPTO_SHA256_ARM='y',
        CRYPTO_SHA512_ARM='y',
        )
    )

groups['linux_crypto_arm64'] = dict(
    linux = dict(
        ARM64_CRYPTO='y',
        CRYPTO_SM4='y',
        CRYPTO_SHA1_ARM64_CE='y',
        CRYPTO_SHA2_ARM64_CE='y',
        CRYPTO_SHA512_ARM64_CE='y',
        CRYPTO_SHA3_ARM64='y',
        CRYPTO_SM3_ARM64_CE='y',
        CRYPTO_SM4_ARM64_CE='y',
        CRYPTO_GHASH_ARM64_CE='y',
        CRYPTO_CRCT10DIF_ARM64_CE='y',
        CRYPTO_AES_ARM64='y',
        CRYPTO_AES_ARM64_CE_CCM='y',
        CRYPTO_AES_ARM64_CE_BLK='y',
        CRYPTO_CHACHA20_NEON='y',
        CRYPTO_NHPOLY1305_NEON='y',
        CRYPTO_AES_ARM64_BS='y',
        )
    )

groups['linux_crypto_x86'] = dict(
    linux = dict(
        CRYPTO_CRC32C_INTEL='y',
        CRYPTO_SHA1_SSSE3='y',
        CRYPTO_SHA256_SSSE3='y',
        CRYPTO_SHA512_SSSE3='y',
        CRYPTO_AES_NI_INTEL='y',
        CRYPTO_DES3_EDE_X86_64='y',
        )
    )

groups['linux_crypto_mv_cesa'] = dict(
    linux = dict(
        CRYPTO_HW='y',
        CRYPTO_DEV_MARVELL_CESA=['y', 'm'],
        SRAM='y',
        )
    )

groups['linux_crypto_smp'] = dict(
    linux = dict(
        CRYPTO_USER='y',
        CRYPTO_PCRYPT='y',
        CRYPTO_CRYPTD='y',
        CRYPTO_AUTHENC='y',
        ),
    user = dict(
        USER_CRCONF='y',
        )
    )

groups['linux_ocf_mv_cesa'] = dict(
    linux = dict(
        SRAM='y'
    ),
    modules = dict(
        OCF_OCF='m',
        OCF_CRYPTODEV='m',
        OCF_KIRKWOOD='m'
        )
    )

groups['linux_ocf_cryptosoft'] = dict(
    modules = dict(
        OCF_OCF='m',
        OCF_CRYPTODEV='m',
        OCF_CRYPTOSOFT='m'
        )
    )

groups['modules_cryptodev'] = dict(
    modules = dict(
        MODULES_CRYPTODEV_CRYPTODEV='m',
        )
    )

groups['modules_qcacld'] = dict(
    modules = dict(
        QCA_CLD_WLAN='y',
        )
    )

groups['linux_rtc'] = dict(
    linux = dict(
        RTC_CLASS='y',
        RTC_HCTOSYS='y',
        RTC_SYSTOHC='y',
        RTC_HCTOSYS_DEVICE='rtc0',
        RTC_SYSTOHC_DEVICE='rtc0',
        RTC_INTF_SYSFS='y',
        RTC_INTF_PROC='y',
        RTC_INTF_DEV='y',
        ),
    )

groups['linux_rtc_imx'] = dict(
    linux = dict(
        RTC_DRV_MXC='y',
        RTC_DRV_SNVS='y',
        ),
    )

groups['linux_input_mousedev'] = dict(
    linux = dict(
        INPUT='y',
        INPUT_MOUSEDEV='y',
        INPUT_MOUSEDEV_PSAUX='y',
        INPUT_MOUSEDEV_SCREEN_X='1024',
        INPUT_MOUSEDEV_SCREEN_Y='768',
        )
    )

groups['linux_hid'] = dict(
    linux = dict(
        HID='y',
        HID_GENERIC='y',
        HID_A4TECH='y',
        HID_APPLE='y',
        HID_BELKIN='y',
        HID_CHERRY='y',
        HID_CHICONY='y',
        HID_CYPRESS='y',
        HID_EZKEY='y',
        HID_KENSINGTON='y',
        HID_LOGITECH='y',
        HID_MICROSOFT='y',
        HID_MONTEREY='y',
        USB_HID='y',
        ),
    )

groups['linux_uio'] = dict(
    linux = dict(
        UIO='m',
        UIO_PDRV_GENIRQ='m',
        UIO_DMEM_GENIRQ='m',
        UIO_PCI_GENERIC='m',
        VFIO='m',
        VFIO_PCI='m',
        ),
    )

groups['linux_netfilter'] = dict(
    linux = dict(
        NETFILTER='y',
        NETFILTER_ADVANCED='y',
        NETFILTER_NETLINK_LOG='m',
        NETFILTER_INGRESS='y',
        NF_CONNTRACK='m',
        NF_CONNTRACK_FTP='m',
        NF_CONNTRACK_H323='m',
        NF_CONNTRACK_IRC='m',
        NF_CONNTRACK_SNMP='m',
        NF_CONNTRACK_PPTP='m',
        NF_CONNTRACK_SIP='m',
        NF_CONNTRACK_TFTP='m',
        NF_CT_NETLINK='m',
        NF_NAT_SNMP_BASIC='m',
        NF_SOCKET_IPV4='m',
        NF_SOCKET_IPV6='m',
        )
    )

groups['linux_netfilter_ipv4'] = dict(
    linux = dict(
        NETFILTER='y',
        NETFILTER_ADVANCED='y',
        NETFILTER_NETLINK_LOG='m',
        NETFILTER_INGRESS='y',
        NF_CONNTRACK='m',
        NF_CONNTRACK_FTP='m',
        NF_CONNTRACK_H323='m',
        NF_CONNTRACK_IRC='m',
        NF_CONNTRACK_SNMP='m',
        NF_CONNTRACK_PPTP='m',
        NF_CONNTRACK_SIP='m',
        NF_CONNTRACK_TFTP='m',
        NF_CT_NETLINK='m',
        NF_NAT_SNMP_BASIC='m',
        NF_SOCKET_IPV4='m',
        )
    )

groups['linux_netfilter_xt'] = dict(
    linux = dict(
        NETFILTER_XTABLES='m',
        NETFILTER_XT_MARK='m',
        NETFILTER_XT_CONNMARK='m',
        NETFILTER_XT_TARGET_CT='m',
        NETFILTER_XT_TARGET_DSCP='m',
        NETFILTER_XT_TARGET_LOG='m',
        NETFILTER_XT_TARGET_NETMAP='m',
        NETFILTER_XT_TARGET_NFLOG='m',
        NETFILTER_XT_TARGET_NOTRACK='m',
        NETFILTER_XT_TARGET_REDIRECT='m',
        NETFILTER_XT_TARGET_TPROXY='m',
        NETFILTER_XT_TARGET_TCPMSS='m',
        NETFILTER_XT_TARGET_TRACE='m',
        NETFILTER_XT_MATCH_ADDRTYPE='m',
        NETFILTER_XT_MATCH_COMMENT='m',
        NETFILTER_XT_MATCH_CONNLIMIT='m',
        NETFILTER_XT_MATCH_CONNTRACK='m',
        NETFILTER_XT_MATCH_DSCP='m',
        NETFILTER_XT_MATCH_ECN='m',
        NETFILTER_XT_MATCH_ESP='m',
        NETFILTER_XT_MATCH_HELPER='m',
        NETFILTER_XT_MATCH_HL='m',
        NETFILTER_XT_MATCH_IPRANGE='m',
        NETFILTER_XT_MATCH_LENGTH='m',
        NETFILTER_XT_MATCH_LIMIT='m',
        NETFILTER_XT_MATCH_MAC='m',
        NETFILTER_XT_MATCH_MULTIPORT='m',
        NETFILTER_XT_MATCH_PKTTYPE='m',
        NETFILTER_XT_MATCH_REALM='m',
        NETFILTER_XT_MATCH_RECENT='m',
        NETFILTER_XT_MATCH_SOCKET='m',
        NETFILTER_XT_MATCH_STATISTIC='m',
        NETFILTER_XT_MATCH_STATE='m',
        NETFILTER_XT_MATCH_STRING='m',
        NETFILTER_XT_MATCH_TCPMSS='m',
        NETFILTER_XT_MATCH_TIME='m',
        NETFILTER_XT_MATCH_BPF='m',
        NF_LOG_IPV4='m',
        NF_REJECT_IPV4='m',
        IP_NF_IPTABLES='m',
        IP_NF_MATCH_RPFILTER='m',
        IP_NF_FILTER='m',
        IP_NF_TARGET_REJECT='m',
        IP_NF_NAT='m',
        IP_NF_TARGET_MASQUERADE='m',
        IP_NF_MANGLE='m',
        IP_NF_TARGET_ECN='m',
        IP_NF_RAW='m'
        )
    )

groups['linux_iptables'] = dict(
    include = [
        'linux_netfilter_xt',
        'linux_netfilter',
        ],
    )

groups['linux_iptables_ipv4'] = dict(
    include = [
        'linux_netfilter_xt',
        'linux_netfilter_ipv4'
        ],
    )

groups['linux_ip6tables'] = dict(
    include = [ 'linux_netfilter' ],
    linux = dict(
        NF_REJECT_IPV6='m',
        NF_LOG_IPV6='m',
        IP6_NF_IPTABLES='m',
        IP6_NF_MATCH_EUI64='m',
        IP6_NF_MATCH_RPFILTER='m',
        IP6_NF_FILTER='m',
        IP6_NF_TARGET_REJECT='m',
        IP6_NF_MANGLE='m',
        IP6_NF_RAW='m',
        IP6_NF_NAT='m',
        IP6_NF_TARGET_MASQUERADE='m',
        )
    )

groups['linux_arptables'] = dict(
    include = [ 'linux_netfilter' ],
    linux = dict(
        IP_NF_ARPTABLES='m',
        IP_NF_ARPFILTER='m',
        IP_NF_ARP_MANGLE='m',
        )
    )

groups['linux_nftables'] = dict(
    include = [ 'linux_netfilter' ],
    linux = dict(
        NF_TABLES='m',
        NFT_META='m',
        NFT_CT='m',
        NFT_RBTREE='m',
        NFT_HASH='m',
        NFT_COUNTER='m',
        NFT_LOG='m',
        NFT_LIMIT='m',
        NFT_MASQ='m',
        NFT_REDIR='m',
        NFT_NAT='m',
        NFT_REJECT='m',
        NFT_COMPAT='m',
        NF_TABLES_IPV4='m',
        NFT_CHAIN_ROUTE_IPV4='m',
        NFT_CHAIN_NAT_IPV4='m',
        NFT_MASQ_IPV4='m',
        NFT_REDIR_IPV4='m',
        NF_TABLES_IPV6='m',
        NFT_CHAIN_ROUTE_IPV6='m',
        NFT_CHAIN_NAT_IPV6='m',
        NFT_MASQ_IPV6='m',
        NFT_REDIR_IPV6='m',
        ),
    user = dict(
        USER_NFTABLES='y',
        )
    )

groups['linux_ipv6'] = dict(
    linux = dict(
        IPV6='y',
        IPV6_SIT='y',
        IPV6_SIT_6RD='y',
        IPV6_MULTIPLE_TABLES='y',
        )
    )

groups['linux_netkey'] = dict(
    include = [ 'linux_crypto' ],
    linux = dict(
        XFRM_USER='y',
        XFRM_INTERFACE='y',
        NET_KEY='y',
        INET_AH='y',
        INET_ESP='y',
        INET_IPCOMP='y',
        NETFILTER_XT_MATCH_POLICY='m',
        IP6_NF_MATCH_AH='m',
        )
    )

groups['linux_netkey_ipv6'] = dict(
    linux = dict(
        INET6_AH='y',
        INET6_ESP='y',
        INET6_IPCOMP='y',
        )
    )

groups['linux_net'] = dict(
    linux = dict(
        NET='y',
        PACKET='y',
        UNIX='y',
        INET='y',
        IP_MULTICAST='y',
        IP_ADVANCED_ROUTER='y',
        IP_MULTIPLE_TABLES='y',
        IP_ROUTE_MULTIPATH='y',
        IP_ROUTE_VERBOSE='y',
        NET_IPIP='y',
        NET_IPGRE_DEMUX='y',
        NET_IPGRE='y',
        SYN_COOKIES='y',
        VLAN_8021Q='y',
        NETDEVICES='y',
        NET_CORE='y',
        TUN='y',
        ETHERNET='y',
        PHYLIB='y',
        PPP='y',
        PPP_BSDCOMP='y',
        PPP_DEFLATE='y',
        PPP_MPPE='y',
        PPPOE='y',
        PPP_ASYNC='y',
        SLIP='y',
        SLIP_COMPRESSED='y',
        BPF_JIT='y',
        )
    )

groups['linux_net_sched'] = dict(
    linux = dict(
        NET_SCHED='y',
        NET_SCH_CBQ='y',
        NET_SCH_HTB='y',
        NET_SCH_PRIO='y',
        NET_SCH_RED='y',
        NET_SCH_SFQ='y',
        NET_SCH_TEQL='y',
        NET_SCH_TBF='y',
        NET_SCH_GRED='y',
        NET_SCH_DSMARK='y',
        NET_SCH_INGRESS='y',
        NET_CLS_TCINDEX='y',
        NET_CLS_ROUTE4='y',
        NET_CLS_FW='y',
        NET_CLS_U32='y',
        NET_CLS_RSVP='y',
        NET_CLS_ACT='y',
        NET_ACT_POLICE='y',
        IFB='y',
        )
    )

groups['linux_net_e1000'] = dict(
    linux = dict(
        NET_VENDOR_INTEL='y',
        E1000='y',
        E1000E='y',
        IGB='y',
        IGB_HWMON='y',
        IGBVF='y',
        )
    )

groups['linux_net_r8169'] = dict(
    linux = dict(
        NET_VENDOR_REALTEK='y',
        R8169='y',
        R8169_NOPROM='y',
        )
    )

groups['linux_net_i225'] = dict(
    linux = dict(
        NET_VENDOR_INTEL='y',
        IGC='y',
        )
    )

groups['linux_bridge'] = dict(
    linux = dict(
        BRIDGE='y',
        BRIDGE_IGMP_SNOOPING='y',
        BRIDGE_VLAN_FILTERING='y',
        )
    )

groups['linux_bridge_netfilter'] = dict(
    include = [ 'linux_bridge' ],
    linux = dict(
        BRIDGE_NETFILTER='y',
        NETFILTER_XT_MATCH_PHYSDEV='m',
        )
    )

groups['linux_i2c'] = dict(
    linux = dict(
        I2C='y',
        I2C_COMPAT='y',
        I2C_HELPER_AUTO='y',
        )
    )

groups['linux_i2c_nexcom'] = dict(
    linux = dict(
        I2C='y',
        I2C_CHARDEV='y',
        I2C_HELPER_AUTO='y',
        I2C_I801='y',
        I2C_ISCH='y',
        I2C_ISMT='y',
        I2C_SCMI='y',
        )
    )

groups['linux_i2c_factory'] = dict(
    linux = dict(
        I2C='m',
        I2C_CHARDEV='m',
        I2C_COMPAT='y',
        I2C_HELPER_AUTO='y',
        I2C_MCP2221='m',
        )
    )

groups['linux_sata'] = dict(
    linux = dict(
        ATA='y',
        ATA_VERBOSE_ERROR='y',
        SATA_PMP='y',
        ATA_SFF='y',
        ATA_BMDMA='y',
        )
    )

groups['hardware_mmc'] = dict(
    linux = dict(
        MMC='y',
        MMC_BLOCK='y',
        MMC_BLOCK_MINORS='32',
        MMC_ARMMMCI='y',
        MMC_STM32_SDMMC='y',
        MMC_SDHCI='y',
        MMC_SDHCI_PLTFM='y',
        MMC_SDHCI_OF_ESDHC='y',
        MMC_CQHCI='y',
        )
    )

groups['hardware_imx6_mmc'] = dict(
    linux = dict(
        MMC='y',
        MMC_SDHCI_ESDHC_IMX='y',
        MMC_BLOCK='y',
        MMC_BLOCK_MINORS=8,
        MMC_SDHCI='y',
        MMC_SDHCI_IO_ACCESSORS='y',
        MMC_SDHCI_PLTFM='y',
        MMC_CQHCI='y',
        )
    )

groups['hardware_370_mmc'] = dict(
    linux = dict(
        MMC='y',
        MMC_BLOCK='y',
        MMC_MVSDIO='y',
        PWRSEQ_EMMC='y',
        PWRSEQ_SIMPLE='y',
        )
    )

groups['linux_pci'] = dict(
    linux = dict(
        PCI='y',
        PCI_QUIRKS='y',
        )
    )

groups['linux_pci_armada_370'] = dict(
    include = [ 'linux_pci' ],
    linux = dict(
        PCI_MVEBU='y',
        PCI_MSI='y',
        SERIAL_8250_PCI='y',
        VGA_ARB='y',
        VGA_ARB_MAX_GPUS=16,
        USB_PCI='y',
        )
    )

groups['linux_pci_layerscape'] = dict(
    include = [ 'linux_pci' ],
    linux = dict(
        PCI_HOST_GENERIC='y',
        PCI_LAYERSCAPE='y',
        PCIE_DW_PLAT_HOST='y',
        PCIEPORTBUS='y',
        PCIEAER='y',
        PCIEASPM='y',
        PCI_PRI='y',
        PCI_PASID='y',
        PCI_IOV='y',
        HW_RANDOM='y',
        )
    )

groups['linux_pci_x86'] = dict(
    include = [ 'linux_pci' ],
    linux = dict(
        PCI_MSI='y',
        PCI_PRI='y',
        PCI_PASID='y',
        PCI_MMCONFIG='y',
        PCIEPORTBUS='y',
        PCIEAER='y',
        PCIEASPM='y',
        PCIEASPM_DEFAULT='y',
        PCI_IOV='y',
        ),
    )

groups['linux_pci_mt7621'] = dict(
    include = [ 'linux_pci' ],
    linux = dict(
        PCIE_MT7621='y',
        PHY_MT7621_PCI='y',
        PCI_MSI='y',
        PCI_QUIRKS='y',
        PCIEPORTBUS='y',
        PCIEAER='y',
        PCIEASPM='y',
        PCIEASPM_DEFAULT='y',
        ),
    )

groups['linux_pci_qcom'] = dict(
    include = [ 'linux_pci'],
    linux = dict(
        PCIEPORTBUS='y',
        PCIEASPM='y',
        PCI_QUIRKS='y',
        PCI_DEBUG='y',
        PCIE_QCOM='y',
        ),
    )

groups['linux_usb'] = dict(
    linux = dict(
        USB_SUPPORT='y',
        USB='y',
        USB_ANNOUNCE_NEW_DEVICES='y',
        USB_EHCI_HCD='y',
        USB_EHCI_ROOT_HUB_TT='y',
        USB_EHCI_TT_NEWSCHED='y',
        USB_ACM='y',
        USB_WDM='y',
        USB_DEFAULT_PERSIST='y',
        GENERIC_PHY='y',
        )
    )

groups['linux_usb_armada_370'] = dict(
    include = [ 'linux_usb' ],
    linux = dict(
        USB_EHCI_HCD_ORION='y',
        USB_EHCI_HCD_PLATFORM='y',
        USB_MARVELL_ERRATA_FE_9049667='y',
        )
    )

groups['linux_usb_armada_380'] = dict(
    include = [ 'linux_usb' ],
    linux = dict(
        USB_EHCI_HCD_ORION='y',
        USB_EHCI_HCD_PLATFORM='y',
        USB_XHCI_HCD='y',
        USB_XHCI_PLATFORM='y',
        USB_XHCI_MVEBU='y',
        USB_ULPI_BUS='y',
        PHY_PXA_28NM_HSIC='y',
        PHY_PXA_28NM_USB2='y',
        )
    )

groups['linux_usb_qcom'] = dict(
    include = [ 'linux_usb' ],
    linux = dict(
        USB_DWC3='y',
        USB_DWC3_QCOM='y',
        USB_CHIPIDEA='y',
        USB_CHIPIDEA_HOST='y',
        USB_CHIPIDEA_PCI='y',
        USB_CHIPIDEA_MSM='y',
        USB_CHIPIDEA_GENERIC='y',
        USB_PCI='y',
        USB_XHCI_HCD='y',
        USB_EHCI_HCD_PLATFORM='y',
        ),
    )

# This includes some external stuff, we'll fix it if we need the space
groups['linux_usb_net_internal'] = dict(
    include = [ 'linux_usb' ],
    linux = dict(
        USB_NET_DRIVERS='y',
        USB_USBNET='y',
        USB_NET_AX8817X='y',
        USB_NET_AX88179_178A='y',
        USB_NET_CDCETHER='y',
        USB_NET_CDC_EEM='y',
        USB_NET_CDC_MBIM='y',
        USB_NET_CDC_NCM='y',
        USB_NET_RNDIS_HOST='y',
        USB_NET_CDC_SUBSET='y',
        USB_BELKIN='y',
        USB_ARMLINUX='y',
        ),
    )

groups['linux_usb_net_external'] = dict(
    include = [ 'linux_usb' ],
    linux = dict(
        USB_RTL8152='y',
        ),
    )

groups['linux_usb_net'] = dict(
    include = [
        'linux_usb_net_internal',
        ],
    )

groups['linux_usb_storage'] = dict(
    include = [ 'linux_usb' ],
    linux = dict(
        USB_STORAGE='y',
        USB_STORAGE_DATAFAB='m',
        USB_STORAGE_FREECOM='m',
        USB_STORAGE_USBAT='m',
        USB_STORAGE_SDDR09='m',
        USB_STORAGE_SDDR55='m',
        USB_STORAGE_JUMPSHOT='m'
        )
    )

groups['linux_usb_serial_common'] = dict(
    include = [ 'linux_usb' ],
    linux = dict(
        USB_SERIAL='y',
        USB_SERIAL_GENERIC='y',
        )
    )

groups['linux_usb_serial'] = dict(
    linux = dict(
        USB_SERIAL_CP210X='y',
        USB_SERIAL_FTDI_SIO='y',
        USB_SERIAL_PL2303='y'
        )
    )

groups['linux_usb_cellular_internal'] = dict(
    include = [
        'linux_usb_net_internal',
        'linux_usb_serial_common',
        ],
    linux = dict(
        USB_NET_QMI_WWAN=['y', 'n'],
        USB_SERIAL_QCAUX='y',
        USB_SERIAL_QUALCOMM='y',
        USB_SERIAL_SIERRAWIRELESS='m',
        USB_SERIAL_IPW='y',
        USB_SERIAL_SIMPLE='y',
        USB_SERIAL_OPTION='y',
        USB_SERIAL_QT2='y',
        )
    )

groups['linux_usb_external'] = dict(
    include = [
        'usb',
        'linux_usb_serial',
        'linux_usb_storage',
        'linux_usb_net',
        ]
    )

groups['hid_api'] = dict(
    include = [
        'usb',
        'linux_hid',
    ],
    linux = dict(
        INPUT='y',
        HIDRAW='y',
        ),
    user = dict(
        USER_HID_API='y',
        )
    )

groups['exfat'] = dict(
    linux = dict(
        EXFAT_FS='y',
        EXFAT_DEFAULT_IOCHARSET='utf8',
        ),
    user = dict(
        USER_EXFAT_UTILS='y',
        )
    )

groups['storage'] = dict(
    user = dict(
        PROP_CONFIG_EVENTD_SMTP='y',
        PROP_CONFIG_EVENTD_SNMP='y',
        PROP_CONFIG_STORAGE='y',
        PROP_CONFIG_STORAGE_SD='y',
        PROP_CONFIG_SYSLOG_PATH='y',
        USER_NETSNMP_APPS='y',
        USER_NETSNMP_APPS_TRAP='y',
        )
    )

groups['config_python_hid'] = dict(
    include = [
        'hid_api',
    ],
    user = dict(
        USER_PYTHON_MODULES_HID='y',
        )
    )

groups['config_pstore'] = dict(
    linux = dict(
        PSTORE='y',
        PSTORE_DEFLATE_COMPRESS='y',
        PSTORE_CONSOLE='y',
        PSTORE_RAM='y',
        )
    )

groups['linux_arm'] = dict(
    linux = dict(
        MMU='y',
        CPU_SW_DOMAIN_PAN='y',
        FORCE_MAX_ZONEORDER='11',
        VMSPLIT_3G='y',
        PINCTRL='y',
        OF='y',
        USE_OF='y',
        ATAGS='y',
        ZBOOT_ROM_TEXT='0x0',
        ZBOOT_ROM_BSS='0x0',
        ARCH_MMAP_RND_BITS='8',
        KERNEL_XZ='y',
        DEBUG_BUGVERBOSE='y',
        UID16='y',
        RSEQ='y',
        STACKPROTECTOR='y',
        STACKPROTECTOR_STRONG='y',
        COMPAT_32BIT_TIME='y',
        ),
    user = dict(
        LIB_INSTALL_LIBGCC_S='y'
        ),
    )

groups['linux_arm64'] = dict(
    linux = dict(
        MMU='y',
        ZONE_DMA32='y',
        VMAP_STACK='y',
        SPARSEMEM_VMEMMAP='y',
        UNMAP_KERNEL_AT_EL0='y',
        OF='y',
        DEBUG_BUGVERBOSE='y',
        RSEQ='y',
        STACKPROTECTOR='y',
        IKCONFIG_PROC='y',
        IKCONFIG='y',
        ELF_CORE='y',
        KALLSYMS='y',
        FUTEX='y',
        AIO='y',
        KSM='y',
        ),
    )

groups['linux_mips'] = dict(
    linux = dict(
        CPU_MIPS32_R2='y',
        PAGE_SIZE_4KB='y',
        MMU='y',
        MIPS_FP_SUPPORT='y',
        FORCE_MAX_ZONEORDER='11',
        PINCTRL='y',
        OF='y',
        USE_OF='y',
        ARCH_MMAP_RND_BITS='8',
        BPF_SYSCALL='y',
        RSEQ='y',
        RAS='y',
        CRC16='y',
        STACKPROTECTOR='y',
        COMPAT_32BIT_TIME='y',
        ),
    user = dict(
        LIB_INSTALL_LIBGCC_S='y'
        ),
    )

groups['linux_mvebu'] = dict(
    linux = dict(
        ARM_PATCH_PHYS_VIRT='y',
        NO_HZ_IDLE='y',
        NO_HZ='y',
        HIGH_RES_TIMERS='y',
        LOG_BUF_SHIFT='17',
        RD_BZIP2='y',
        RD_LZMA='y',
        RD_XZ='y',
        RD_LZO='y',
        KALLSYMS='y',
        ELF_CORE='y',
        FUTEX='y',
        AIO='y',
        VM_EVENT_COUNTERS='y',
        COMPAT_BRK='y',
        MODULE_FORCE_UNLOAD='y',
        BLK_DEV_BSG='y',
        MSDOS_PARTITION='y',
        EFI_PARTITION='y',
        ARCH_MULTIPLATFORM='y',
        ARCH_MULTI_V7='y',
        ARCH_MVEBU='y',
        ARM_THUMB='y',
        SWP_EMULATE='y',
        KUSER_HELPERS='y',
        CACHE_FEROCEON_L2='y',
        HZ_100='y',
        AEABI='y',
        CROSS_MEMORY_ATTACH='y',
        ARM_APPENDED_DTB='y',
        ARM_ATAG_DTB_COMPAT='y',
        ARM_ATAG_DTB_COMPAT_CMDLINE_FROM_BOOTLOADER='y',
        CMDLINE='',
        AUTO_ZRELADDR='y',
        VFP='y',
        NEON='y',
        CORE_DUMP_DEFAULT_ELF_HEADERS='y',
        STANDALONE='y',
        FW_LOADER='y',
        EXTRA_FIRMWARE='',
        MTD_CMDLINE_PARTS='y',
        MTD_OF_PARTS='y',
        MTD_RAW_NAND='y',
        MTD_NAND_MARVELL='y',
        MTD_UBI='y',
        MTD_UBI_WL_THRESHOLD='4096',
        MTD_UBI_BEB_LIMIT='20',
        MTD_UBI_GLUEBI='y',
        SCSI='y',
        SCSI_PROC_FS='y',
        BLK_DEV_SD='y',
        SCSI_LOWLEVEL='y',
        NET_VENDOR_MARVELL='y',
        MVMDIO='y',
        MVNETA='y',
        SERIAL_8250='y',
        SERIAL_8250_DEPRECATED_OPTIONS='y',
        SERIAL_8250_CONSOLE='y',
        SERIAL_8250_DMA='y',
        SERIAL_8250_DW='y',
        SERIAL_OF_PLATFORM='y',
        HW_RANDOM='y',
        SPI='y',
        SPI_ORION='y',
        PPS='y',
        PTP_1588_CLOCK='y',
        PINMUX='y',
        PINCONF='y',
        USB_STORAGE='y',
        NEW_LEDS='y',
        LEDS_CLASS='y',
        LEDS_TRIGGERS='y',
        RAS='y',
        EDAC='y',
        EDAC_LEGACY_SYSFS='y',
        DMADEVICES='y',
        DW_DMAC='y',
        MV_XOR='y',
        EXT4_FS='y',
        EXT3_FS='y',
        EXT4_USE_FOR_EXT2='y',
        DNOTIFY='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE='437',
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        DEBUG_MEMORY_INIT='y',
        ARM_UNWIND='y',
        UNCOMPRESS_INCLUDE='../mach-mvebu/include/mach/uncompress.h',
        CRYPTO_SHA512='y',
        CRYPTO_LZO='y',
        CRYPTO_ANSI_CPRNG='m',
        CRC16='y',
        XZ_DEC_ARM='y',
        XZ_DEC_ARMTHUMB='y',
        ),
    )

groups['hardware_armada_370'] = dict(
    include = [
        'linux_arm',
        'linux_mvebu',
        'linux_usb_armada_370',
        'linux_usb_external',
        ],
    linux = dict(
        MACH_ARMADA_370='y',
        CC_OPTIMIZE_FOR_PERFORMANCE='y',
        ARM_PATCH_IDIV='y',
        HARDEN_BRANCH_PREDICTOR='y',
        STRICT_KERNEL_RWX='y',
        STRICT_MODULE_RWX='y',
        DEBUG_ALIGN_RODATA='y',
        IWMMXT='y',
        PJ4B_ERRATA_4742='y',
        ARM_ERRATA_754322='y',
        CACHE_L2X0='y',
        DEBUG_LL='y',
        DEBUG_MVEBU_UART0='y',
        DEBUG_UART_PHYS='0xd0012000',
        DEBUG_UART_VIRT='0xfec12000',
        DEBUG_UART_8250_SHIFT=2,
        DEBUG_UNCOMPRESS='y',
        THERMAL='y',
        THERMAL_OF='y',
        THERMAL_DEFAULT_GOV_STEP_WISE='y',
        THERMAL_EMERGENCY_POWEROFF_DELAY_MS='0',
        ARMADA_THERMAL='y',
        ),
    )

groups['hardware_armada_380'] = dict(
    include = [
        'linux_arm',
        'linux_mvebu',
        'linux_usb_armada_380',
        'linux_usb_external',
        ],
    linux = dict(
        MACH_ARMADA_38X='y',
        SMP='y',
        SMP_ON_UP='y',
        HIGHMEM='y',
        HIGHPTE='y',
        BOUNCE='y',
        ARM_CPU_TOPOLOGY='y',
        NR_CPUS='2',
        RCU_CPU_STALL_TIMEOUT='21',
        LOG_CPU_MAX_BUF_SHIFT='12',
        CC_OPTIMIZE_FOR_PERFORMANCE='y',
        HARDEN_BRANCH_PREDICTOR='y',
        STRICT_KERNEL_RWX='y',
        STRICT_MODULE_RWX='y',
        DEBUG_ALIGN_RODATA='y',
        SLUB_CPU_PARTIAL='y',
        ARM_PATCH_IDIV='y',
        ARM_ERRATA_643719='y',
        ARM_ERRATA_720789='y',
        ARM_ERRATA_754322='y',
        CACHE_L2X0='y',
        THERMAL='y',
        THERMAL_OF='y',
        THERMAL_DEFAULT_GOV_STEP_WISE='y',
        THERMAL_EMERGENCY_POWEROFF_DELAY_MS='0',
        ARMADA_THERMAL='y',
        ),
    user = dict(
        BOOT_UBOOT_MARVELL_380='y'
        ),
    )

groups['hardware_imx6'] = dict(
    include = [
        'linux_arm',
        ],
    linux = dict(
        ARCH_MULTIPLATFORM='y',
        ARCH_MULTI_V7='y',
        ARCH_MXC='y',
        ARM_IMX6Q_CPUFREQ='y',
        ARM_PATCH_PHYS_VIRT='y',
        ARM_PATCH_IDIV='y',
        ARM_APPENDED_DTB='y',
        ARM_ATAG_DTB_COMPAT='y',
        ARM_ATAG_DTB_COMPAT_CMDLINE_FROM_BOOTLOADER='y',
        ARM_ERRATA_754322='y',
        ARM_ERRATA_775420='y',
        ARM_UNWIND='y',
        AUTO_ZRELADDR='y',
        SOC_IMX6UL='y',
        MXS_DMA='y',
        MX3_IPU='y',
        MX3_IPU_IRQS=4,
        IMX_IRQSTEER='y',
        CACHE_L2X0='y',
        HARDEN_BRANCH_PREDICTOR='y',
        STRICT_KERNEL_RWX='y',
        STRICT_MODULE_RWX='y',
        DMADEVICES='y',
        VFP='y',
        NEON='y',
        DW_DMAC='y',
        HZ_100='y',
        NO_HZ_IDLE='y',
        NO_HZ='y',
        HIGH_RES_TIMERS='y',
        CC_OPTIMIZE_FOR_PERFORMANCE='y',
        KALLSYMS='y',
        ELF_CORE='y',
        CORE_DUMP_DEFAULT_ELF_HEADERS='y',
        AIO='y',
        VM_EVENT_COUNTERS='y',
        KUSER_HELPERS='y',
        MODULE_FORCE_UNLOAD='y',
        FW_LOADER='y',
        EXTRA_FIRMWARE='',
        BLK_DEV_BSG='y',
        MSDOS_PARTITION='y',
        EFI_PARTITION='y',
        AEABI='y',
        FUTEX='y',
        CROSS_MEMORY_ATTACH='y',
        CMDLINE_FROM_BOOTLOADER='y',
        RD_BZIP2='y',
        RD_LZMA='y',
        RD_XZ='y',
        RD_LZO='y',
        RELAY='y',
        MTD_CMDLINE_PARTS='y',
        MTD_OF_PARTS='y',
        MTD_RAW_NAND='y',
        MTD_NAND_GPMI_NAND='y',
        MTD_UBI='y',
        MTD_UBI_WL_THRESHOLD='4096',
        MTD_UBI_BEB_LIMIT='20',
        MTD_UBI_GLUEBI='y',
        SCSI='y',
        BLK_DEV_SD='y',
        SCSI_LOWLEVEL='y',
        NET_VENDOR_FREESCALE='y',
        FEC='y',
        MICREL_PHY='y',
        FIXED_PHY='y',
        PTP_1588_CLOCK='y',
        PPS='y',
        CPU_FREQ='y',
        CPU_FREQ_DEFAULT_GOV_PERFORMANCE='y',
        REGULATOR='y',
        REGULATOR_ANATOP='y',
        USB_CHIPIDEA='y',
        USB_CHIPIDEA_HOST='y',
        USB_CHIPIDEA_MSM='y',
        USB_CHIPIDEA_IMX='y',
        USB_CHIPIDEA_GENERIC='y',
        USB_MXS_PHY='y',
        RESET_CONTROLLER='y',
        EXTCON='y',
        USB_EHCI_HCD_PLATFORM='y',
        GPIO_MXC='y',
        SERIAL_IMX='y',
        SERIAL_IMX_CONSOLE='y',
        DEBUG_ALIGN_RODATA='y',
        NOP_USB_XCEIV='y',
        NEW_LEDS='y',
        LEDS_CLASS='y',
        LEDS_TRIGGERS='y',
        DNOTIFY='y',
        CRYPTO_SHA512='y',
        CRYPTO_ANSI_CPRNG='m',
        CRYPTO_CCM='y',
        CRYPTO_GCM='y',
        XZ_DEC_ARM='y',
        XZ_DEC_ARMTHUMB='y',
        MFD_SYSCON='y',
        SRAM='y',
        WATCHDOG='y',
        WATCHDOG_STOP_ON_REBOOT=0,
        IMX2_WDT='y',
        HW_RANDOM='y',
        HW_RANDOM_IMX_RNGC='y',
        CRC16='y',
        ),
    user = dict(
        USER_ETHTOOL_ETHTOOL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_BUSYBOX_GETTY='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_EMCTEST_EMCTEST='y',
        USER_SIGS_SIGS='y',
        USER_IMX_KOBS='y',
        LIB_LIBFTDI='y',
        PROP_SIM_SIM='y',
        PROP_MTST='y',
        )
    )

groups['hardware_arm64'] = dict(
    include = [
        'linux_arm64',
        'linux_64bit',
        'linux_rtc',
        ],
    linux = dict(
        ARM64_HW_AFDBM='y',
        ARM64_PAN='y',
        ARM64_USE_LSE_ATOMICS='y',
        ARM64_LSE_ATOMICS='y',
        ARM64_RAS_EXTN='y',
        ARM64_CNP='y',
        ARM64_PTR_AUTH='y',
        ARM64_SVE='y',
        ARM64_ERRATUM_826319='y',
        ARM64_ERRATUM_827319='y',
        ARM64_ERRATUM_824069='y',
        ARM64_ERRATUM_819472='y',
        ARM64_ERRATUM_832075='y',
        ARM64_ERRATUM_843419='y',
        ARM64_ERRATUM_858921='y',
        ARM64_ERRATUM_1024718='y',
        ARM64_ERRATUM_1165522='y',
        ARM64_ERRATUM_1286807='y',
        ARM64_ERRATUM_1463225='y',
        RODATA_FULL_DEFAULT_ENABLED='y',
        HWSPINLOCK='y',
        SLUB_CPU_PARTIAL='y',
        CMA='y',
        DMA_CMA='y',
        HZ_PERIODIC='y',
        HIGH_RES_TIMERS='y',
        MODULE_FORCE_UNLOAD='y',
        ARM_ARCH_TIMER_EVTSTREAM='y',
        HISILICON_ERRATUM_161010101='y',
        ARM_SMMU='y',
        ARM_SMMU_V3='y',
        BLK_DEV_BSG='y',
        BLK_DEV_LOOP='y',
        STANDALONE='y',
        FW_LOADER='y',
        EXTRA_FIRMWARE='',
        SCSI='y',
        BLK_DEV_SD='y',
        CC_OPTIMIZE_FOR_PERFORMANCE='y',
        JUMP_LABEL='y',
        SCHED_MC='y',
        POWER_SUPPLY='y',
        HWMON='y',
        NEW_LEDS='y',
        LEDS_CLASS='y',
        LEDS_TRIGGERS='y',
        LEDS_TRIGGER_TIMER='y',
        LEDS_TRIGGER_HEARTBEAT='y',
        LEDS_TRIGGER_CPU='y',
        LEDS_TRIGGER_DEFAULT_ON='y',
        LEDS_TRIGGER_PANIC='y',
        LEDS_SYSCON='y',
        IOMMU_SUPPORT='y',
        PPS='y',
        DNOTIFY='y',
        DEBUG_MEMORY_INIT='y',
        MSDOS_PARTITION='y',
        EFI_PARTITION='y',
        CRYPTO_MANAGER_DISABLE_TESTS='y',
        CRYPTO_NULL='y',
        CRYPTO_HW='y',
        CRYPTO_SHA512='y',
        CRC16='y',
        CRC_T10DIF='y',
        VM_EVENT_COUNTERS='y',
        POWER_RESET_SYSCON='y',
        XZ_DEC_ARM='y',
        ),
    user = dict(
        USER_ETHTOOL_ETHTOOL='y',
        USER_LINUX_FIRMWARE='y',
        LIB_INSTALL_LIBGCC_S='y'
        ),
    )

groups['hardware_layerscape'] = dict(
    include = [
        'hardware_arm64',
        ],
    linux = dict(
        ARCH_LAYERSCAPE='y',
        CLK_QORIQ='y',
        CMA_SIZE_MBYTES='32',
        FSL_ERRATUM_A008585='y',
        SERIAL_8250='y',
        SERIAL_8250_DEPRECATED_OPTIONS='y',
        SERIAL_8250_CONSOLE='y',
        SERIAL_8250_DMA='y',
        SERIAL_OF_PLATFORM='y',
        SPI='y',
        SPI_FSL_QUADSPI='y',
        I2C='y',
        I2C_IMX='y',
        I2C_COMPAT='y',
        I2C_CHARDEV='y',
        I2C_MUX='y',
        I2C_HELPER_AUTO='y',
        I2C_DESIGNWARE_PLATFORM='y',
        FSL_MC_BUS='y',
        DMADEVICES='y',
        FSL_EDMA='y',
        PWRSEQ_EMMC='y',
        PWRSEQ_SIMPLE='y',
        MTD_OF_PARTS='y',
        MTD_SPI_NOR='y',
        MTD_SPI_NOR_USE_4K_SECTORS='y',
        BLK_DEV_RAM_SIZE=41943,
        SECCOMP='y',
        EXT4_FS='y',
        EXT4_USE_FOR_EXT2='y',
        VFAT_FS='y',
        CONFIGFS_FS='y',
        CRYPTO_DEV_FSL_CAAM='y',
        CRYPTO_DEV_FSL_CAAM_CRYPTO_API_QI='y',
        CRYPTO_DEV_FSL_CAAM_COMMON='y',
        CRYPTO_DEV_FSL_CAAM_JR='y',
        CRYPTO_DEV_FSL_CAAM_RINGSIZE='9',
        CRYPTO_DEV_FSL_CAAM_CRYPTO_API='y',
        CRYPTO_DEV_FSL_CAAM_AHASH_API='y',
        CRYPTO_DEV_FSL_CAAM_PKC_API='y',
        CRYPTO_DEV_FSL_CAAM_RNG_API='y',
        CRYPTO_DEV_FSL_CAAM_CRYPTO_API_DESC='y',
        CRYPTO_DEV_FSL_CAAM_AHASH_API_DESC='y',
        ),
    user = dict(
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        POOR_ENTROPY='n',
       )
    )

groups['hardware_layerscape_usb'] = dict(
    linux = dict(
        USB_PCI='y',
        USB_STORAGE='y',
        USB_XHCI_HCD='y',
        USB_MUSB_HDRC='y',
        USB_DWC3='y',
        USB_DWC3_OF_SIMPLE='y',
        USB_DWC2='y',
        USB_DWC2_PCI='y',
        NOP_USB_XCEIV='y',
        USB_ULPI='y',
        ),
    )

groups['hardware_layerscape_1012'] = dict(
    include = [
        'hardware_layerscape',
        ],
    linux = dict(
        NET_VENDOR_FREESCALE='y',
        FSL_FMAN='y',
        FSL_DPAA='y',
        FSL_XGMAC_MDIO='y',
        FSL_PPFE='y',
        FSL_PPFE_UTIL_DISABLED='y',
        DEBUG_FS='y',
        BLK_DEBUG_FS='y',
        GPIO_MPC8XXX='y',
        ),
    user = dict(
        USER_QORIQ_ENGINE_PFE_BIN='y',
       )
    )

groups['hardware_layerscape_1046'] = dict(
    include = [
        'hardware_layerscape',
        'hardware_wireless_ath10k',
        ],
    linux = dict(
        NET_VENDOR_FREESCALE='y',
        FSL_FMAN='y',
        FSL_XGMAC_MDIO='y',
        FSL_DPAA='y',
        FSL_DPAA_ETH='y',
        BLK_DEBUG_FS='y',
        DEBUG_FS='y',
        GPIO_PCA953X='y',
        ),
    )

groups['hardware_connectez_1046'] = dict(
    include = [
        'hardware_layerscape',
        'hardware_layerscape_usb',
        'hardware_cellular_cm',
        'linux_usb_net',
        'linux_usb_storage',
        'hardware_nor',
        'hardware_mmc',
        'linux_pci_layerscape',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        ],
    linux = dict(
        NET_VENDOR_FREESCALE='y',
        FSL_FMAN='y',
        FSL_XGMAC_MDIO='y',
        FSL_DPAA='y',
        FSL_DPAA_ETH='y',
        BLK_DEBUG_FS='y',
        GPIO_PCA953X='y',
        GPIO_CDEV = 'y',
        GPIO_CDEV_V1 = 'y',
        LEDS_GPIO = 'y',
        SERIAL_8250_PCI='m',
        SERIAL_8250_NR_UARTS=40,
        SERIAL_8250_RUNTIME_UARTS=40,
        SERIAL_8250_EXTENDED='y',
        HW_RANDOM_SHA204='y',
        SENSORS_LM90='y',
        FSL_SYSFS_SFP='m',
        SERIAL_8250_PERICOM_DIGI_EXTENSIONS='y',
        SERIAL_8250_PERICOM='m',
        GPIO_PERICOM='m',
        REALTEK_PHY='y',
        USB_AWUSB_DEFAULT_CLAIM_PORT='y',
        USB_AWUSB_NUM_PORTS='2',
        USB_MON='y',
        DEBUG_FS='y',
        FW_LOADER_USER_HELPER='y',
        SENSORS_NCT7802='y',
        GPIO_MPC8XXX='y',
        SFP='y',
        ),
    user = dict(
        PROP_SECUREBOOT='y',
        PROP_SECUREBOOT_PROGRAM_ECC508='y',
        PROP_SECUREBOOT_TOOLS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_KERNEL_DRIVER='y',
        USER_NETFLASH_MINIMUM_VERSION='20.5.0',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        USER_MMC_UTILS='y',
        USER_GEN_OTPMK_DRBG='y',
        BOOT_UBOOT_FREESCALE_QORIQ='y',
        BOOT_UBOOT_FREESCALE_QORIQ_TARGET='cez1046_qspi',
        PROP_SIM_SIM='y',
        LIB_LIBFTDI='y',
        PROP_MTST='y',
        PROP_AWUSBD='y',
        USER_USBHUBCTRL_USBHUBCTRL='y',
        USER_USBUTILS='y',
        ),
    )

groups['hardware_connectez_1012'] = dict(
    include = [
        'hardware_layerscape_1012',
        ],
    linux = dict(
        GPIO_CDEV = 'y',
        GPIO_CDEV_V1 = 'y',
        LEDS_GPIO = 'y',
        SERIAL_8250_PCI='m',
        SERIAL_8250_NR_UARTS=8,
        SERIAL_8250_RUNTIME_UARTS=8,
        SERIAL_8250_EXAR='m',
        SERIAL_8250_EXTENDED='y',
        HW_RANDOM_SHA204='y',
        SENSORS_LM90='y',
        FSL_SYSFS_SFP='m',
        ),
    user = dict(
        PROP_SECUREBOOT='y',
        PROP_SECUREBOOT_PROGRAM_ECC508='y',
        PROP_SECUREBOOT_TOOLS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_KERNEL_DRIVER='y',
        USER_NETFLASH_MINIMUM_VERSION='20.5.0',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        USER_MMC_UTILS='y',
        USER_GEN_OTPMK_DRBG='y',
        ),
    )

groups['hardware_anywhereusb2'] = dict(
    include = [
        'hardware_layerscape_1012',
        'hardware_layerscape_usb',
        ],
    linux = dict(
        GPIO_CDEV = 'y',
        GPIO_CDEV_V1 = 'y',
        LEDS_GPIO = 'y',
        HW_RANDOM_SHA204='y',
        SENSORS_LM90='y',
        FSL_SYSFS_SFP='m',
        LEDS_LP5569='y',
        GPIO_PCA953X='y',
        GPIO_PCA953X_IRQ='y',
        ),
    user = dict(
        PROP_SECUREBOOT='y',
        PROP_SECUREBOOT_PROGRAM_ECC508='y',
        PROP_SECUREBOOT_TOOLS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_KERNEL_DRIVER='y',
        USER_NETFLASH_MINIMUM_VERSION='20.5.0',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        USER_MMC_UTILS='y',
        USER_GEN_OTPMK_DRBG='y',
        BOOT_UBOOT_FREESCALE_QORIQ='y',
        BOOT_UBOOT_FREESCALE_QORIQ_TARGET='awusb2_commercial_qspi',
        ),
    )

groups['hardware_connectez1'] = dict(
    include = [
        'hardware_connectez_1012',
        ],
    linux = dict(
        FW_LOADER_USER_HELPER='y',
        SERIAL_XR20M1280='y',
        SERIAL_XR20M1280_SPI='y',
        SPI_MASTER='y',
        SPI_FSL_DSPI='y',
        ),
    user = dict(
        BOOT_UBOOT_FREESCALE_QORIQ='y',
        BOOT_UBOOT_FREESCALE_QORIQ_TARGET='cez1012mini_qspi',
        ),
    )

groups['hardware_connectez2_4'] = dict(
    include = [
        'hardware_connectez_1012',
        'hardware_layerscape_usb',
        'hardware_cellular_cm',
        'linux_usb_net',
        ],
    linux = dict(
        GPIO_PCA953X='y',
        GPIO_PCA953X_IRQ='y',
        LEDS_LP5569='y',
        SERIAL_8250_PERICOM_DIGI_EXTENSIONS='y',
        SERIAL_8250_PERICOM='m',
        ),
    user = dict(
        BOOT_UBOOT_FREESCALE_QORIQ='y',
        BOOT_UBOOT_FREESCALE_QORIQ_TARGET='cez1012_qspi',
        PROP_SIM_SIM='y',
        LIB_LIBFTDI='y',
        PROP_MTST='y',
        ),
    )

groups['hardware_ipq6018'] = dict(
    include = [
        'hardware_arm64',
        ],
    linux = dict(
        ARCH_QCOM='y',
        ARCH_RANDOM='y',
        ARM64_AMU_EXTN='y',
        ARM64_BTI='y',
        ARM64_E0PD='y',
        ARM64_ERRATUM_1319367='y',
        ARM64_ERRATUM_1530923='y',
        ARM64_ERRATUM_1542419='y',
        ARM64_ERRATUM_1508412='y',
        ARM64_TAGGED_ADDR_ABI='y',
        ARM64_TLB_RANGE='y',
        ARM64_MTE='y',
        ARM_CPUIDLE='y',
        ARM_SMCCC_SOC_ID='y',
        ARM_SMMU_DISABLE_BYPASS_BY_DEFAULT='y',
        BLK_DEV_RAM_SIZE=65536,
        CAVIUM_ERRATUM_30115='y',
        CAVIUM_TX2_ERRATUM_219='y',
        CMA_SIZE_MBYTES='0',
        COMMON_CLK_QCOM='y',
        CPU_FREQ='y',
        CPU_FREQ_STAT='y',
        CPU_FREQ_GOV_POWERSAVE='y',
        CPU_FREQ_GOV_USERSPACE='y',
        CPU_FREQ_GOV_ONDEMAND='y',
        CPU_FREQ_GOV_CONSERVATIVE='y',
        CPU_IDLE='y',
        CPU_IDLE_GOV_MENU='y',
        CPUFREQ_DT='y',
        CRYPTO_POLY1305_NEON='y',
        DEBUG_FS='y',
        DMADEVICES='y',
        FUJITSU_ERRATUM_010001='y',
        FW_CACHE='y',
        FW_LOADER_USER_HELPER='y',
        FW_LOADER_USER_HELPER_FALLBACK='y',
        HISILICON_ERRATUM_161600802='y',
        HOTPLUG_CPU='y',
        HWSPINLOCK_QCOM='y',
        I2C='y',
        I2C_COMPAT='y',
        I2C_CHARDEV='y',
        I2C_HELPER_AUTO='y',
        I2C_QUP='y',
        IPQ_APSS_6018='y',
        IPQ_GCC_6018='y',
        IPQ_SUBSYSTEM_RAMDUMP='y',
        KPSS_XCC='y',
        MAILBOX='y',
        MDIO_QCA='y',
        MFD_QCOM_RPM='y',
        MFD_SPMI_PMIC='y',
        MHI_QCA_BUS='y',
        NOP_USB_XCEIV='y',
        PHY_QCOM_PCIE2='y',
        PHY_QCOM_QMP='y',
        PHY_QCOM_QUSB2='y',
        PINCTRL_MSM='y',
        PINCTRL_IPQ6018='y',
        PINCTRL_QCOM_SPMI_PMIC='y',
        PINCTRL_SINGLE='y',
        PM='y',
        PM_SLEEP='y',
        POWER_RESET_MSM='y',
        POWER_SUPPLY_HWMON='y',
        QCOM_BAM_DMA='y',
        QCOM_FALKOR_ERRATUM_1003='y',
        QCOM_FALKOR_ERRATUM_1009='y',
        QCOM_QDF2400_ERRATUM_0065='y',
        QCOM_FALKOR_ERRATUM_E1041='y',
        QCOM_Q6V5_ADSP='y',
        QCOM_Q6V5_MSS='y',
        QCOM_Q6V5_PAS='y',
        QCOM_Q6V5_WCSS='y',
        QCOM_SYSMON='y',
        QCOM_WCNSS_PIL='y',
        QCOM_AOSS_QMP='y',
        QCOM_COMMAND_DB='y',
        QCOM_GENI_SE='y',
        QCOM_GSBI='y',
        QCOM_LLCC='y',
        QCOM_OCMEM='y',
        QCOM_RMTFS_MEM='y',
        QCOM_RPMH='y',
        QCOM_SMEM='y',
        QCOM_SMD_RPM='y',
        QCOM_SMP2P='y',
        QCOM_SMSM='y',
        QCOM_SOCINFO='y',
        QCOM_WCNSS_CTRL='y',
        QCOM_APR='y',
        QCOM_A53PLL='y',
        QCOM_CLK_APCS_MSM8916='y',
        QCOM_CLK_APCC_MSM8996='y',
        QCOM_CLK_RPM='y',
        QCOM_APCS_IPC='y',
        QCOM_IPCC='y',
        QCOM_IOMMU='y',
        QCOM_QFPROM='y',
        QCOM_TSENS='y',
        QCOM_EBI2='y',
        REGULATOR='y',
        REGULATOR_FIXED_VOLTAGE='y',
        REGULATOR_GPIO='y',
        REGULATOR_QCOM_RPM='y',
        REGULATOR_QCOM_SPMI='y',
        REMOTEPROC='y',
        RPMSG_CHAR='y',
        RPMSG_QCOM_GLINK_RPM='y',
        RPMSG_QCOM_GLINK_SMEM='y',
        RPMSG_QCOM_SMD='y',
        SERIAL_MSM='y',
        SERIAL_MSM_CONSOLE='y',
        SECCOMP='y',
        SOCIONEXT_SYNQUACER_PREITS='y',
        SPI='y',
        SPI_QUP='y',
        SPMI='y',
        SPMI_MSM_PMIC_ARB='y',
        SUSPEND='y',
        THERMAL='y',
        THERMAL_HWMON='y',
        THERMAL_OF='y',
        THERMAL_WRITABLE_TRIPS='y',
        THERMAL_GOV_USER_SPACE='y',
        ZONE_DMA='y',
        ),
    user = dict(
        BOOT_UBOOT_QUALCOMM_IPQ='y',
        BOOT_UBOOT_QUALCOMM_IPQ_TARGET='ipq6018',
        ),
    )

groups['hardware_ipq_nss'] = dict(
    linux = dict(
        BRIDGE_NETFILTER='y',
        NF_CONNTRACK_EVENTS='y',
        NF_CONNTRACK_CHAIN_EVENTS='y',
        NETFILTER_XT_MATCH_PHYSDEV='m',
        ),
    modules = dict(
        MODULES_QCA_IPQ='y',
        MODULES_QCA_SSDK='y',
        MODULES_QCA_NSS_DP='y',
        MODULES_QCA_NSS_DRV='y',
        MODULES_QCA_NSS_ECM='y',
        )
    )

groups['hardware_ex50'] = dict(
    include = [
        'hardware_ipq6018',
        'hardware_ipq_nss',
        'linux_pci_qcom',
        'linux_usb_qcom',
        ],
    linux = dict(
        AT803X_PHY='y',
        CONFIGFS_FS='y',
        EXT4_FS='y',
        EXT4_USE_FOR_EXT2='y',
        GPIO_CDEV = 'y',
        GPIO_CDEV_V1 = 'y',
        GPIO_PCF857X='y',
        HW_RANDOM='y',
        HW_RANDOM_SHA204='y',
        I2C_GPIO='y',
        LEDS_GPIO = 'y',
        LEDS_TRIGGER_ONESHOT='y',
        LEDS_TRIGGER_ACTIVITY='y',
        LEDS_TRIGGER_GPIO='y',
        LEDS_TRIGGER_NETDEV='y',
        LEDS_TRIGGER_SYNC='y',
        MEMORY='y',
        MTD_CFI='y',
        MTD_CFI_INTELEXT='y',
        MTD_CFI_AMDSTD='y',
        MTD_OF_PARTS='y',
        MTD_SPI_NOR='m',
        MTD_RAW_NAND='y',
        MTD_NAND_QCOM='y',
        MTD_UBI='y',
        MTD_UBI_GLUEBI='y',
        NVMEM_SYSFS='y',
        PTP_1588_CLOCK='y',
        RTC_NVMEM='y',
        RTC_DRV_PCF8563='y',
        SCSI_PROC_FS='y',
        SCSI_LOWLEVEL='y',
        SHORTCUT_FE='m',
        SNAPDOG='y',
        SWCONFIG='y',
        SYSFS_PERSISTENT_MEM='y',
        USB_STORAGE='y',
        VFAT_FS='y',
        ),
    user = dict(
        USER_I2C_TOOLS='y',
        PROP_SIM_SIM='y',
        PROP_SECUREBOOT='y',
        PROP_SECUREBOOT_PROGRAM_ECC508='y',
        PROP_SECUREBOOT_TOOLS='y',
        USER_MTD_UTILS_NANDWRITE='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_KERNEL_DRIVER='y',
        USER_NETFLASH_MINIMUM_VERSION='21.2.0',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        PROP_QCA_NSS='y',
        PROP_QCA_NSS_FIRMWARE='y',
        ),
    )

groups['linux_32bit'] = dict(
    linux = {
        '32BIT':'y'
        }
    )

groups['linux_64bit'] = dict(
    linux = {
        '64BIT':'y'
        }
    )

groups['linux_x86_base'] = dict(
    include = [
        'linux_smp',
         ],
    linux = dict(
        IA32_EMULATION='y',
        KERNEL_XZ='y',
        RELOCATABLE='y',
        RANDOMIZE_BASE='y',
        RANDOMIZE_MEMORY='y',
        LOG_BUF_SHIFT='17',
        LOG_CPU_MAX_BUF_SHIFT='12',
        ELF_CORE='y',
        CORE_DUMP_DEFAULT_ELF_HEADERS='y',
        DEBUG_BUGVERBOSE='y',
        KALLSYMS='y',
        SYSVIPC='y',
        SGETMASK_SYSCALL='y',
        SYSFS_SYSCALL='y',
        BPF_SYSCALL='y',
        FHANDLE='y',
        USELIB='y',
        FUTEX='y',
        AIO='y',
        DMIID='y',
        PERF_EVENTS='y',
        VM_EVENT_COUNTERS='y',
        COMPAT_BRK='n',
        COMPAT_32BIT_TIME='y',
        MSDOS_PARTITION='y',
        EFI_PARTITION='y',
        EXT4_FS='y',
        EXT3_FS='y',
        EXT4_USE_FOR_EXT2='y',
        OVERLAY_FS='y',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='y',
        ISO9660_FS='y',
        JOLIET='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE='437',
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        ARCH_MMAP_RND_BITS='28',
        ARCH_MMAP_RND_COMPAT_BITS='8',
        X86_INTEL_MEMORY_PROTECTION_KEYS='y',
        UNWINDER_ORC='y',
        STACKPROTECTOR='y',
        STACKPROTECTOR_STRONG='y',
        VMAP_STACK='y',
        JUMP_LABEL='y',
        UID16='y',
        RSEQ='y',
        ),
    user = dict(
        LIB_INSTALL_LIBGCC_S='y'
        ),
    )

groups['linux_x86'] = dict(
    include = [
        'linux_x86_base',
         ],
    linux = dict(
        POSIX_MQUEUE='y',
        KALLSYMS_ALL='y',
        RD_BZIP2='y',
        RD_LZMA='y',
        RD_XZ='y',
        RD_LZO='y',
        RD_LZ4='y',
        SOFTLOCKUP_DETECTOR='y',
        ),
    )

groups['hardware_x86_base'] = dict(
    include = [
        'linux_64bit',
        'linux_pci_x86',
        'linux_rtc',
        ],
    linux = dict(
        HZ_PERIODIC='y',
        HIGH_RES_TIMERS='y',
        ISA_DMA_API='y',
        CPU_IDLE_GOV_MENU='y',
        BLK_DEV_BSG='y',
        CROSS_MEMORY_ATTACH='y',
        STANDALONE='y',
        FW_LOADER='y',
        FIRMWARE_MEMMAP='y',
        EXTRA_FIRMWARE='',
        SCSI='y',
        SCSI_PROC_FS='y',
        BLK_DEV_SD='y',
        X86_MPPARSE='y',
        X86_PM_TIMER='y',
        CC_OPTIMIZE_FOR_PERFORMANCE='y',
        SCHED_OMIT_FRAME_POINTER='y',
        HYPERVISOR_GUEST='y',
        GENERIC_CPU='y',
        CPU_SUP_INTEL='y',
        CPU_SUP_AMD='y',
        CPU_SUP_CENTAUR='y',
        CPU_IDLE='y',
        DMI='y',
        SCHED_MC='y',
        X86_VSYSCALL_EMULATION='y',
        X86_MSR='y',
        X86_CPUID='y',
        SPARSEMEM_VMEMMAP='y',
        MTRR='y',
        MTRR_SANITIZER='y',
        MTRR_SANITIZER_ENABLE_DEFAULT='0',
        MTRR_SANITIZER_SPARE_REG_NR_DEFAULT='1',
        ARCH_RANDOM='y',
        X86_PLATFORM_DEVICES='y',
        X86_FEATURE_NAMES='y',
        EFI=['y', 'n'],
        HZ_250='y',
        PHYSICAL_ALIGN='0x1000000',
        PINCTRL='y',
        LEGACY_VSYSCALL_NONE='y',
        COMPAT_VDSO='y',
        ACPI_REV_OVERRIDE_POSSIBLE='y',
        INPUT='y',
        INPUT_KEYBOARD='y',
        INPUT_LEDS='y',
        SERIAL_8250='y',
        SERIAL_8250_DEPRECATED_OPTIONS='y',
        SERIAL_8250_CONSOLE='y',
        SERIAL_8250_PNP='y',
        EARLY_PRINTK='y',
        PNP='y',
        RAS='y',
        RTC_DRV_CMOS='y',
        ATA_ACPI='y',
        SATA_AHCI='y',
        SATA_MOBILE_LPM_POLICY='0',
        HW_RANDOM='y',
        HW_RANDOM_INTEL='y',
        HW_RANDOM_AMD='y',
        HW_RANDOM_VIA='y',
        POWER_SUPPLY='y',
        HWMON='y',
        THERMAL='y',
        THERMAL_HWMON='y',
        THERMAL_DEFAULT_GOV_STEP_WISE='y',
        THERMAL_EMERGENCY_POWEROFF_DELAY_MS='0',
        IO_DELAY_0X80='y',
        NEW_LEDS='y',
        LEDS_CLASS='y',
        LEDS_TRIGGERS='y',
        SSB='y',
        SSB_PCIHOST='y',
        SSB_DRIVER_PCICORE='y',
        IOMMU_SUPPORT='y',
        AMD_IOMMU='y',
        AMD_IOMMU_V2='y',
        INTEL_IOMMU='y',
        INTEL_IOMMU_DEFAULT_ON='y',
        IRQ_REMAP='y',
        PPS='y',
        DNOTIFY='y',
        DEBUG_MEMORY_INIT='y',
        CRYPTO_NULL='y',
        CRYPTO_MD4='y',
        CRYPTO_HW='y',
        CRYPTO_DEV_PADLOCK='y',
        CRYPTO_DEV_PADLOCK_AES='y',
        CRYPTO_DEV_PADLOCK_SHA='y',
        CRYPTO_SHA512='y',
        CRC16='y',
        ),
    )

groups['hardware_x86'] = dict(
    include = [
        'hardware_x86_base',
        ],
    linux = dict(
        MODULE_FORCE_UNLOAD='y',
        PROCESSOR_SELECT='y',
        PHYSICAL_START='0x100000',
        USB_XHCI_HCD='y',
        USB_EHCI_HCD_PLATFORM='y',
        USB_OHCI_HCD='y',
        USB_OHCI_HCD_PCI='y',
        USB_UHCI_HCD='y',
        HARDLOCKUP_DETECTOR='y',
        SWCONFIG='y',
        PTP_1588_CLOCK='y',
        USB_STORAGE='y',
        USB_PCI='y',
        CRYPTO_MANAGER_DISABLE_TESTS='y',
        XZ_DEC_X86='y',
        ),
    )

groups['linux_container'] = dict(
    linux = dict(
        NAMESPACES='y',
        PID_NS='y',
        IPC_NS='y',
        USER_NS='y',
        UTS_NS='y',
        NET_NS='y',
        CGROUPS='y',
        CGROUP_CPUACCT='y',
        CGROUP_DEVICE='y',
        CGROUP_FREEZER='y',
        CGROUP_PERF='y',
        CGROUP_SCHED='y',
        BLK_CGROUP='y',
        MEMCG='y',
        SWAP='y',
        MEMCG_SWAP='y',
        )
    )

groups['linux_kvm'] = dict(
    linux = dict(
        VIRTUALIZATION='y',
        KVM='y',
        KVM_INTEL='y',
        KVM_AMD='y',
        VHOST_MENU='y',
        VHOST_NET='m',
        VIRT_DRIVERS='y',
        VIRTIO_MENU='y',
        VIRTIO_BLK='y',
        VIRTIO_CONSOLE='y',
        VIRTIO_PCI='y',
        VIRTIO_PCI_LEGACY='y',
        VIRTIO_BALLOON='y',
        VIRTIO_MMIO='y',
        VIRTIO_NET='y',
        VETH='y',
        VMXNET3='y',
        HYPERV='y',
        HYPERV_BALLOON='y',
        HYPERV_NET='y',
        NLMON='y',
        TASKSTATS='y',
        ),
    )

groups['linux_acpi'] = dict(
    linux = dict(
        ACPI='y',
        ACPI_AC='y',
        ACPI_BATTERY='y',
        ACPI_BUTTON='y',
        ACPI_FAN='y',
        ACPI_PROCESSOR='y',
        ACPI_PROCESSOR_AGGREGATOR='y',
        ACPI_THERMAL='y',
        ACPI_TABLE_UPGRADE='y',
        ),
    )

groups['hardware_bcm53118'] = dict(
    linux = dict(
        BROADCOM_BCM53118='y',
        ),
    user = dict(
        PROP_BCM53118='y',
        PROP_SWTEST_SWCONFIG='y',
        PROP_SWTEST_SWTEST='y',
        PROP_SWTEST_SWITCH_NUM='1',
        PROP_SWTEST_BCM53118='y',
        )
    )

groups['hardware_88e6350'] = dict(
    user = dict(
        PROP_SWTEST_SWCONFIG='y',
        PROP_SWTEST_SWTEST='y',
        PROP_SWTEST_SWITCH_NUM='1',
        PROP_SWTEST_MVL88E6350='y',
        )
    )

groups['hardware_88e6390'] = dict(
    user = dict(
        PROP_SWTEST_SWCONFIG='y',
        PROP_SWTEST_SWTEST='y',
        PROP_SWTEST_SWITCH_NUM='1',
        PROP_SWTEST_MVL88E6390='y',
        )
    )

groups['hardware_nand'] = dict(
    linux = dict(
        UBIFS_FS='y',
        ),
    user = dict(
        USER_MTD_UTILS='y',
        USER_MTD_UTILS_ERASE='y',
        USER_MTD_UTILS_NANDDUMP='y',
        USER_MTD_UTILS_UBIUPDATEVOL='y',
        USER_MTD_UTILS_UBIMKVOL='y',
        USER_MTD_UTILS_UBIRMVOL='y',
        USER_MTD_UTILS_UBINFO='y',
        USER_MTD_UTILS_UBIATTACH='y',
        USER_MTD_UTILS_UBIDETACH='y',
        USER_MTD_UTILS_UBIFORMAT='y',
        USER_MTD_UTILS_UBIRENAME='y',
        USER_MTD_UTILS_MTDINFO='y',
        USER_MTD_UTILS_UBIRSVOL='y',
        USER_UTIL_LINUX_LIBUUID='y',
        )
    )

groups['hardware_nor'] = dict(
    linux = dict(
        MTD='y',
        MTD_CMDLINE_PARTS='y',
        MTD_COMPLEX_MAPPINGS='y',
        ),
    user = dict(
        USER_MTD_UTILS='y',
        USER_MTD_UTILS_ERASE='y',
        USER_MTD_UTILS_ERASEALL='y',
        USER_MTD_UTILS_LOCK='y',
        USER_MTD_UTILS_UNLOCK='y',
        )
    )

groups['hardware_cellular_external'] = dict(
    include = [
        'linux_usb_cellular_internal',
        ],
    linux = dict(
        USB_SIERRA_NET='m',
        USB_NET_SIERRA='m',
        USB_SIERRA_FORCE_QMI_CONFIG='y',
        ),
    user = dict(
        USER_USB_MODESWITCH='y',
        ),
    )

groups['hardware_cellular_sierra'] = dict(
    include = [
        'linux_usb_cellular_internal',
        ],
    linux = dict(
        SYSVIPC='y',
        ),
    user = dict(
        PROP_SIERRA='y',
        PROP_USBRESET_USBRESET='y',
        USER_LIBQMI_QMI_FIRMWARE_UPDATE='y',
        ),
    )

groups['hardware_cellular_telit'] = dict(
    include = [
        'linux_usb_cellular_internal',
        ],
    user = dict(
        PROP_TELIT='y',
        C_PLUS_PLUS='y',
        PROP_TELIT_CUSTOM_FIRMWARE='y',
        ),
    )

groups['hardware_cellular_quectel'] = dict(
    include = [
        'linux_usb_cellular_internal',
        ],
    user = dict(
        PROP_QUECTEL='y',
        )
    )

groups['hardware_cellular_unitac'] = dict(
    include = [
        'linux_usb_cellular_internal',
        ],
    user = dict(
        PROP_UNITAC='y',
        )
    )

groups['hardware_cellular_thales'] = dict(
    include = [
        'linux_usb_cellular_internal',
    ],
    user = dict(
        PROP_THALES='y',
    )
)

groups['hardware_cellular_cm'] = dict(
    include = [
        'hardware_cellular_sierra',
        'hardware_cellular_telit',
        'hardware_cellular_quectel',
        'hardware_cellular_unitac'
        ],
    user = dict(
        PROP_SIERRA_SDK='y',
        )
    )

groups['hardware_bluetooth'] = dict(
    include = [
        ],
    linux = dict(
        BT='y',
        BT_BREDR='y',
        BT_RFCOMM='y',
        BT_RFCOMM_TTY='y',
        BT_BNEP='y',
        BT_BNEP_MC_FILTER='y',
        BT_BNEP_PROTO_FILTER='y',
        BT_HS='y',
        BT_LE='y',
        BT_DEBUGFS='y',
        BT_HCIUART='y',
        BT_HCIUART_H4='y',
        DEBUG_FS='y',
        )
    )

groups['hardware_bluetooth_usb'] = dict(
    include = [
        ],
    linux = dict(
        BT='y',
        BT_BREDR='y',
        BT_HS='y',
        BT_LE='y',
        BT_HCIBTUSB='m',
        BT_HCIBTUSB_BCM='y',
        BT_HCIBTUSB_RTL='y',
        )
    )

groups['hardware_wireless'] = dict(
    include = [
        'linux_bridge',
        ],
    linux = dict(
        WIRELESS='y',
        WIRELESS_EXT='y',
        WEXT_PRIV='y',
        SYSTEM_TRUSTED_KEYS="",
        CFG80211='y',
        CFG80211_DEFAULT_PS='y',
        CFG80211_WEXT='y',
        CFG80211_CRDA_SUPPORT='y',
        CFG80211_CERTIFICATION_ONUS='y',
        CFG80211_REQUIRE_SIGNED_REGDB='y',
        CFG80211_USE_KERNEL_REGDB_KEYS='y',
        MAC80211='y',
        MAC80211_RC_MINSTREL='y',
        MAC80211_RC_DEFAULT_MINSTREL='y',
        MAC80211_LEDS='y',
        WLAN='y',
        KEYS='y',
        ),
    user = dict(
        USER_IW='y',
        USER_WIRELESS_TOOLS='y',
        USER_WIRELESS_TOOLS_IWCONFIG='y',
        USER_WIRELESS_TOOLS_IWGETID='y',
        USER_WIRELESS_TOOLS_IWLIST='y',
        USER_WIRELESS_TOOLS_IWPRIV='y',
        USER_WIRELESS_TOOLS_IWSPY='y',
        USER_WPA_SUPPLICANT='y',
        USER_WPA_PASSPHRASE='y',
        USER_CRDA='y',
        PROP_CONFIG_WPA_V1=['y', 'n'],
        )
    )

groups['hardware_wireless_ath9k'] = dict(
    include = [
        'hardware_wireless',
        ],
    linux = dict(
        WLAN_VENDOR_ATH='y',
        ATH9K_BTCOEX_SUPPORT='y',
        ATH9K='m',
        ATH9K_PCI='y',
        ATH9K_PCOEM='y',
        )
    )

groups['hardware_wireless_ath10k'] = dict(
    include = [
        'hardware_wireless',
        ],
    linux = dict(
        WLAN_VENDOR_ATH='y',
        ATH10K='m',
        ATH10K_PCI='m',
        ATH10K_DFS_CERTIFIED='y',
        ATH_REG_NO_FORCED_RADAR='y',
        ATH_REG_FORCE_DEFAULT='y',
        )
    )

groups['hardware_wireless_ath10k_sdio'] = dict(
    include = [
        'hardware_wireless',
        ],
    linux = dict(
        WLAN_VENDOR_ATH='y',
        ATH10K='m',
        ATH10K_SDIO='m',
        ATH10K_DFS_CERTIFIED='y',
        ATH_REG_NO_FORCED_RADAR='y',
        ATH_REG_FORCE_DEFAULT='y',
        ),
    user = dict(
        USER_ATH10K_FIRMWARE='y',
        )
    )

groups['hardware_wireless_ath11k'] = dict(
    include = [
        'hardware_wireless',
        ],
    linux = dict(
        CRYPTO_MICHAEL_MIC='y',
        WLAN_VENDOR_ATH='y',
        ATH11K='m',
        ATH11K_PCI='m',
        MHI_BUS='m',
        FW_LOADER_USER_HELPER='y',
        FW_LOADER_USER_HELPER_FALLBACK='y'
        ),
    user = dict(
        USER_ATH11K_FIRMWARE='y',
        )
)

groups['hardware_wireless_mediatek'] = dict(
    include = [
        'hardware_wireless',
        ],
    linux = dict(
        WLAN_VENDOR_MEDIATEK='y',
        MT76x2E='m',
        MT7603E='m',
        MT76_LEDS='n'
        ),
    user = dict(
        USER_MT76='y'
        ),
    )

groups['hardware_wireless_mwifiex_sdio'] = dict(
    include = [
        'hardware_wireless',
        ],
    linux = dict(
        WLAN_VENDOR_MARVELL='y',
        MWIFIEX='m',
        MWIFIEX_SDIO='m',
        )
    )

groups['hardware_wireless_ax210'] = dict(
    include = [
        'hardware_wireless',
        ],
    linux = dict(
        WLAN_VENDOR_INTEL='y',
        IWLWIFI='m',
        IWLWIFI_LEDS='y',
        IWLMVM='m',
        IWLWIFI_OPMODE_MODULAR='y',
        IWLWIFI_DEBUG='y'
        ),
    user = dict(
        USER_LINUX_FIRMWARE='y',
        USER_LINUX_FIRMWARE_AX210='y'
        ),
    )

groups['hardware_wireless_qca_wifi'] = dict(
    include = [
        'hardware_wireless',
        ],
    linux = dict(
        RELAY='y',
        QRTR='y',
        QRTR_SMD='y',
        QRTR_MHI='y',
        NL80211_TESTMODE='y',
        CNSS2='y',
        CNSS2_GENL='y',
        ARM_PMU='y',
        CORESIGHT='y',
        MODULE_TAINT_OVERRIDE='y',
        ),
    modules = dict(
        MODULES_QCA_WIFI='y',
        ),
    user = dict(
        PROP_QCA_WIFI='y',
        PROP_QCA_WIFI_FIRMWARE='y',
        PROP_QCA_WIFI_CFG80211TOOL='y',
        USER_HOSTAPD_QCA_EXTENSIONS='y',
        )
    )

groups['hardware_power'] = dict(
    user = dict(
        PROP_POWER='y',
    )
)

groups['hardware_accelerometer'] = dict(
    linux = dict(
        IIO='y',
        IIO_BUFFER='y',
        IIO_KFIFO_BUF='y',
        IIO_TRIGGERED_BUFFER='y',
        IIO_TRIGGER='y',
        IIO_CONSUMERS_PER_TRIGGER='2',
        IIO_ST_LIS2HH12='y',
        IIO_ST_LIS2HH12_I2C='y',
        )
    )

groups['wireless_ixx4'] = dict(
    include = [
        'config_wireless',
        ],
    linux = dict(
        CFG80211='m',
        CFG80211_CRDA_SUPPORT='y',
        CFG80211_DEFAULT_PS='y',
        WEXT_PRIV='y',
        WIRELESS='y',
        WIRELESS_EXT='y',
        WLAN='y',
        WLAN_VENDOR_INTERSIL='y',
        ),
    user = dict(
        USER_CRDA='n',
        USER_CRDA_REGDB='y',
        USER_HOSTAPD_HOSTAPD_CLI='y',
        USER_IW='y',
        USER_WIRELESS_TOOLS='y',
        USER_WIRELESS_TOOLS_IWCONFIG='y',
        USER_WIRELESS_TOOLS_IWLIST='y',
        USER_WIRELESS_TOOLS_IWPRIV='y',
        USER_WPA_CLI='y',
        USER_WPA_SUPPLICANT='y',
        USER_WPA_PASSPHRASE='y',
        )
    )

groups['hardware_virtio'] = dict(
    linux = dict(
        PARAVIRT='y',
        KVM_GUEST='y',
        VIRTIO_MENU='y',
        VIRTIO_BLK='y',
        VIRTIO_NET='y',
        VIRTIO_CONSOLE='y',
        HW_RANDOM_VIRTIO='y',
        VIRT_DRIVERS='y',
        VIRTIO_PCI='y',
        VIRTIO_PCI_LEGACY='y',
        PTP_1588_CLOCK_KVM='y',
        )
    )

groups['hardware_5400'] = dict(
    include = [
        'hardware_nand',
        'linux_arm',
        'linux_usb_external',
        'linux_pci',
        'linux_usb_net_external',
        ],
    linux = dict(
        ARM_PATCH_PHYS_VIRT='y',
        ARCH_MULTI_V5='y',
        AUTO_ZRELADDR='y',
        SYSVIPC='y',
        POSIX_MQUEUE='y',
        HZ_PERIODIC='y',
        LOG_BUF_SHIFT='14',
        CC_OPTIMIZE_FOR_SIZE='y',
        KALLSYMS='y',
        FUTEX='y',
        AIO='y',
        PCI_QUIRKS='y',
        COMPAT_BRK='y',
        MSDOS_PARTITION='y',
        ARCH_MULTIPLATFORM='y',
        ARCH_MVEBU='y',
        MACH_KIRKWOOD='y',
        MACH_5400_RM_DT='y',
        CPU_FEROCEON_OLD_ID='y',
        ARM_THUMB='y',
        CACHE_FEROCEON_L2='y',
        PCI_MVEBU='y',
        PCI_MSI='y',
        HZ_100='y',
        AEABI='y',
        UACCESS_WITH_MEMCPY='y',
        ARM_APPENDED_DTB='y',
        ARM_ATAG_DTB_COMPAT='y',
        ARM_ATAG_DTB_COMPAT_CMDLINE_FROM_BOOTLOADER='y',
        CMDLINE='console=ttyS0,115200',
        CMDLINE_FROM_BOOTLOADER='y',
        UNCOMPRESS_INCLUDE='../mach-mvebu/include/mach/uncompress.h',
        STANDALONE='y',
        MTD_CMDLINE_PARTS='y',
        MTD_OF_PARTS='y',
        MTD_RAW_NAND='y',
        MTD_NAND_ORION='y',
        MTD_NAND_ECC_SW_HAMMING='y',
        MTD_NAND_ECC_SW_BCH='y',
        MTD_UBI='y',
        MTD_UBI_WL_THRESHOLD=4096,
        MTD_UBI_BEB_LIMIT=20,
        MTD_UBI_GLUEBI='y',
        SCSI='y',
        SCSI_PROC_FS='y',
        BLK_DEV_SD='y',
        BLK_DEV_SR='m',
        NET_VENDOR_MARVELL='y',
        MV643XX_ETH='y',
        MVMDIO='y',
        MARVELL_PHY='y',
        SNAPDOG='y',
        SERIAL_8250='y',
        SERIAL_8250_DEPRECATED_OPTIONS='y',
        SERIAL_8250_CONSOLE='y',
        SERIAL_8250_PCI='m',
        SERIAL_8250_NR_UARTS=8,
        SERIAL_8250_RUNTIME_UARTS=8,
        SERIAL_OF_PLATFORM='y',
        PINMUX='y',
        PINCONF='y',
        POWER_SUPPLY='y',
        POWER_RESET='y',
        POWER_RESET_GPIO='y',
        REGULATOR='y',
        REGULATOR_FIXED_VOLTAGE='y',
        USB_PCI='y',
        USB_EHCI_HCD_ORION='y',
        USB_OHCI_HCD='y',
        USB_OHCI_HCD_PCI='y',
        USB_UHCI_HCD='y',
        USB_SERIAL='y',
        USB_SERIAL_ARK3116='y',
        IOMMU_SUPPORT='y',
        EXT3_FS='y',
        EXT4_FS='y',
        EXT4_USE_FOR_EXT2='y',
        DNOTIFY='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE=437,
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        JFFS2_FS='y',
        JFFS2_FS_DEBUG='0',
        JFFS2_FS_WRITEBUFFER='y',
        SQUASHFS_ZLIB='y',
        ARM_UNWIND='y',
        CRYPTO_AUTHENC='y',
        CRYPTO_MD4='y',
        CRYPTO_BLOWFISH='y',
        CRYPTO_DES='y',
        CRYPTO_LZO='y',
        CRC16='y',
        XZ_DEC_X86='y',
        XZ_DEC_POWERPC='y',
        XZ_DEC_IA64='y',
        XZ_DEC_ARM='y',
        XZ_DEC_ARMTHUMB='y',
        XZ_DEC_SPARC='y',
        BLK_DEV_RAM_SIZE=16384,
        ),
    user = dict(
        BOOT_UBOOT='y',
        BOOT_UBOOT_TARGET='5400-rm',
        USER_SIGS_SIGS='y',
        USER_ETHTOOL_ETHTOOL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_BUSYBOX_GETTY='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_EMCTEST_EMCTEST='y',
        USER_BUSYBOX_LSPCI='y',
        USER_KWBOOTFLASH_KWBOOTFLASH='y',
        )
    )

groups['hardware_5400_rm'] = dict(
    include = [
        'hardware_5400',
        'hardware_cellular_sierra',
        'hardware_cellular_external',
        ],
    )

groups['hardware_5401_rm'] = dict(
    include = [
        'hardware_5400',
        ],
    )

groups['hardware_connectit_mini'] = dict(
    include = [
        'hardware_imx6',
        'hardware_nand',
        'hardware_cellular_telit',
        'linux_usb_external',
        'linux_usb_net_internal',
        'linux_usb_storage',
        ],
    linux = dict(
        SNAPDOG='y',
        SNAPDOG_GPIO_BANK='2',
        SNAPDOG_GPIO_BIT='16',
        SNAPDOG_BOOT_TIMEOUT='120',
        BLK_DEV_RAM_SIZE=30720,
        LOG_BUF_SHIFT='17',
        UNCOMPRESS_INCLUDE='../mach-imx/include/mach/uncompress.h',
        CMDLINE='console=null',
        COMPAT_BRK='y',
        STANDALONE='y',
        SCSI_PROC_FS='y',
        EXT4_FS='y',
        EXT3_FS='y',
        EXT4_USE_FOR_EXT2='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE='437',
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        DEBUG_MEMORY_INIT='y',
        I2C='y',
        I2C_CHARDEV='y',
        I2C_IMX='y',
        I2C_MUX='y',
        NVMEM='y',
        NVMEM_SYSFS='y',
        NVMEM_IMX_OCOTP='y',
        HW_RANDOM_SHA204='y',
        HW_RANDOM_SHA204_LOCKCFG='n',
        ),
    user = dict(
        BOOT_UBOOT='y',
        BOOT_UBOOT_TARGET='connectit-mini',
        USER_I2C_TOOLS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_KERNEL_DRIVER='y',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        USER_SIGS_SIGS='y',
        PROP_SECUREBOOT='y',
        PROP_SECUREBOOT_PROGRAM_ECC508='y',
        PROP_SECUREBOOT_TOOLS='y',
        PROP_IMX_FUSES='y',
        POOR_ENTROPY='n',
        )
    )

groups['hardware_ex12'] = dict(
    include = [
        'hardware_imx6',
        'hardware_nand',
        'hardware_cellular_telit',
        'linux_usb_net_internal',
        'linux_usb_storage',
        ],
    linux = dict(
        SNAPDOG='y',
        SNAPDOG_GPIO_BANK='1',
        SNAPDOG_GPIO_BIT='28',
        SNAPDOG_BOOT_TIMEOUT='120',
        BLK_DEV_RAM_SIZE=30720,
        LOG_BUF_SHIFT='17',
        UNCOMPRESS_INCLUDE='../mach-imx/include/mach/uncompress.h',
        CMDLINE='console=null',
        COMPAT_BRK='y',
        STANDALONE='y',
        SCSI_PROC_FS='y',
        EXT4_FS='y',
        EXT3_FS='y',
        EXT4_USE_FOR_EXT2='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE='437',
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        DEBUG_MEMORY_INIT='y',
        I2C='y',
        I2C_CHARDEV='y',
        I2C_IMX='y',
        I2C_MUX='y',
        NVMEM='y',
        NVMEM_SYSFS='y',
        NVMEM_IMX_OCOTP='y',
        HW_RANDOM_SHA204='y',
        HW_RANDOM_SHA204_LOCKCFG='n',
        MMC='y',
        PWRSEQ_EMMC='y',
        PWRSEQ_SIMPLE='y',
        MMC_BLOCK='y',
        MMC_BLOCK_MINORS='8',
        MMC_SDHCI='y',
        MMC_SDHCI_IO_ACCESSORS='y',
        MMC_SDHCI_PLTFM='y',
        MMC_SDHCI_ESDHC_IMX='y',
        MMC_CQHCI='y',
        ),
    user = dict(
        BOOT_UBOOT='y',
        BOOT_UBOOT_TARGET='ex12',
        USER_I2C_TOOLS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_KERNEL_DRIVER='y',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        USER_SIGS_SIGS='y',
        PROP_SECUREBOOT='y',
        PROP_SECUREBOOT_PROGRAM_ECC508='y',
        PROP_SECUREBOOT_TOOLS='y',
        PROP_IMX_FUSES='y',
        POOR_ENTROPY='n',
        )
    )

groups['hardware_ix'] = dict(
    include = [
        'hardware_imx6',
        'hardware_nand',
        'linux_usb_net_internal',
        'linux_usb_storage',
        ],
    linux = dict(
        SNAPDOG=['y', 'n'],
        SNAPDOG_GPIO_BANK=['4', ''],
        SNAPDOG_GPIO_BIT=['20', ''],
        SNAPDOG_BOOT_TIMEOUT=['120', ''],
        BLK_DEV_RAM_SIZE=46080,
        LOG_BUF_SHIFT='17',
        UNCOMPRESS_INCLUDE='../mach-imx/include/mach/uncompress.h',
        CMDLINE='console=null',
        COMPAT_BRK='y',
        STANDALONE='y',
        SCSI_PROC_FS='y',
        EXT4_FS='y',
        EXT3_FS='y',
        EXT4_USE_FOR_EXT2='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE='437',
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        DEBUG_MEMORY_INIT='y',
        IMX_SDMA='m',
        I2C='y',
        I2C_CHARDEV='y',
        I2C_IMX='y',
        I2C_MUX='y',
        NVMEM='y',
        NVMEM_SYSFS='y',
        NVMEM_IMX_OCOTP='y',
        SENSORS_LM75='y',
        HW_RANDOM_SHA204='y',
        HW_RANDOM_SHA204_LOCKCFG='n',
        ),
    user = dict(
        BOOT_UBOOT='y',
        USER_LINUX_FIRMWARE='y',
        USER_I2C_TOOLS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_KERNEL_DRIVER='y',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        USER_SIGS_SIGS='y',
        PROP_SECUREBOOT='y',
        PROP_SECUREBOOT_PROGRAM_ECC508='y',
        PROP_SECUREBOOT_TOOLS='y',
        PROP_IMX_FUSES='y',
        POOR_ENTROPY='n',
        )
    )

groups['hardware_ix10'] = dict(
    include = [
        'hardware_ix',
        'hardware_cellular_quectel',
        ],
    user = dict(
        BOOT_UBOOT_TARGET='ix10',
        )
    )

groups['hardware_ix15'] = dict(
    include = [
        'hardware_ix',
        'hardware_cellular_quectel',
        'hardware_imx6_mmc',
        'hardware_accelerometer',
        'linux_rtc',
        'linux_rtc_imx',
        ],
    linux = dict(
        SNAPDOG='n',
        SNAPDOG_GPIO_BANK='',
        SNAPDOG_GPIO_BIT='',
        SNAPDOG_BOOT_TIMEOUT='',
        ),
    user = dict(
        BOOT_UBOOT_TARGET='ix15',
        )
    )

groups['hardware_ix20'] = dict(
    include = [
        'hardware_ix',
        'hardware_cellular_cm',
        ],
    user = dict(
        BOOT_UBOOT_TARGET='ix20',
        )
    )

groups['hardware_ix20w'] = dict(
    include = [
        'hardware_ix20',
        'hardware_imx6_mmc',
        'hardware_wireless_ath10k_sdio',
        ],
    linux = dict(
        PWRSEQ_EMMC='y',
        PWRSEQ_SIMPLE='y',
        )
    )

groups['hardware_ix30'] = dict(
    include = [
        'hardware_ix',
        'hardware_cellular_cm',
        'linux_rtc',
        'linux_usb_external',
        ],
    linux=dict(
        REGULATOR_FIXED_VOLTAGE='y',
        RTC_DRV_DS1307='y',
        GPIO_CDEV='y',
        GPIO_CDEV_V1='y',
        GPIO_PCA953X='y',
        I2C_GPIO='y',
        IIO='y',
        MAX1363='y',
        IIO_CONSUMERS_PER_TRIGGER='2',
        GPIO_FABRICATED='y'
        ),
    user = dict(
        BOOT_UBOOT_TARGET='ix30',
        )
    )

groups['hardware_connectit4'] = dict(
    include = [
        'hardware_imx6',
        'hardware_nand',
        'hardware_cellular_cm',
        'linux_usb_net_internal',
        'linux_usb_storage',
        ],
    linux = dict(
        SNAPDOG='y',
        SNAPDOG_GPIO_BANK='2',
        SNAPDOG_GPIO_BIT='16',
        SNAPDOG_BOOT_TIMEOUT='120',
        BLK_DEV_RAM_SIZE=32768,
        LOG_BUF_SHIFT='17',
        UNCOMPRESS_INCLUDE='../mach-imx/include/mach/uncompress.h',
        CMDLINE='console=null',
        COMPAT_BRK='y',
        STANDALONE='y',
        SCSI_PROC_FS='y',
        EXT4_FS='y',
        EXT3_FS='y',
        EXT4_USE_FOR_EXT2='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE='437',
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        DEBUG_MEMORY_INIT='y',
        NVMEM='y',
        NVMEM_SYSFS='y',
        NVMEM_IMX_OCOTP='y',
        ),
    user = dict(
        BOOT_UBOOT_FREESCALE_IMX6='y',
        BOOT_UBOOT_FREESCALE_IMX6_TARGET='connectit4',
        )
    )

groups['hardware_connectit16'] = dict(
    include = [
        'hardware_connectit48',
        ]
    )

groups['hardware_connectit48'] = dict(
    include = [
        'hardware_x86',
        'hardware_cellular_cm',
        'linux_x86',
        'linux_acpi',
        'linux_kvm',
        'linux_container',
        'linux_usb_external',
        'linux_usb_net_internal',
        'linux_usb_storage',
        'linux_hugepages',
        'linux_sata',
        ],
    linux = dict(
        NR_CPUS='4',
        IOMMU_SUPPORT='y',
        INTEL_IOMMU='y',
        INTEL_IOMMU_DEFAULT_ON='y',
        NET_VENDOR_INTEL='y',
        IXGBE='y',
        SENSORS_CORETEMP='y',
        SENSORS_ACPI_POWER='y',
        SENSORS_NCT6775='y',
        RTC_DRV_CMOS_RAW_ALARM_BYTES='y',
        BLK_DEV_RAM_SIZE=65536,
        LOG_BUF_SHIFT='17',
        STANDALONE='y',
        SCSI_PROC_FS='y',
        EXT4_FS='y',
        EXT3_FS='y',
        EXT4_USE_FOR_EXT2='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE='437',
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        DEBUG_MEMORY_INIT='y',
        SERIAL_8250_NR_UARTS='64',
        SERIAL_8250_RUNTIME_UARTS='64',
        SERIAL_8250_PCI='y',
        SERIAL_8250_EXAR='y',
        SERIAL_8250_EXTENDED='y',
        SERIAL_8250_MANY_PORTS='y',
        SERIAL_8250_SHARE_IRQ='y',
        INPUT_EVDEV='y',
        HW_RANDOM_SHA204='y',
        HW_RANDOM_SHA204_LOCKCFG='n',
        # pstore ramdisk support
        CMDLINE_BOOL='y',
        CMDLINE='memmap=64k!256M ramoops.mem_address=0x10000000 ramoops.mem_size=0x10000 ramoops.ecc=1 ramoops.console_size=0x1000 ramoops.record_size=0x1000',
        ),
    user = dict(
        USER_GRUB='y',
        USER_GRUB_EFI='y',
        USER_GRUB_ENVTOOLS='y',
        USER_GRUB_DISABLE_VGA_CONSOLE='y',
        USER_UBOOT_ENVTOOLS='n',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_KERNEL_DRIVER='y',
        USER_NETFLASH_ATECC508A_ALG_ECDSA='y',
        USER_NETFLASH_EMBEDDED_KERNEL='y',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        USER_I2C_TOOLS='y',
        PROP_SECUREBOOT='y',
        PROP_SECUREBOOT_PROGRAM_ECC508='y',
        PROP_SECUREBOOT_TOOLS='y',
        USER_BUSYBOX_GETTY='y',
        USER_LEDCMD_LEDCMD='n',
        PROP_GPIO_NEXCOM_MCU='y',
        USER_BUSYBOX_ACPID='y',
        USER_BUSYBOX_FEATURE_ACPID_COMPAT='y',
        POOR_ENTROPY='n',
        PROP_CONFIG_POWER_MONITOR='y',
        )
    )

groups['hardware_virtualdal'] = dict(
    include = [
        'hardware_x86',
        'linux_x86',
        'linux_acpi',
        'linux_kvm',
        'linux_container',
        'linux_hugepages',
        'linux_sata',
        'linux_usb',
        'linux_net_e1000',
        ],
    linux = dict(
        NR_CPUS='4',
        IOMMU_SUPPORT='y',
        INTEL_IOMMU='y',
        INTEL_IOMMU_DEFAULT_ON='y',
        NET_VENDOR_INTEL='y',
        IXGBE='y',
        E1000='y',
        E1000E='y',
        SENSORS_CORETEMP='y',
        SENSORS_ACPI_POWER='y',
        SENSORS_NCT6775='y',
        RTC_DRV_CMOS_RAW_ALARM_BYTES='y',
        BLK_DEV_RAM_SIZE=65536,
        LOG_BUF_SHIFT='17',
        STANDALONE='y',
        SCSI_PROC_FS='y',
        EXT4_FS='y',
        EXT3_FS='y',
        EXT4_USE_FOR_EXT2='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE='437',
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        DEBUG_MEMORY_INIT='y',
        INPUT_EVDEV='y',
        KEYBOARD_ATKBD='y',
        SERIO_SERPORT='y',
        HYPERV_KEYBOARD='y',
        VT='y',
        EFI='n',
        CONSOLE_TRANSLATIONS='y',
        VT_CONSOLE='y',
        VGA_ARB='y',
        DRM='y',
        DRM_FBDEV_EMULATION='y',
        DRM_BOCHS='y',
        FB='y',
        FB_VESA='y',
        VGA_CONSOLE='y',
        FRAMEBUFFER_CONSOLE='y',
        HID='y',
        HID_GENERIC='y',
        ),
    user = dict(
        USER_GRUB='y',
        USER_GRUB_ENVTOOLS='y',
        USER_UBOOT_ENVTOOLS='n',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_BUSYBOX_GETTY='y',
        USER_LEDCMD_LEDCMD='n',
        USER_BUSYBOX_ACPID='y',
        USER_BUSYBOX_FEATURE_ACPID_COMPAT='y',
        )
    )

groups['hardware_6300_cx'] = dict(
    include = [
        'hardware_armada_370',
        'hardware_nand',
        'hardware_cellular_sierra',
        ],
    linux = dict(
        MACH_6300CX='y',
        MICREL_PHY='y',
        MARVELL_PHY='y',
        REALTEK_PHY='y',
        BLK_DEV_RAM_SIZE=32768,
        SERIAL_8250_NR_UARTS='4',
        SERIAL_8250_RUNTIME_UARTS='4',
        SNAPDOG='y',
        ),
    user = dict(
        USER_ETHTOOL_ETHTOOL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_BUSYBOX_GETTY='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_EMCTEST_EMCTEST='y',
        USER_BUSYBOX_LSPCI='y',
        BOOT_UBOOT_MARVELL_370='y',
        BOOT_UBOOT_MARVELL_370_TARGET='ac6300cx',
        )
    )

groups['hardware_6310_dx'] = dict(
    include = [
        'hardware_imx6',
        'hardware_nand',
        'hardware_cellular_cm',
        'linux_usb_net_internal',
        'linux_usb_storage',
        ],
    linux = dict(
        SNAPDOG='y',
        SNAPDOG_GPIO_BANK='2',
        SNAPDOG_GPIO_BIT='16',
        SNAPDOG_BOOT_TIMEOUT='120',
        BLK_DEV_RAM_SIZE=32768,
        LOG_BUF_SHIFT='17',
        UNCOMPRESS_INCLUDE='../mach-imx/include/mach/uncompress.h',
        CMDLINE='console=null',
        COMPAT_BRK='y',
        STANDALONE='y',
        SCSI_PROC_FS='y',
        EXT4_FS='y',
        EXT3_FS='y',
        EXT4_USE_FOR_EXT2='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE='437',
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        DEBUG_MEMORY_INIT='y',
        NVMEM='y',
        NVMEM_SYSFS='y',
        NVMEM_IMX_OCOTP='y',
        ),
    user = dict(
        BOOT_UBOOT_FREESCALE_IMX6='y',
        BOOT_UBOOT_FREESCALE_IMX6_TARGET='ac6310dx',
        )
    )

groups['hardware_ix14-base'] = dict(
    include = [
        'hardware_imx6',
        'hardware_nand',
        'hardware_cellular_telit',
        'linux_rtc',
        'linux_container',
        'hardware_bluetooth',
        'hardware_accelerometer',
        ],
    linux = dict(
        ALLOW_DEV_COREDUMP='y',
        ARM_ARCH_TIMER_EVTSTREAM='y',
        ARM_PMU='y',
        ARM_THUMB='y',
        BLK_DEV_RAM_SIZE=46080,
        BOUNCE='y',
        CMDLINE='console=null',
        FAIR_GROUP_SCHED='y',
        CFS_BANDWIDTH='y',
        BLK_DEBUG_FS='y',
        CMA='y',
        COMPACTION='y',
        CONFIGFS_FS='m',
        CONNECTOR='y',
        CPU_IDLE='y',
        CRC_T10DIF='y',
        DEBUG_FS='y',
        DEVMEM='y',
        DMA_CMA='y',
        IMX_SDMA='m',
        EEPROM_AT24='y',
        GPIO_MCA_CC6UL='y',
        HAVE_ARM_ARCH_TIMER='y',
        HIGHMEM='y',
        HIGHPTE='y',
        HWMON='y',
        I2C='y',
        I2C_ALGOBIT='y',
        I2C_CHARDEV='y',
        I2C_IMX='y',
        I2C_MUX='y',
        IKCONFIG_PROC='y',
        IKCONFIG='y',
        IMX_WEIM='y',
        LEDS_GPIO='y',
        LEDS_LP5569='y',
        LEDS_TRIGGER_BACKLIGHT='y',
        LEDS_TRIGGER_GPIO='y',
        LEDS_TRIGGER_HEARTBEAT='y',
        LEDS_TRIGGER_ONESHOT='y',
        LEDS_TRIGGERS='y',
        LEDS_TRIGGER_TIMER='y',
        LOG_BUF_SHIFT='18',
        MCA_CC6UL_ADC='y',
        MCA_CC6UL_WATCHDOG='y',
        MFD_MCA_CC6UL='y',
        MIGRATION='y',
        NEON='y',
        NETFILTER_XT_MATCH_CGROUP='m',
        NET_VENDOR_FREESCALE='y',
        NETWORK_FILESYSTEMS='y',
        NEW_LEDS='y',
        NOP_USB_XCEIV='y',
        NVMEM='y',
        NVMEM_SYSFS='y',
        PERF_EVENTS='y',
        PM_DEBUG='y',
        POSIX_MQUEUE='y',
        POWER_RESET_SYSCON_POWEROFF='y',
        POWER_RESET='y',
        POWER_SUPPLY='y',
        PPP_MULTILINK='y',
        PROC_EVENTS='y',
        PWM_IMX27='y',
        PWM='y',
        PM='y',
        RELAY='y',
        NVMEM_IMX_OCOTP='y',
        RTC_DRV_MXC='y',
        RTC_DRV_SNVS='y',
        RTC_INTF_DEV_UIE_EMUL='y',
        RTC_DRV_MCA_CC6UL='y',
        RTC_NVMEM='y',
        RT_GROUP_SCHED='y',
        SECCOMP='y',
        SENSORS_LM73='y',
        SMSC_PHY='y',
        STRICT_KERNEL_RWX='y',
        STRICT_MODULE_RWX='y',
        SUSPEND='y',
        SWAP='y',
        SYSFS_SYSCALL='y',
        SYSTEM_TRUSTED_KEYS='',
        SYSVIPC='y',
        THERMAL_DEFAULT_GOV_STEP_WISE='y',
        THERMAL_EMERGENCY_POWEROFF_DELAY_MS='0',
        THERMAL_HWMON='y',
        THERMAL_OF='y',
        THERMAL='y',
        CPU_THERMAL='y',
        IMX_THERMAL='y',
        UEVENT_HELPER_PATH='',
        UEVENT_HELPER='y',
        UNCOMPRESS_INCLUDE='debug/uncompress.h',
        EXTCON='y',
        VDSO='y',
        VF610_ADC='y',
        VFAT_FS='y',
        WATCHDOG_HANDLE_BOOT_ENABLED='y',
        WATCHDOG_NOWAYOUT='y',
        WATCHDOG='y',
        WATCHDOG_STOP_ON_REBOOT=0,
        SECURITYFS='y',
        SECURITY_NETWORK='y',
        SECURITY_PATH='y',
        AUDIT='y',
        MODVERSIONS='y',
        MODULE_SRCVERSION_ALL='y',
        KALLSYMS_ALL='y',
        FHANDLE='y',
        REGULATOR_ANATOP='y',
        REGULATOR_FIXED_VOLTAGE='y',
        REGULATOR_GPIO='y',
        REGULATOR_PFUZE100='y',
        REGULATOR='y',
        UBIFS_FS_XATTR='y',
        UBIFS_FS_SECURITY='y',
        CRYPTO_CRC32C='y',
        CRYPTO_DEV_FSL_CAAM_AHASH_API='y',
        CRYPTO_DEV_FSL_CAAM_CRYPTO_API='y',
        CRYPTO_DEV_FSL_CAAM_JR='y',
        CRYPTO_DEV_FSL_CAAM_PKC_API='y',
        CRYPTO_DEV_FSL_CAAM_RINGSIZE='9',
        CRYPTO_DEV_FSL_CAAM_RNG_API='y',
        CRYPTO_DEV_FSL_CAAM='y',
        CRYPTO_HW='y',
        CRYPTO_MD4='y',
        CRYPTO_MD5='y',
        CRYPTO_SHA1='y',
        CRYPTO_SHA512='y'
        ),
    user = dict(
        BOOT_UBOOT_DIGI_PRODUCTION='y',
        BOOT_UBOOT_DIGI_TARGET='ix14',
        BOOT_UBOOT_DIGI_VERSION='5bc67edc07e649a0f734bd23b546d9fcaf440a59',
        BOOT_UBOOT_DIGI='y',
        LIB_LIBFTDI='y',
        POOR_ENTROPY='n',
        PROP_MTST='y',
        PROP_PREBUILT='y',
        PROP_PREBUILT_DIGI_MCA_TOOL='y',
        PROP_SIM_SIM='y',
        USER_BUSYBOX_FEATURE_FANCY_SLEEP='y',
        USER_BUSYBOX_FLOAT_DURATION='y',
        USER_BUSYBOX_STRINGS='y',
        USER_EMCTEST_EMCTEST='y',
        USER_ETHTOOL_ETHTOOL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_FLATFSD_MCA_UBOOTENV_FACTORYRESET='y',
        USER_I2C_TOOLS='y',
        USER_IMX_KOBS='y',
        USER_LEDCMD_LEDCMD='n',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_I2C_BUS='0',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        USER_SIGS_SIGS='y',
        USER_LINUX_FIRMWARE='y',
        )
    )

groups['hardware_ix14'] = dict(
    include = [
        'hardware_ix14-base',
        ]
    )

groups['hardware_mt7621'] = dict(
    include = [
        'hardware_nand',
        'linux_mips',
        'linux_smp',
        'linux_32bit',
        'linux_pci_mt7621',
        'linux_i2c',
        'linux_usb_net_internal',
        'linux_usb_storage',
        'linux_rtc',
        ],
    linux = dict(
        RALINK='y',
        SOC_MT7621='y',
        PINCTRL_RALINK='y',
        PINCTRL_MT7621='y',
        MIPS_NO_APPENDED_DTB='y',
        MIPS_CMDLINE_FROM_BOOTLOADER='y',
        CPU_LITTLE_ENDIAN='y',
        MIPS_MT_SMP='y',
        SCHED_SMT='y',
        PCPU_DEV_REFCNT='y',
        MIPS_MT_FPAFF='y',
        MIPS_CPS='y',
        CLK_MT7621='y',
        CPU_NEEDS_NO_SMARTMIPS_OR_MICROMIPS='y',
        CPU_ISOLATION='y',
        NR_CPUS='4',
        RCU_CPU_STALL_TIMEOUT='21',
        HZ_100='y',
        HZ_PERIODIC='y',
        HIGH_RES_TIMERS='y',
        LOG_BUF_SHIFT='17',
        LOG_CPU_MAX_BUF_SHIFT='12',
        CC_OPTIMIZE_FOR_PERFORMANCE='y',
        STACKPROTECTOR_STRONG='y',
        KALLSYMS='y',
        ELF_CORE='y',
        CORE_DUMP_DEFAULT_ELF_HEADERS='y',
        AIO='y',
        VM_EVENT_COUNTERS='y',
        COMPAT_BRK='y',
        MODULE_FORCE_UNLOAD='y',
        STANDALONE='y',
        FW_LOADER='y',
        EXTRA_FIRMWARE='',
        BLK_DEV_BSG='y',
        MSDOS_PARTITION='y',
        EFI_PARTITION='y',
        FUTEX='y',
        CROSS_MEMORY_ATTACH='y',
        RELAY='y',
        BLK_DEV_RAM_SIZE=32768,
        MTD_CMDLINE_PARTS='y',
        MTD_OF_PARTS='y',
        MTD_RAW_NAND='y',
        MTD_NAND_MT7621='y',
        MTD_UBI='y',
        MTD_UBI_WL_THRESHOLD='4096',
        MTD_UBI_BEB_LIMIT='20',
        MTD_UBI_GLUEBI='y',
        I2C_MT7621_MANUAL='y',
        GPIO_MT7621='y',
        GPIO_PCF857X='y',
        NET_SWITCHDEV='y',
        SWCONFIG='y',
        SCSI='y',
        SCSI_PROC_FS='y',
        BLK_DEV_SD='y',
        STAGING='y',
        NET_VENDOR_MEDIATEK='y',
        NET_MEDIATEK_SOC='y',
        MEDIATEK_GE_PHY='y',
        USB_PCI='y',
        USB_OHCI_HCD='y',
        USB_OHCI_HCD_PCI='y',
        USB_EHCI_HCD_PLATFORM='y',
        USB_XHCI_HCD='y',
        USB_XHCI_PLATFORM='y',
        SERIAL_8250='y',
        SERIAL_8250_CONSOLE='y',
        SERIAL_8250_NR_UARTS='3',
        SERIAL_8250_RUNTIME_UARTS='3',
        SERIAL_OF_PLATFORM='y',
        RTC_DRV_CMOS='y',
        RESET_CONTROLLER='y',
        MFD_SYSCON='y',
        NEW_LEDS='y',
        LEDS_CLASS='y',
        LEDS_TRIGGERS='y',
        EXT4_FS='y',
        EXT3_FS='y',
        EXT4_USE_FOR_EXT2='y',
        DNOTIFY='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE='437',
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        DEBUG_MEMORY_INIT='y',
        CRYPTO_SHA512='y',
        CRYPTO_ANSI_CPRNG='m',
        CRYPTO_CCM='y',
        CRYPTO_GCM='y',
        WATCHDOG='y',
        WATCHDOG_STOP_ON_REBOOT=0,
        HW_RANDOM='y',
        ),
    user = dict(
        USER_ETHTOOL_ETHTOOL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_LSPCI='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_LINUX_FIRMWARE='y',
        USER_EMCTEST_EMCTEST='y',
        USER_SIGS_SIGS='y',
        LIB_LIBFTDI='y',
        PROP_MTST='y',
        )
    )

groups['hardware_ex15'] = dict(
    include = [
        'hardware_mt7621',
        'hardware_cellular_cm',
        ],
    linux = dict(
        DTB_MT7621_EX15='y',
        USB_XHCI_MTK='y',
        NET_DSA='y',
        NET_DSA_MT7530='y',
        SNAPDOG='y',
        MEMORY='y',
        SYSFS_PERSISTENT_MEM='y',
        ),
    user = dict(
        PROP_SIM_SIM='y',
        BOOT_UBOOT='y',
        BOOT_UBOOT_TARGET='ex15',
        USER_NETFLASH_CRYPTO_ECDSA_SW='y',
        USER_NETFLASH_PUBKEY_KERNEL_CMDLINE='y',
        USER_NETFLASH_CRYPTO_ECDSA_SW_OPTIONAL='y',
        USER_NETFLASH_DEVMODE_KERNEL_CMDLINE='y',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        )
    )

groups['hardware_ex15w'] = dict(
    include = [
        'hardware_ex15',
        'hardware_wireless_ath10k',
        ]
    )

groups['hardware_lr54'] = dict(
    include = [
        'hardware_mt7621',
        'hardware_cellular_sierra',
        'hardware_cellular_telit',
        ],
    linux = dict(
        GPIO_CDEV = 'y',
        GPIO_CDEV_V1 = 'y',
        DTB_MT7621_LR54='y',
        USB_XHCI_MTK='y',
        NET_DSA='y',
        NET_DSA_MT7530='y',
        MT7621_WDT='y',
        SQUASHFS_ZLIB='y',
        HWMON='y',
        SENSORS_LM73='y',
        USB_SERIAL_CP210X='y',
        RTC_DRV_DS1307='y',
        INPUT='y',
        INPUT_LEDS='y',
        HW_RANDOM_SHA204='y',
        HW_RANDOM_SHA204_LOCKCFG='n',
        I2C_CHARDEV='y',
        LEDS_TRIGGER_TIMER='y',
        LEDS_GPIO='y',
        MEMORY='y',
        SYSFS_PERSISTENT_MEM='y',
        JFFS2_FS='y',
        JFFS2_FS_DEBUG='0',
        JFFS2_FS_WRITEBUFFER='y',
        JFFS2_FS_XATTR='y',
        ),
    modules = dict(
        MODULES_MTK_EIP93='y',
        ),
    user = dict(
        USER_BUSYBOX_LONG_OPTS='y',
        BOOT_UBOOT='y',
        BOOT_UBOOT_TARGET='lr54',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_ALG_HMAC='y',
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=6,
        USER_NETFLASH_ATECC508A_KERNEL_DRIVER='y',
        USER_NETFLASH_DEVMODE_KERNEL_CMDLINE='y',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        USER_BUSYBOX_HEXDUMP='y',
        USER_LEDCMD_LEDCMD='n',
        PROP_LEDCMD_SYSFS='y',
        PROP_SIM_SIM='y',
        POOR_ENTROPY='n',
        )
    )

products['Digi EX50'] = dict(
    vendor = 'Digi',
    product = 'EX50',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_ex50',
        'hardware_nor',
        'hardware_nand',
        'hardware_cellular_sierra',
        'hardware_wireless_qca_wifi',
        'hardware_accelerometer',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        'config_ex50',
        'config_cellular',
        'config_nemo',
        'config_wireless',
        'config_bonding',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        ],
        user = dict(
            LIB_OPENSSL3='y',
            LIB_OPENSSL_FIPS='y',
            PROP_SIERRA_SDK='y',
            PROP_CONFIG_WPA3_ENTERPRISE="y",
            PROP_5G_SUPPORT="y",
        )
    )

groups['hardware_lr54_minimal'] = dict(
    include = [
        'hardware_nand',
        'linux_mips',
        'linux_smp',
        'linux_32bit',
        'linux_pci_mt7621',
        'linux_i2c',
        'linux_rtc',
        ],
    linux = dict(
        GPIO_CDEV = 'y',
        GPIO_CDEV_V1 = 'y',
        LOCALVERSION_AUTO='y',
        SYSVIPC='y',
        BLK_DEV_INITRD='y',
        RD_LZMA='y',
        MULTIUSER='y',
        POSIX_TIMERS='y',
        PRINTK='y',
        BUG='y',
        BASE_FULL='y',
        EPOLL='y',
        SIGNALFD='y',
        TIMERFD='y',
        EVENTFD='y',
        SHMEM='y',
        ADVISE_SYSCALLS='y',
        EMBEDDED='y',
        SLUB_DEBUG='y',
        SLAB_MERGE_DEFAULT='y',
        MODULES='y',
        MODULE_UNLOAD='y',
        BLOCK='y',
        PARTITION_ADVANCED='y',
        BINFMT_ELF='y',
        BINFMT_SCRIPT='y',
        COREDUMP='y',
        DEVTMPFS='y',
        DEVTMPFS_MOUNT='y',
        PREVENT_FIRMWARE_BUILD='y',
        MTD='y',
        MTD_BLOCK='y',
        BLK_DEV='y',
        BLK_DEV_LOOP='y',
        BLK_DEV_RAM='y',
        BLK_DEV_RAM_COUNT='4',
        TTY='y',
        UNIX98_PTYS='y',
        GPIO_SYSFS='y',
        USB_SUPPORT='y',
        USB='y',
        USB_ANNOUNCE_NEW_DEVICES='y',
        USB_DEFAULT_PERSIST='y',
        USB_EHCI_HCD='y',
        USB_EHCI_ROOT_HUB_TT='y',
        USB_EHCI_TT_NEWSCHED='y',
        USB_SERIAL='y',
        GENERIC_PHY='y',
        FILE_LOCKING='y',
        INOTIFY_USER='y',
        PROC_FS='y',
        PROC_SYSCTL='y',
        PROC_PAGE_MONITOR='y',
        SYSFS='y',
        TMPFS='y',
        MISC_FILESYSTEMS='y',
        SQUASHFS='y',
        SQUASHFS_XZ='y',
        NLS_CODEPAGE_437='y',
        NLS_ISO8859_1='y',
        COREDUMP_PRINTK='y',
        KEYS='y',
        CRYPTO_RSA='y',
        CRYPTO_MANAGER_DISABLE_TESTS='y',
        CRYPTO_AUTHENC='y',
        CRYPTO_ECHAINIV='y',
        CRYPTO_CBC='y',
        CRYPTO_ECB='y',
        CRYPTO_CMAC='y',
        CRYPTO_MD5='y',
        CRYPTO_SHA1='y',
        CRYPTO_DES='y',
        ASYMMETRIC_KEY_TYPE='y',
        ASYMMETRIC_PUBLIC_KEY_SUBTYPE='y',
        X509_CERTIFICATE_PARSER='y',
        PKCS7_MESSAGE_PARSER='y',
        SYSTEM_TRUSTED_KEYRING='y',
        CRC_CCITT='y',
        SECTION_MISMATCH_WARN_ONLY='y',
        PANIC_TIMEOUT='-1',
        RALINK='y',
        SOC_MT7621='y',
        PINCTRL_RALINK='y',
        MIPS_NO_APPENDED_DTB='y',
        MIPS_CMDLINE_FROM_BOOTLOADER='y',
        CPU_LITTLE_ENDIAN='y',
        MIPS_MT_SMP='y',
        SCHED_SMT='y',
        MIPS_MT_FPAFF='y',
        MIPS_CPS='y',
        CPU_NEEDS_NO_SMARTMIPS_OR_MICROMIPS='y',
        CPU_ISOLATION='y',
        NR_CPUS='4',
        RCU_CPU_STALL_TIMEOUT='21',
        HZ_100='y',
        HZ_PERIODIC='y',
        HIGH_RES_TIMERS='y',
        LOG_BUF_SHIFT='17',
        LOG_CPU_MAX_BUF_SHIFT='12',
        CC_OPTIMIZE_FOR_PERFORMANCE='y',
        STACKPROTECTOR_STRONG='y',
        KALLSYMS='y',
        ELF_CORE='y',
        CORE_DUMP_DEFAULT_ELF_HEADERS='y',
        AIO='y',
        VM_EVENT_COUNTERS='y',
        COMPAT_BRK='y',
        MODULE_FORCE_UNLOAD='y',
        STANDALONE='y',
        FW_LOADER='y',
        EXTRA_FIRMWARE='',
        BLK_DEV_BSG='y',
        MSDOS_PARTITION='y',
        EFI_PARTITION='y',
        FUTEX='y',
        CROSS_MEMORY_ATTACH='y',
        RELAY='y',
        BLK_DEV_RAM_SIZE=32768,
        MTD_CMDLINE_PARTS='y',
        MTD_OF_PARTS='y',
        MTD_RAW_NAND='y',
        MTD_NAND_MT7621='y',
        MTD_UBI='y',
        MTD_UBI_WL_THRESHOLD='4096',
        MTD_UBI_BEB_LIMIT='20',
        MTD_UBI_GLUEBI='y',
        I2C_MT7621_MANUAL='y',
        GPIOLIB='y',
        GPIO_MT7621='y',
        NET_SWITCHDEV='n',
        SWCONFIG='n',
        SCSI='y',
        SCSI_PROC_FS='y',
        BLK_DEV_SD='y',
        STAGING='y',
        USB_PCI='y',
        USB_XHCI_HCD='y',
        USB_XHCI_PLATFORM='y',
        SERIAL_8250='y',
        SERIAL_8250_CONSOLE='y',
        SERIAL_8250_NR_UARTS='3',
        SERIAL_8250_RUNTIME_UARTS='3',
        SERIAL_OF_PLATFORM='y',
        RTC_DRV_CMOS='y',
        RESET_CONTROLLER='y',
        MFD_SYSCON='y',
        NEW_LEDS='y',
        LEDS_CLASS='y',
        LEDS_GPIO='y',
        LEDS_TRIGGERS='y',
        LEDS_TRIGGER_TIMER='y',
        DNOTIFY='y',
        DEBUG_MEMORY_INIT='y',
        CRYPTO_SHA512='y',
        CRYPTO_ANSI_CPRNG='m',
        CRYPTO_CCM='y',
        CRYPTO_GCM='y',
        WATCHDOG='y',
        WATCHDOG_STOP_ON_REBOOT=0,
        HW_RANDOM='y',
        MT7621_WDT='y',
        SQUASHFS_ZLIB='y',
        USB_SERIAL_CP210X='y',
        RTC_DRV_DS1307='y',
        HW_RANDOM_SHA204='y',
        HW_RANDOM_SHA204_LOCKCFG='n',
        MEMORY='y',
        SYSFS_PERSISTENT_MEM='y',
        JFFS2_FS='y',
        JFFS2_FS_DEBUG='0',
        JFFS2_FS_WRITEBUFFER='y',
        JFFS2_FS_XATTR='y',
        UEVENT_HELPER='y',
        UEVENT_HELPER_PATH=""
        ),
    user = dict(
        USER_ETHTOOL_ETHTOOL='n',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_BUSYBOX_LONG_OPTS='y',
        USER_BUSYBOX_LSPCI='y',
        USER_BUSYBOX_SEQ='y',
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_MDEV='y',
        USER_MTD_UTILS_NANDWRITE='y',
        POOR_ENTROPY='n',
        )
    )

groups['hardware_tx54'] = dict(
    include = [
        'hardware_mt7621',
        'hardware_cellular_sierra',
        'hardware_cellular_telit',
        'hardware_cellular_quectel',
        'hardware_cellular_thales',
        'hardware_wireless_ath10k',
        'gnss_hardware',
        'hardware_power',
        'hardware_cellular_external',
        'linux_usb_external'
        ],
    linux = dict(
        GPIO_CDEV = 'y',
        GPIO_CDEV_V1 = 'y',
        DTB_MT7621_TX54='y',
        NET_DSA='y',
        NET_DSA_MT7530='y',
        MT7621_WDT='y',
        SQUASHFS_ZLIB='y',
        HWMON='y',
        SENSORS_LM73='y',
        USB_SERIAL_CP210X='y',
        PINCTRL_MCP23S08='y',
        RTC_DRV_DS3232='y',
        RTC_NVMEM='y',
        NVMEM_SYSFS='y',
        INPUT='y',
        INPUT_LEDS='y',
        INPUT_MISC='y',
        INPUT_ADXL34X='y',
        INPUT_ADXL34X_I2C='y',
        HW_RANDOM_SHA204='y',
        HW_RANDOM_SHA204_LOCKCFG='n',
        MFD_MCU_TX54='y',
        INPUT_EVDEV='y',
        INPUT_MCU_TX54_KEYS='y',
        I2C_CHARDEV='y',
        POWER_RESET='y',
        POWER_RESET_MCU_TX54='y',
        POWER_SUPPLY='y',
        BATTERY_MCU_TX54='y',
        LEDS_MCU_TX54='y',
        LEDS_TRIGGER_TIMER='y',
        LEDS_GPIO='y',
        ),
    modules = dict(
        MODULES_MTK_EIP93='y',
        ),
    user = dict(
        USER_BUSYBOX_LONG_OPTS='y',
        BOOT_UBOOT='y',
        BOOT_UBOOT_TARGET='tx54',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_ALG_ECDSA='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_KERNEL_DRIVER='y',
        USER_NETFLASH_DEVMODE_KERNEL_CMDLINE='y',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        PROP_TX54_MCU='y',
        USER_BUSYBOX_ACPID='y',
        USER_BUSYBOX_FEATURE_ACPID_COMPAT='y',
        USER_BUSYBOX_HEXDUMP='y',
        USER_LEDCMD_LEDCMD='n',
        PROP_LEDCMD_SYSFS='y',
        POOR_ENTROPY='n',
        )
    )

groups['hardware_tx54_minimal'] = dict(
    include = [
        'hardware_nand',
        'linux_mips',
        'linux_smp',
        'linux_32bit',
        'linux_pci_mt7621',
        'linux_i2c',
        'linux_rtc',
        ],
    linux = dict(
        GPIO_CDEV = 'y',
        GPIO_CDEV_V1 = 'y',
        LOCALVERSION_AUTO='y',
        SYSVIPC='y',
        BLK_DEV_INITRD='y',
        RD_LZMA='y',
        MULTIUSER='y',
        POSIX_TIMERS='y',
        PRINTK='y',
        BUG='y',
        BASE_FULL='y',
        EPOLL='y',
        SIGNALFD='y',
        TIMERFD='y',
        EVENTFD='y',
        SHMEM='y',
        ADVISE_SYSCALLS='y',
        EMBEDDED='y',
        SLUB_DEBUG='y',
        SLAB_MERGE_DEFAULT='y',
        MODULES='y',
        MODULE_UNLOAD='y',
        BLOCK='y',
        PARTITION_ADVANCED='y',
        BINFMT_ELF='y',
        BINFMT_SCRIPT='y',
        COREDUMP='y',
        DEVTMPFS='y',
        DEVTMPFS_MOUNT='y',
        PREVENT_FIRMWARE_BUILD='y',
        MTD='y',
        MTD_BLOCK='y',
        BLK_DEV='y',
        BLK_DEV_LOOP='y',
        BLK_DEV_RAM='y',
        BLK_DEV_RAM_COUNT='4',
        INPUT_MISC='y',
        INPUT_ADXL34X='y',
        INPUT_ADXL34X_I2C='y',
        TTY='y',
        UNIX98_PTYS='y',
        LEDMAN='y',
        GPIO_SYSFS='y',
        HWMON='y',
        SENSORS_LM73='y',
        USB_SUPPORT='y',
        USB='y',
        USB_ANNOUNCE_NEW_DEVICES='y',
        USB_DEFAULT_PERSIST='y',
        USB_EHCI_HCD='y',
        USB_EHCI_ROOT_HUB_TT='y',
        USB_EHCI_TT_NEWSCHED='y',
        USB_ACM='y',
        USB_WDM='y',
        USB_SERIAL='y',
        USB_SERIAL_GENERIC='y',
        USB_SERIAL_SIMPLE='y',
        USB_SERIAL_IPW='y',
        USB_SERIAL_QCAUX='y',
        USB_SERIAL_QUALCOMM='y',
        USB_SERIAL_OPTION='y',
        USB_SERIAL_QT2='y',
        GENERIC_PHY='y',
        FILE_LOCKING='y',
        INOTIFY_USER='y',
        OVERLAY_FS='y',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='y',
        PROC_FS='y',
        PROC_SYSCTL='y',
        PROC_PAGE_MONITOR='y',
        SYSFS='y',
        TMPFS='y',
        MISC_FILESYSTEMS='y',
        SQUASHFS='y',
        SQUASHFS_XZ='y',
        NLS_CODEPAGE_437='y',
        NLS_ISO8859_1='y',
        COREDUMP_PRINTK='y',
        KEYS='y',
        CRYPTO_RSA='y',
        CRYPTO_MANAGER_DISABLE_TESTS='y',
        CRYPTO_AUTHENC='y',
        CRYPTO_ECHAINIV='y',
        CRYPTO_CBC='y',
        CRYPTO_ECB='y',
        CRYPTO_CMAC='y',
        CRYPTO_MD5='y',
        CRYPTO_SHA1='y',
        CRYPTO_DES='y',
        ASYMMETRIC_KEY_TYPE='y',
        ASYMMETRIC_PUBLIC_KEY_SUBTYPE='y',
        X509_CERTIFICATE_PARSER='y',
        PKCS7_MESSAGE_PARSER='y',
        SYSTEM_TRUSTED_KEYRING='y',
        CRC_CCITT='y',
        SECTION_MISMATCH_WARN_ONLY='y',
        PANIC_TIMEOUT='-1',
        RALINK='y',
        SOC_MT7621='y',
        PINCTRL_RALINK='y',
        MIPS_NO_APPENDED_DTB='y',
        MIPS_CMDLINE_FROM_BOOTLOADER='y',
        CPU_LITTLE_ENDIAN='y',
        MIPS_MT_SMP='y',
        SCHED_SMT='y',
        MIPS_MT_FPAFF='y',
        MIPS_CPS='y',
        CPU_NEEDS_NO_SMARTMIPS_OR_MICROMIPS='y',
        CPU_ISOLATION='y',
        NR_CPUS='4',
        RCU_CPU_STALL_TIMEOUT='21',
        HZ_100='y',
        HZ_PERIODIC='y',
        HIGH_RES_TIMERS='y',
        LOG_BUF_SHIFT='17',
        LOG_CPU_MAX_BUF_SHIFT='12',
        CC_OPTIMIZE_FOR_PERFORMANCE='y',
        STACKPROTECTOR_STRONG='y',
        KALLSYMS='y',
        ELF_CORE='y',
        CORE_DUMP_DEFAULT_ELF_HEADERS='y',
        AIO='y',
        VM_EVENT_COUNTERS='y',
        COMPAT_BRK='y',
        MODULE_FORCE_UNLOAD='y',
        STANDALONE='y',
        FW_LOADER='y',
        EXTRA_FIRMWARE='',
        BLK_DEV_BSG='y',
        MSDOS_PARTITION='y',
        EFI_PARTITION='y',
        FUTEX='y',
        CROSS_MEMORY_ATTACH='y',
        RELAY='y',
        BLK_DEV_RAM_SIZE=32768,
        MTD_CMDLINE_PARTS='y',
        MTD_OF_PARTS='y',
        MTD_RAW_NAND='y',
        MTD_NAND_MT7621='y',
        MTD_UBI='y',
        MTD_UBI_WL_THRESHOLD='4096',
        MTD_UBI_BEB_LIMIT='20',
        MTD_UBI_GLUEBI='y',
        I2C_MT7621_MANUAL='y',
        GPIO_MT7621='y',
        GPIO_PCF857X='y',
        NET_SWITCHDEV='n',
        SWCONFIG='n',
        SCSI='y',
        SCSI_PROC_FS='y',
        BLK_DEV_SD='y',
        STAGING='y',
        USB_PCI='y',
        USB_OHCI_HCD='y',
        USB_OHCI_HCD_PCI='y',
        USB_EHCI_HCD_PLATFORM='y',
        USB_XHCI_HCD='y',
        USB_XHCI_PLATFORM='y',
        SERIAL_8250='y',
        SERIAL_8250_CONSOLE='y',
        SERIAL_8250_NR_UARTS='3',
        SERIAL_8250_RUNTIME_UARTS='3',
        SERIAL_OF_PLATFORM='y',
        RESET_CONTROLLER='y',
        MFD_SYSCON='y',
        NEW_LEDS='y',
        LEDS_CLASS='y',
        LEDS_TRIGGERS='y',
        EXT4_FS='y',
        EXT3_FS='y',
        EXT4_USE_FOR_EXT2='y',
        DNOTIFY='y',
        MSDOS_FS='y',
        VFAT_FS='y',
        FAT_DEFAULT_CODEPAGE='437',
        FAT_DEFAULT_IOCHARSET='iso8859-1',
        DEBUG_MEMORY_INIT='y',
        CRYPTO_SHA512='y',
        CRYPTO_ANSI_CPRNG='m',
        CRYPTO_CCM='y',
        CRYPTO_GCM='y',
        WATCHDOG='y',
        WATCHDOG_STOP_ON_REBOOT=0,
        HW_RANDOM='y',
        MT7621_WDT='y',
        SQUASHFS_ZLIB='y',
        USB_SERIAL_CP210X='y',
        PINCTRL_MCP23S08='y',
        RTC_DRV_CMOS='y',
        RTC_DRV_DS3232='y',
        RTC_NVMEM='y',
        NVMEM_SYSFS='y',
        INPUT='y',
        INPUT_LEDS='y',
        HW_RANDOM_SHA204='y',
        HW_RANDOM_SHA204_LOCKCFG='n',
        UEVENT_HELPER='y',
        UEVENT_HELPER_PATH="",
        MFD_MCU_TX54='y',
        LEDS_GPIO='y',
        LEDS_MCU_TX54='y',
        LEDS_TRIGGER_TIMER='y',
        ),
    user = dict(
        USER_ETHTOOL_ETHTOOL='n',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_BUSYBOX_LONG_OPTS='y',
        USER_BUSYBOX_LSPCI='y',
        USER_BUSYBOX_SEQ='y',
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_MDEV='y',
        USER_MTD_UTILS_NANDWRITE='y',
        POOR_ENTROPY='n',
        )
    )

groups['hardware_tx64_minimal'] = dict(
    include = [
        'hardware_x86_base',
        'linux_x86_base',
        'linux_i2c',
        'linux_sata',
        'linux_hpet',
        'linux_hugepages',
        'linux_input_mousedev',
        'linux_acpi',
        ],
    linux = dict(
        X86_INTEL_LPSS='y',
        X86_CPU_RESCTRL='y',
        SPI='y',
        SENSORS_K8TEMP='y',
        SENSORS_K10TEMP='y',
        SENSORS_FAM15H_POWER='y',
        SENSORS_ACPI_POWER='y',
        CONSOLE_TRANSLATIONS='y',
        VT_CONSOLE='y',
        NVRAM='y',
        I2C_I801='y',
        I2C_ISCH='y',
        I2C_ISMT='y',
        I2C_PIIX4='y',
        I2C_SCMI='y',
        I2C_CHARDEV='y',
        RTC_DRV_CMOS_RAW_ALARM_BYTES='y',
        ACPI_I2C_OPREGION='y',
        NR_CPUS='4',
        SERIAL_8250_PCI='y',
        SERIAL_8250_EXAR='y',
        SERIAL_8250_EXTENDED='y',
        SERIAL_8250_NR_UARTS='8',
        SERIAL_8250_RUNTIME_UARTS='8',
        SERIAL_8250_MANY_PORTS='y',
        VT='y',
        VGA_CONSOLE='y',
        DUMMY_CONSOLE_COLUMNS='80',
        DUMMY_CONSOLE_ROWS='25',
        WATCHDOG_HANDLE_BOOT_ENABLED='n',
        WATCHDOG_NOWAYOUT='n',
        WATCHDOG='y',
        WATCHDOG_STOP_ON_REBOOT=0,
        LPC_ICH='y',
        ITCO_WDT='y',
        INPUT_EVDEV='y',
        HW_RANDOM_SHA204='y',
        HW_RANDOM_SHA204_LOCKCFG='n',
        I2C_DESIGNWARE_PCI='m',
        MODULES='y',
        ),
    user = dict(
        LIB_INSTALL_LIBATOMIC='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        USER_UBOOT_ENVTOOLS='n',
        USER_GRUB='y',
        USER_GRUB_ENVTOOLS='y',
        USER_LEDCMD_LEDCMD='n',
        PROP_GPIO_NEXCOM_MCU='y',
        USER_BUSYBOX_ACPID='y',
        USER_BUSYBOX_FEATURE_ACPID_COMPAT='y',
        POOR_ENTROPY='n',
        )
    )

groups['hardware_tx64'] = dict(
    include = [
        'hardware_tx64_minimal',
        'hardware_x86',
        'hardware_cellular_sierra',
        'hardware_cellular_telit',
        'hardware_cellular_quectel',
        'hardware_cellular_external',
        'hardware_wireless_ath10k',
        'gnss_hardware',
        'hardware_power',
        'linux_x86',
        'linux_net_e1000',
        'linux_net_r8169',
        'linux_net_i225',
        'linux_hid',
        'linux_uio',
        'linux_kvm',
        'linux_container',
        'linux_usb_external',
        ],
    linux = dict(
        BLK_DEV_RAM_SIZE=65536,
        I2C_ALGOBIT='y',
        CPU_FREQ='y',
        CPU_FREQ_DEFAULT_GOV_PERFORMANCE='y',
        X86_INTEL_PSTATE='y',
        # pstore ramdisk support
        CMDLINE_BOOL='y',
        CMDLINE='memmap=64k!256M ramoops.mem_address=0x10000000 ramoops.mem_size=0x10000 ramoops.ecc=1 ramoops.console_size=0x1000 ramoops.record_size=0x1000',
        ),
    user = dict(
        LIB_LIBPCIACCESS='y',
        USER_LIBQMI_QMI_NETWORK='y',
        USER_LINUX_FIRMWARE='y',
        USER_EMCTEST_EMCTEST='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_ALG_HMAC='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=1,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_KERNEL_DRIVER='y',
        USER_NETFLASH_EMBEDDED_KERNEL='y',
        USER_NETFLASH_DEVMODE_KERNEL_CMDLINE='y',
        USER_NETFLASH_VERIFY_FW_PRODUCT_INFO='y',
        USER_DISCARD_INETD_ECHO='y',
        USER_DISCARD_ECHO_NO_INSTALL='y',
        USER_SIGS_SIGS='y',
        USER_PROCPS='y',
        USER_PROCPS_TLOAD='y',
        USER_PROCPS_VMSTAT='y',
        USER_PROCPS_W='y',
        USER_PCIUTILS='y',
        USER_ETHTOOL_ETHTOOL='y',
        USER_LM_SENSORS='y',
        USER_BUSYBOX_GETTY='y',
        USER_GRUB_EFI='y',
        USER_GRUB_DISABLE_VGA_CONSOLE='y',
        USER_UTIL_LINUX_FSTRIM='y',
        POOR_ENTROPY='n',
        )
    )

groups['hardware_6335_mx'] = dict(
    include = [
        'hardware_armada_370',
        'hardware_nand',
        'hardware_88e6350',
        'hardware_cellular_cm',
        'hardware_cellular_external',
        'linux_pci_armada_370',
        'linux_i2c',
        ],
    linux = dict(
        MACH_6330MX='y',
        BLK_DEV_RAM_SIZE=32768,
        NET_SWITCHDEV='y',
        NET_DSA='y',
        NET_DSA_MV88E6XXX='y',
        MARVELL_PHY='y',
        FIXED_PHY='y',
        SERIAL_8250_NR_UARTS='4',
        SERIAL_8250_RUNTIME_UARTS='4',
        SNAPDOG='y',
        GPIO_PCF857X='y',
        I2C_MV64XXX='y',
        ),
    user = dict(
        USER_ETHTOOL_ETHTOOL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_BUSYBOX_GETTY='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_EMCTEST_EMCTEST='y',
        USER_BUSYBOX_LSPCI='y',
        USER_SIGS_SIGS='y',
        USER_LINUX_FIRMWARE='y',
        PROP_SIM_SIM='y',
        PROP_SWTEST_MII='y',
        LIB_LIBFTDI='y',
        PROP_MTST='y',
        BOOT_UBOOT_MARVELL_370='y',
        BOOT_UBOOT_MARVELL_370_TARGET='ac6350sr',
        )
    )

groups['hardware_6330_mx'] = dict(
    include = [
        'hardware_6335_mx',
        'hardware_wireless_ath9k',
        ],
    )

groups['hardware_6355_sr'] = dict(
    include = [
        'hardware_armada_370',
        'hardware_nand',
        'hardware_88e6350',
        'hardware_cellular_cm',
        'hardware_cellular_external',
        'linux_pci_armada_370',
        'linux_i2c',
        ],
    linux = dict(
        MACH_6350SR='y',
        BLK_DEV_RAM_SIZE=32768,
        NET_SWITCHDEV='y',
        NET_DSA='y',
        NET_DSA_MV88E6XXX='y',
        MARVELL_PHY='y',
        FIXED_PHY='y',
        SERIAL_8250_NR_UARTS='4',
        SERIAL_8250_RUNTIME_UARTS='4',
        SNAPDOG='y',
        GPIO_PCF857X='y',
        I2C_MV64XXX='y',
        ),
    user = dict(
        USER_ETHTOOL_ETHTOOL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_BUSYBOX_GETTY='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_EMCTEST_EMCTEST='y',
        USER_BUSYBOX_LSPCI='y',
        USER_SIGS_SIGS='y',
        USER_LINUX_FIRMWARE='y',
        PROP_SIM_SIM='y',
        PROP_SWTEST_MII='y',
        LIB_LIBFTDI='y',
        PROP_MTST='y',
        BOOT_UBOOT_MARVELL_370='y',
        BOOT_UBOOT_MARVELL_370_TARGET='ac6350sr',
        )
    )

groups['hardware_6350_sr'] = dict(
    include = [
        'hardware_6355_sr',
        'hardware_wireless_ath9k',
        ],
    )

groups['hardware_8300'] = dict(
    include = [
        'hardware_armada_370',
        'hardware_bcm53118',
        'hardware_nand',
        'hardware_cellular_external',
        'hardware_wireless',
        'linux_pci_armada_370',
        'linux_hid',
        'linux_input_mousedev',
        'linux_net_r8169',
        'linux_sata',
        ],
    linux = dict(
        MACH_8300='y',
        SATA_MV='y',
        MARVELL_PHY='y',
        REALTEK_PHY='y',
        IP_MULTICAST='y',
        IP_ROUTE_MULTIPATH='y',
        NET_IPGRE_DEMUX='y',
        NET_IPGRE='y',
        NETFILTER_NETLINK_ACCT='m',
        NETFILTER_XT_TARGET_HL='m',
        NETFILTER_XT_MATCH_NFACCT='m',
        VT='y',
        CONSOLE_TRANSLATIONS='y',
        VT_CONSOLE='y',
        SERIAL_8250_NR_UARTS='4',
        SERIAL_8250_RUNTIME_UARTS='4',
        SNAPDOG='y',
        BLK_DEV_RAM_SIZE=16384,
        ),
    user = dict(
        LIB_LIBKRB5='y',
        LIB_CYRUSSASL='y',
        LIB_LIBLDAP='y',
        LIB_LIBLZO='y',
        LIB_LIBPAM_PAM_PERMIT='y',
        USER_PAM_KRB5='y',
        LIB_LIBFTDI='y',
        USER_LIBQMI_QMI_NETWORK='y',
        PROP_FLASH_FLASH='y',
        PROP_FLASH_TAG_BASE='0x0',
        PROP_FLASH_TAG_OFFSET='0x0',
        PROP_FLASH_TAG_SIZE='0x0',
        PROP_FLASH_TAG_ERASED='0xff',
        USER_EMCTEST_EMCTEST='y',
        PROP_FTDI_LED_FTDI_LED='y',
        PROP_LOGD_LOGD='y',
        PROP_SWTEST_MII='y',
        USER_LINUX_FIRMWARE='y',
        USER_INIT_CONSOLE_SH='y',
        USER_CRON_CRON='y',
        USER_NETFLASH_CRYPTO='y',
        USER_NETFLASH_CRYPTO_V2='y',
        USER_NETFLASH_CRYPTO_OPTIONAL='y',
        USER_MTD_UTILS_ERASEALL='y',
        USER_MTD_UTILS_LOCK='y',
        USER_MTD_UTILS_UNLOCK='y',
        USER_FLATFSD_CONFIG_BLOBS='y',
        USER_FLATFSD_COMPRESSED='y',
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_E2FSCK='y',
        USER_E2FSPROGS_MKE2FS='y',
        USER_E2FSPROGS_E2LABEL='y',
        USER_E2FSPROGS_FSCK='y',
        USER_E2FSPROGS_TUNE2FS='y',
        USER_DHCP_ISC_RELAY_DHCRELAY='y',
        USER_DIALD_DIALD='y',
        USER_DISCARD_INETD_ECHO='y',
        USER_DISCARD_ECHO_NO_INSTALL='y',
        USER_IPROUTE2_TC_TC='y',
        USER_IPROUTE2_IP_ROUTEF='y',
        USER_IPROUTE2_IP_ROUTEL='y',
        USER_IPROUTE2_IP_RTACCT='y',
        USER_IPROUTE2_IP_RTMON='y',
        USER_PPTP_PPTP='y',
        USER_SIGS_SIGS='y',
        USER_PROCPS='y',
        USER_PROCPS_SYSCTL='y',
        USER_PROCPS_TLOAD='y',
        USER_PROCPS_VMSTAT='y',
        USER_PROCPS_W='y',
        USER_BUSYBOX_CPIO='y',
        USER_BUSYBOX_FEATURE_CPIO_O='y',
        USER_BUSYBOX_FEATURE_CPIO_P='y',
        USER_BUSYBOX_UNZIP='y',
        USER_BUSYBOX_DOS2UNIX='y',
        USER_BUSYBOX_UUDECODE='y',
        USER_BUSYBOX_UUENCODE='y',
        USER_BUSYBOX_LOSETUP='y',
        USER_BUSYBOX_EJECT='y',
        USER_BUSYBOX_FEATURE_EJECT_SCSI='y',
        USER_BUSYBOX_TIME='y',
        USER_BUSYBOX_ARPING='y',
        USER_BUSYBOX_FEATURE_UDHCP_PORT='y',
        USER_BUSYBOX_LSPCI='y',
        USER_BUSYBOX_SEQ='y',
        USER_ETHTOOL_ETHTOOL='y',
        BOOT_UBOOT_MARVELL_370='y',
        BOOT_UBOOT_MARVELL_370_TARGET='ac8300',
        )
    )

groups['hardware_cm7100'] = dict(
    include = [
        'hardware_armada_370',
        'hardware_nor',
        'hardware_370_mmc',
        'linux_usb_serial_common',
        'linux_pci_armada_370',
        'linux_crypto_mv_cesa',
        'linux_rtc',
        ],
    linux = dict(
        MACH_CM71xx='y',
        BLK_DEV_RAM_SIZE=32768,
        MTD_SPI_NOR='y',
        MTD_SPI_NOR_USE_4K_SECTORS='y',
        MARVELL_PHY='y',
        MICREL_PHY='y',
        REALTEK_PHY='y',
        FIXED_PHY='y',
        LEDS_GPIO='y',
        LEDS_CLASS_FLASH='y',
        LEDS_TRIGGER_TIMER='y',
        LEDS_TRIGGER_NETDEV='y',
        LEDS_TRIGGER_HEARTBEAT='y',
        LEDS_TRIGGER_DEFAULT_ON='y',
        NVMEM_SYSFS='y',
        RTC_NVMEM='y',
        RTC_DRV_MV='y',
        SERIAL_8250_EXAR='y',
        SERIAL_8250_EXTENDED='y',
        SERIAL_8250_MANY_PORTS='y',
        SNAPDOG='y',
        SQUASHFS_CRAMFS_MAGIC='y',
        JFFS2_FS='y',
        JFFS2_FS_WRITEBUFFER='y',
        JFFS2_FS_XATTR='y',
        JFFS2_FS_POSIX_ACL='y',
        JFFS2_FS_SECURITY='y',
        ),
    user = dict(
        USER_ETHTOOL_ETHTOOL='y',
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        USER_BUSYBOX_GETTY='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_BUSYBOX_LSPCI='y',
        USER_SIGS_SIGS='y',
        )
    )

groups['hardware_cm7116'] = dict(
    include = [
        'hardware_cm7100',
        ],
    linux = dict(
        SERIAL_8250_NR_UARTS='20',
        SERIAL_8250_RUNTIME_UARTS='20',
        )
    )

groups['hardware_cm7132'] = dict(
    include = [
        'hardware_cm7100',
        ],
    linux = dict(
        SERIAL_8250_NR_UARTS='36',
        SERIAL_8250_RUNTIME_UARTS='36',
        )
    )

groups['hardware_cm7148'] = dict(
    include = [
        'hardware_cm7100',
        ],
    linux = dict(
        SERIAL_8250_NR_UARTS='52',
        SERIAL_8250_RUNTIME_UARTS='52',
        )
    )

groups['hardware_9400_ua'] = dict(
    include = [
        'hardware_x86',
        'hardware_nor',
        'hardware_88e6350',
        'hardware_cellular_external',
        'linux_x86',
        'linux_net_e1000',
        'linux_net_r8169',
        'linux_i2c',
        'linux_sata',
        'linux_hid',
        'linux_uio',
        'linux_hpet',
        'linux_hugepages',
        'linux_input_mousedev',
        'linux_acpi',
        'linux_kvm',
        'linux_container',
        'linux_usb_external',
        'flashrom',
        ],
    linux = dict(
        X86_AMD_PLATFORM_DEVICE='y',
        SPI='y',
        MTD_SPI_NOR='y',
        MTD_MAP_BANK_WIDTH_1='y',
        MTD_MAP_BANK_WIDTH_2='y',
        MTD_MAP_BANK_WIDTH_4='y',
        MTD_CFI_I1='y',
        MTD_CFI_I2='y',
        MTD_AMDFCH='y',
        SPI_AMDFCH='y',
        GPIO_AMDFCH='y',
        SENSORS_K8TEMP='y',
        SENSORS_K10TEMP='y',
        SENSORS_FAM15H_POWER='y',
        SENSORS_ACPI_POWER='y',
        VT='y',
        CONSOLE_TRANSLATIONS='y',
        VT_CONSOLE='y',
        BLK_DEV_RAM_SIZE=65536,
        SERIAL_8250_EXTENDED='y',
        NVRAM='y',
        I2C_ALGOBIT='y',
        ACPI_I2C_OPREGION='y',
        NR_CPUS='4',
        SERIAL_8250_NR_UARTS='4',
        SERIAL_8250_RUNTIME_UARTS='4',
        VGA_CONSOLE='y',
        DUMMY_CONSOLE_COLUMNS='80',
        DUMMY_CONSOLE_ROWS='25',
        ),
    user = dict(
        BOOT_COREBOOT='y',
        PROP_SWTEST_MII='y',
        LIB_INSTALL_LIBATOMIC='y',
        LIB_LIBPCIACCESS='y',
        USER_LIBQMI_QMI_NETWORK='y',
        USER_LINUX_FIRMWARE='y',
        USER_EMCTEST_EMCTEST='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        USER_DISCARD_INETD_ECHO='y',
        USER_DISCARD_ECHO_NO_INSTALL='y',
        USER_SIGS_SIGS='y',
        USER_PROCPS='y',
        USER_PROCPS_TLOAD='y',
        USER_PROCPS_VMSTAT='y',
        USER_PROCPS_W='y',
        USER_PCIUTILS='y',
        USER_ETHTOOL_ETHTOOL='y',
        USER_GRUB='y',
        USER_LM_SENSORS='y',
        USER_BUSYBOX_GETTY='y',
        )
    )

groups['hardware_sprite'] = dict(
    include = [
        'hardware_armada_380',
        'hardware_nand',
        'hardware_cellular_sierra',
        'linux_usb_net_external',
        ],
    linux = dict(
        BROADCOM_PHY='y',
        BLK_DEV_RAM_SIZE=16384,
        SERIAL_8250_NR_UARTS='4',
        SERIAL_8250_RUNTIME_UARTS='4',
        SNAPDOG='y',
        CRYPTO_DES='y',
        ),
    user = dict(
        USER_ETHTOOL_ETHTOOL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_EMCTEST_EMCTEST='y',
        USER_BUSYBOX_LSPCI='y',
        USER_BUSYBOX_SEQ='y',
        USER_BUSYBOX_GETTY='y',
        BOOT_UBOOT_MARVELL_380_TARGET='ac_sprite',
        )
    )

groups['hardware_u115'] = dict(
    include = [
        'hardware_armada_380',
        'hardware_nand',
        'hardware_88e6390',
        'hardware_cellular_cm',
        'hardware_cellular_external',
        'linux_i2c',
        'linux_usb_net_external',
        'linux_usb_storage',
        ],
    linux = dict(
        BLK_DEV_RAM_SIZE=32768,
        MARVELL_PHY='y',
        GPIO_PCF857X='y',
        I2C_MV64XXX='y',
        MACH_U115='y',
        NET_DSA='y',
        NET_DSA_MV88E6XXX='y',
        SERIAL_8250_NR_UARTS='4',
        SERIAL_8250_RUNTIME_UARTS='4',
        SNAPDOG='y',
        ),
    user = dict(
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        USER_ETHTOOL_ETHTOOL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES=['y','n'],
        USER_EMCTEST_EMCTEST='y',
        USER_BUSYBOX_LSPCI='y',
        USER_BUSYBOX_SEQ='y',
        USER_BUSYBOX_GETTY='y',
        PROP_SIM_SIM=['y','n'],
        PROP_SWTEST_MII='y',
        BOOT_UBOOT_MARVELL_380_TARGET='ac_u115',
        )
    )

groups['hardware_att_u115'] = dict(
    include = [
        'hardware_armada_380',
        'hardware_nand',
        'hardware_88e6390',
        'hardware_cellular_cm',
        'hardware_cellular_external',
        'linux_i2c',
        'linux_usb_net_external',
        'linux_usb_storage',
        ],
    linux = dict(
        BLK_DEV_RAM_SIZE=32768,
        MARVELL_PHY='y',
        GPIO_PCF857X='y',
        I2C_MV64XXX='y',
        MACH_U115='y',
        NET_SWITCHDEV='y',
        MV88E6350_PHY='y',
        SERIAL_8250_NR_UARTS='4',
        SERIAL_8250_RUNTIME_UARTS='4',
        SNAPDOG='y',
        ),
    user = dict(
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        USER_ETHTOOL_ETHTOOL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO='y',
        USER_NETFLASH_CRYPTO_V2='y',
        USER_EMCTEST_EMCTEST='y',
        USER_BUSYBOX_LSPCI='y',
        USER_BUSYBOX_SEQ='y',
        USER_BUSYBOX_GETTY='y',
        USER_CRYPTO_TOOLS_CRYPTOTEST='y',
        USER_CRYPTO_TOOLS_CRYPTOKEYTEST='y',
        BOOT_UBOOT_MARVELL_380_TARGET='att_u115',
        PROP_SIM_SIM='y',
        PROP_SWTEST_DSA_SYSFS='y',
        PROP_SWTEST_DSA_SYSFS_PATH='/sys/devices/platform/soc/soc:internal-regs/f1072004.mdio/mdio_bus/f1072004.mdio-mii/f1072004.mdio-mii:10',
        )
    )

groups['hardware_factoryU115'] = dict(
    include = [
        'hardware_u115',
        'busybox_big',
        ],
    linux = dict(
        BLK_DEV_LOOP='y',
        BLK_DEV_LOOP_MIN_COUNT='8',
        CRYPTO_CCM='y',
        CRYPTO_GCM='y',
        CONSOLE_TRANSLATIONS='y',
        DEBUG_FS='y',
        NET_IPGRE_DEMUX='y',
        NET_IPGRE='y',
        RELAY='y',
        USB_SERIAL_CP210X='y',
        USB_SERIAL_PL2303='y',
        VT='y',
        VT_CONSOLE='y',
        ),
    user = dict(
        USER_NETFLASH_DUAL_IMAGES='n',
        PROP_SIM_SIM='n',
        LIB_DBUS_GLIB='y',
        LIB_DBUS='y',
        LIB_LIBKRB5='y',
        LIB_LIBLZO='y',
        LIB_LIBFTDI='y',
        LIB_NETFILTER_CONNTRACK='y',
        LIB_NETFILTER_CTHELPER='y',
        LIB_NETFILTER_CTTIMEOUT='y',
        LIB_NETFILTER_QUEUE='y',
        PROP_LOGD_LOGD='y',
        USER_BUSYBOX_CPIO='y',
        USER_BUSYBOX_FEATURE_CPIO_O='y',
        USER_BUSYBOX_FEATURE_CPIO_P='y',
        USER_BUSYBOX_UNZIP='y',
        USER_BUSYBOX_DOS2UNIX='y',
        USER_BUSYBOX_UUDECODE='y',
        USER_BUSYBOX_UUENCODE='y',
        USER_BUSYBOX_LOSETUP='y',
        USER_BUSYBOX_EJECT='y',
        USER_BUSYBOX_FEATURE_EJECT_SCSI='y',
        USER_BUSYBOX_TIME='y',
        USER_BUSYBOX_ARPING='y',
        USER_BUSYBOX_FEATURE_UDHCP_PORT='y',
        USER_CRON_CRON='y',
        USER_DIALD_DIALD='y',
        USER_DISCARD_INETD_ECHO='y',
        USER_DISCARD_ECHO_NO_INSTALL='y',
        USER_EMCTEST_EMCTEST='y',
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_E2FSCK='y',
        USER_E2FSPROGS_MKE2FS='y',
        USER_E2FSPROGS_E2LABEL='y',
        USER_E2FSPROGS_FSCK='y',
        USER_E2FSPROGS_TUNE2FS='y',
        USER_INIT_CONSOLE_SH='y',
        USER_IPROUTE2_TC_TC='y',
        USER_IPROUTE2_IP_ROUTEF='y',
        USER_IPROUTE2_IP_ROUTEL='y',
        USER_IPROUTE2_IP_RTACCT='y',
        USER_IPROUTE2_IP_RTMON='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_MTD_UTILS_ERASEALL='y',
        USER_MTD_UTILS_LOCK='y',
        USER_MTD_UTILS_UNLOCK='y',
        USER_PROCPS='y',
        USER_PROCPS_SYSCTL='y',
        USER_PROCPS_TLOAD='y',
        USER_PROCPS_VMSTAT='y',
        USER_PROCPS_W='y',
        USER_SIGS_SIGS='y',
        )
    )

groups['hardware_porter'] = dict(
    include = [
        'linux_64bit',
        'linux_pci',
        'linux_x86',
        'linux_sata',
        'hardware_cellular_sierra',
        'hardware_cellular_external',
        'hardware_virtio'
        ],
    linux = dict(
        ACPI='y',
        ACPI_BUTTON='y',
        ATA_ACPI='y',
        INPUT_EVDEV='y',
        BLK_DEV_LOOP='y',
        BLK_DEV_LOOP_MIN_COUNT=4,
        BLK_DEV_RAM_SIZE=65536,
        BLK_DEV_SD='y',
        BLK_DEV_BSG='y',
        CC_OPTIMIZE_FOR_PERFORMANCE='y',
        VT='y',
        VT_CONSOLE='y',
        SERIAL_8250='y',
        SERIAL_8250_DEPRECATED_OPTIONS='y',
        SERIAL_8250_CONSOLE='y',
        SERIAL_8250_NR_UARTS='4',
        SERIAL_8250_RUNTIME_UARTS='4',
        SERIAL_8250_EXTENDED='y',
        SERIAL_8250_PNP='y',
        EARLY_PRINTK='y',
        HZ_PERIODIC='y',
        HIGH_RES_TIMERS='y',
        MODULE_FORCE_UNLOAD='y',
        ISA_DMA_API='y',
        CROSS_MEMORY_ATTACH='y',
        STANDALONE='y',
        SCSI_PROC_FS='y',
        X86_MPPARSE='y',
        SCHED_OMIT_FRAME_POINTER='y',
        HYPERVISOR_GUEST='y',
        GENERIC_CPU='y',
        DMI='y',
        NR_CPUS='4',
        SCHED_MC='y',
        X86_VSYSCALL_EMULATION='y',
        X86_MSR='y',
        X86_CPUID='y',
        SPARSEMEM_VMEMMAP='y',
        ARCH_RANDOM='y',
        HZ_250='y',
        PHYSICAL_START='0x100000',
        PHYSICAL_ALIGN='0x1000000',
        LEGACY_VSYSCALL_NONE='y',
        COMPAT_VDSO='y',
        USB_XHCI_HCD='y',
        USB_EHCI_HCD_PLATFORM='y',
        USB_OHCI_HCD='y',
        USB_OHCI_HCD_PCI='y',
        USB_UHCI_HCD='y',
        USB_PCI='y',
        SATA_AHCI='y',
        SATA_MOBILE_LPM_POLICY='0',
        HW_RANDOM='y',
        IO_DELAY_0X80='y',
        HARDLOCKUP_DETECTOR='y',
        SSB='y',
        SSB_PCIHOST='y',
        SSB_DRIVER_PCICORE='y',
        SWCONFIG='y',
        DUMMY_CONSOLE_COLUMNS='80',
        DUMMY_CONSOLE_ROWS='25',
        PPS='y',
        PTP_1588_CLOCK='y',
        DNOTIFY='y',
        CRYPTO_MANAGER_DISABLE_TESTS='y',
        CRYPTO_NULL='y',
        CRYPTO_MD4='y',
        CRYPTO_DES='y',
        CRYPTO_HW='y',
        CRYPTO_DEV_PADLOCK='y',
        CRYPTO_DEV_PADLOCK_AES='y',
        CRYPTO_DEV_PADLOCK_SHA='y',
        CRYPTO_DEV_VIRTIO='m',
        XZ_DEC_X86='y',
        CRYPTO_SHA512='y',
        CRC16='y',
        ),
    user = dict(
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_DUAL_IMAGES_A='/mnt/disk/image',
        USER_NETFLASH_DUAL_IMAGES_B='/mnt/disk/image1',
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        USER_DISCARD_INETD_ECHO='y',
        USER_DISCARD_ECHO_NO_INSTALL='y',
        USER_EMCTEST_EMCTEST='y',
        USER_BUSYBOX_LSPCI='y',
        USER_BUSYBOX_GETTY='y',
        USER_GRUB='y',
        USER_ETHTOOL_ETHTOOL='y',
        )
    )


groups['hardware_factory8300'] = dict(
    include = [
        'hardware_armada_370',
        'hardware_bcm53118',
        'hardware_nand',
        'hardware_cellular_telit',
        'linux_pci_armada_370',
        'linux_i2c_factory',
        'linux_hid',
        'linux_input_mousedev',
        'linux_net_r8169',
        'linux_sata',
        ],
    linux = dict(
        DEBUG_FS='y',
        MACH_8300='y',
        SATA_MV='y',
        MARVELL_PHY='y',
        REALTEK_PHY='y',
        IP_MULTICAST='y',
        IP_ROUTE_MULTIPATH='y',
        NET_IPGRE_DEMUX='y',
        NET_IPGRE='y',
        BLK_DEV_LOOP='y',
        BLK_DEV_LOOP_MIN_COUNT='8',
        USB_SERIAL_CP210X='y',
        USB_SERIAL_PL2303='y',
        CRYPTO_CCM='y',
        CRYPTO_GCM='y',
        RELAY='y',
        SERIAL_8250_NR_UARTS='4',
        SERIAL_8250_RUNTIME_UARTS='4',
        SNAPDOG='y',
        VT='y',
        CONSOLE_TRANSLATIONS='y',
        VT_CONSOLE='y',
        BLK_DEV_RAM_SIZE=16384,
        ),
    user = dict(
        LIB_LIBKRB5='y',
        LIB_LIBLZO='y',
        LIB_LIBFTDI='y',
        LIB_DBUS_GLIB='y',
        LIB_DBUS='y',
        LIB_NETFILTER_CONNTRACK='y',
        LIB_NETFILTER_CTHELPER='y',
        LIB_NETFILTER_CTTIMEOUT='y',
        LIB_NETFILTER_QUEUE='y',
        PROP_FLASH_FLASH='y',
        PROP_FLASH_TAG_BASE='0x0',
        PROP_FLASH_TAG_OFFSET='0x0',
        PROP_FLASH_TAG_SIZE='0x0',
        PROP_FLASH_TAG_ERASED='0xff',
        PROP_FTDI_LED_FTDI_LED='y',
        PROP_LOGD_LOGD='y',
        PROP_SWTEST_MII='y',
        USER_LINUX_FIRMWARE='y',
        USER_INIT_CONSOLE_SH='y',
        USER_CRON_CRON='y',
        USER_NETFLASH_CRYPTO='y',
        USER_NETFLASH_CRYPTO_OPTIONAL='y',
        USER_MTD_UTILS_ERASEALL='y',
        USER_MTD_UTILS_LOCK='y',
        USER_MTD_UTILS_UNLOCK='y',
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_E2FSCK='y',
        USER_E2FSPROGS_MKE2FS='y',
        USER_E2FSPROGS_E2LABEL='y',
        USER_E2FSPROGS_FSCK='y',
        USER_E2FSPROGS_TUNE2FS='y',
        USER_DIALD_DIALD='y',
        USER_DISCARD_INETD_ECHO='y',
        USER_DISCARD_ECHO_NO_INSTALL='y',
        USER_IPROUTE2_TC_TC='y',
        USER_IPROUTE2_IP_ROUTEF='y',
        USER_IPROUTE2_IP_ROUTEL='y',
        USER_IPROUTE2_IP_RTACCT='y',
        USER_IPROUTE2_IP_RTMON='y',
        USER_SIGS_SIGS='y',
        USER_PROCPS='y',
        USER_PROCPS_SYSCTL='y',
        USER_PROCPS_TLOAD='y',
        USER_PROCPS_VMSTAT='y',
        USER_PROCPS_W='y',
        USER_BUSYBOX_CPIO='y',
        USER_BUSYBOX_FEATURE_CPIO_O='y',
        USER_BUSYBOX_FEATURE_CPIO_P='y',
        USER_BUSYBOX_UNZIP='y',
        USER_BUSYBOX_DOS2UNIX='y',
        USER_BUSYBOX_UUDECODE='y',
        USER_BUSYBOX_UUENCODE='y',
        USER_BUSYBOX_LOSETUP='y',
        USER_BUSYBOX_EJECT='y',
        USER_BUSYBOX_FEATURE_EJECT_SCSI='y',
        USER_BUSYBOX_TIME='y',
        USER_BUSYBOX_ARPING='y',
        USER_BUSYBOX_FEATURE_UDHCP_PORT='y',
        USER_BUSYBOX_LSPCI='y',
        USER_ETHTOOL_ETHTOOL='y',
        USER_EMCTEST_EMCTEST='y',
        BOOT_UBOOT_MARVELL_370='y',
        BOOT_UBOOT_MARVELL_370_TARGET='ac8300',
        )
    )

groups['busybox'] = dict(
    user = dict(
        USER_BUSYBOX='y',
        USER_BUSYBOX_PIE='y',
        USER_BUSYBOX_ARPING='y',
        USER_BUSYBOX_ETHER_WAKE='y',
        USER_BUSYBOX_INCLUDE_SUSv2='y',
        USER_BUSYBOX_FEATURE_BUFFERS_USE_MALLOC='y',
        USER_BUSYBOX_FEATURE_TEST_64='y',
        USER_BUSYBOX_SHOW_USAGE='y',
        USER_BUSYBOX_FEATURE_VERBOSE_USAGE='y',
        USER_BUSYBOX_FEATURE_COMPRESS_USAGE='y',
        USER_BUSYBOX_LFS='y',
        USER_BUSYBOX_LONG_OPTS='y',
        USER_BUSYBOX_FEATURE_DEVPTS='y',
        USER_BUSYBOX_NO_DEBUG_LIB='y',
        USER_BUSYBOX_INSTALL_APPLET_SYMLINKS='y',
        USER_BUSYBOX_FEATURE_EDITING='y',
        USER_BUSYBOX_FEATURE_TAB_COMPLETION='y',
        USER_BUSYBOX_FEATURE_NON_POSIX_CP='y',
        USER_BUSYBOX_FEATURE_SKIP_ROOTFS='y',
        USER_BUSYBOX_MONOTONIC_SYSCALL='y',
        USER_BUSYBOX_IOCTL_HEX2STR_ERROR='y',
        USER_BUSYBOX_FEATURE_SEAMLESS_XZ='y',
        USER_BUSYBOX_FEATURE_SEAMLESS_LZMA='y',
        USER_BUSYBOX_FEATURE_SEAMLESS_BZ2='y',
        USER_BUSYBOX_FEATURE_SEAMLESS_GZ='y',
        USER_BUSYBOX_GUNZIP='y',
        USER_BUSYBOX_GZIP='y',
        USER_BUSYBOX_FEATURE_GZIP_LONG_OPTIONS='y',
        USER_BUSYBOX_TAR='y',
        USER_BUSYBOX_FEATURE_TAR_CREATE='y',
        USER_BUSYBOX_FEATURE_TAR_FROM='y',
        USER_BUSYBOX_FEATURE_TAR_NOPRESERVE_TIME='y',
        USER_BUSYBOX_FEATURE_TAR_GNU_EXTENSIONS='y',
        USER_BUSYBOX_BASENAME='y',
        USER_BUSYBOX_CAT='y',
        USER_BUSYBOX_DATE='y',
        USER_BUSYBOX_FEATURE_DATE_ISOFMT='y',
        USER_BUSYBOX_FEATURE_DATE_NANO='y',
        USER_BUSYBOX_FEATURE_DATE_COMPAT='y',
        USER_BUSYBOX_ID='y',
        USER_BUSYBOX_GROUPS='y',
        USER_BUSYBOX_TEST='y',
        USER_BUSYBOX_TOUCH='y',
        USER_BUSYBOX_FEATURE_TOUCH_SUSV3='y',
        USER_BUSYBOX_TR='y',
        USER_BUSYBOX_FEATURE_TR_CLASSES='y',
        USER_BUSYBOX_FEATURE_TR_EQUIV='y',
        USER_BUSYBOX_CHGRP='y',
        USER_BUSYBOX_CHMOD='y',
        USER_BUSYBOX_CHOWN='y',
        USER_BUSYBOX_FEATURE_CHOWN_LONG_OPTIONS='y',
        USER_BUSYBOX_CHROOT='y',
        USER_BUSYBOX_CP='y',
        USER_BUSYBOX_FEATURE_CP_LONG_OPTIONS='y',
        USER_BUSYBOX_CUT='y',
        USER_BUSYBOX_DD='y',
        USER_BUSYBOX_DF='y',
        USER_BUSYBOX_FEATURE_DF_FANCY='y',
        USER_BUSYBOX_DIRNAME='y',
        USER_BUSYBOX_DU='y',
        USER_BUSYBOX_FEATURE_DU_DEFAULT_BLOCKSIZE_1K='y',
        USER_BUSYBOX_ECHO='y',
        USER_BUSYBOX_FEATURE_FANCY_ECHO='y',
        USER_BUSYBOX_ENV='y',
        USER_BUSYBOX_EXPR='y',
        USER_BUSYBOX_EXPR_MATH_SUPPORT_64='y',
        USER_BUSYBOX_FALSE='y',
        USER_BUSYBOX_FSYNC='y',
        USER_BUSYBOX_HEAD='y',
        USER_BUSYBOX_LN='y',
        USER_BUSYBOX_LS='y',
        USER_BUSYBOX_FEATURE_LS_FILETYPES='y',
        USER_BUSYBOX_FEATURE_LS_FOLLOWLINKS='y',
        USER_BUSYBOX_FEATURE_LS_RECURSIVE='y',
        USER_BUSYBOX_FEATURE_LS_SORTFILES='y',
        USER_BUSYBOX_FEATURE_LS_TIMESTAMPS='y',
        USER_BUSYBOX_FEATURE_LS_USERNAME='y',
        USER_BUSYBOX_MD5SUM='y',
        USER_BUSYBOX_MKDIR='y',
        USER_BUSYBOX_MKFIFO='y',
        USER_BUSYBOX_MKNOD='y',
        USER_BUSYBOX_MV='y',
        USER_BUSYBOX_NICE='y',
        USER_BUSYBOX_PRINTF='y',
        USER_BUSYBOX_PWD='y',
        USER_BUSYBOX_READLINK='y',
        USER_BUSYBOX_FEATURE_READLINK_FOLLOW='y',
        USER_BUSYBOX_RM='y',
        USER_BUSYBOX_RMDIR='y',
        USER_BUSYBOX_SLEEP='y',
        USER_BUSYBOX_SORT='y',
        USER_BUSYBOX_STAT='y',
        USER_BUSYBOX_FEATURE_STAT_FORMAT='y',
        USER_BUSYBOX_STTY='y',
        USER_BUSYBOX_SYNC='y',
        USER_BUSYBOX_TAIL='y',
        USER_BUSYBOX_FEATURE_FANCY_TAIL='y',
        USER_BUSYBOX_TEE='y',
        USER_BUSYBOX_TRUE='y',
        USER_BUSYBOX_TTY='y',
        USER_BUSYBOX_UNAME='y',
        USER_BUSYBOX_UNAME_OSNAME='Digi Accelerated Linux',
        USER_BUSYBOX_UNIQ='y',
        USER_BUSYBOX_USLEEP='y',
        USER_BUSYBOX_WC='y',
        USER_BUSYBOX_YES='y',
        USER_BUSYBOX_FEATURE_HUMAN_READABLE='y',
        USER_BUSYBOX_CLEAR='y',
        USER_BUSYBOX_RESET='y',
        USER_BUSYBOX_MKTEMP='y',
        USER_BUSYBOX_WHICH='y',
        USER_BUSYBOX_VI='y',
        USER_BUSYBOX_FEATURE_VI_8BIT='y',
        USER_BUSYBOX_FEATURE_VI_COLON='y',
        USER_BUSYBOX_FEATURE_VI_YANKMARK='y',
        USER_BUSYBOX_FEATURE_VI_SEARCH='y',
        USER_BUSYBOX_FEATURE_VI_USE_SIGNALS='y',
        USER_BUSYBOX_FEATURE_VI_DOT_CMD='y',
        USER_BUSYBOX_FEATURE_VI_READONLY='y',
        USER_BUSYBOX_FEATURE_VI_SETOPTS='y',
        USER_BUSYBOX_FEATURE_VI_SET='y',
        USER_BUSYBOX_FEATURE_VI_WIN_RESIZE='y',
        USER_BUSYBOX_FEATURE_VI_ASK_TERMINAL='y',
        USER_BUSYBOX_CMP='y',
        USER_BUSYBOX_SED='y',
        USER_BUSYBOX_FEATURE_ALLOW_EXEC='y',
        USER_BUSYBOX_FIND='y',
        USER_BUSYBOX_FEATURE_FIND_MTIME='y',
        USER_BUSYBOX_FEATURE_FIND_PERM='y',
        USER_BUSYBOX_FEATURE_FIND_TYPE='y',
        USER_BUSYBOX_FEATURE_FIND_NEWER='y',
        USER_BUSYBOX_FEATURE_FIND_SIZE='y',
        USER_BUSYBOX_FEATURE_FIND_LINKS='y',
        USER_BUSYBOX_GREP='y',
        USER_BUSYBOX_EGREP='y',
        USER_BUSYBOX_FGREP='y',
        USER_BUSYBOX_FEATURE_GREP_CONTEXT='y',
        USER_BUSYBOX_XARGS='y',
        USER_BUSYBOX_FEATURE_XARGS_SUPPORT_QUOTES='y',
        USER_BUSYBOX_HALT='y',
        USER_BUSYBOX_REBOOT='y',
        USER_BUSYBOX_POWEROFF='y',
        USER_BUSYBOX_USE_BB_CRYPT='y',
        USER_BUSYBOX_USE_BB_CRYPT_SHA='y',
        USER_BUSYBOX_DMESG='y',
        USER_BUSYBOX_FREERAMDISK='y',
        USER_BUSYBOX_MORE='y',
        USER_BUSYBOX_MOUNT='y',
        USER_BUSYBOX_FEATURE_MOUNT_NFS='y',
        USER_BUSYBOX_FEATURE_MOUNT_FLAGS='y',
        USER_BUSYBOX_RDATE='y',
        USER_BUSYBOX_UMOUNT='y',
        USER_BUSYBOX_FEATURE_UMOUNT_ALL='y',
        USER_BUSYBOX_FEATURE_MOUNT_LOOP='y',
        USER_BUSYBOX_FEATURE_MOUNT_LOOP_CREATE='y',
        USER_BUSYBOX_TIMEOUT='y',
        USER_BUSYBOX_TIME='y',
        USER_BUSYBOX_VOLNAME='y',
        USER_BUSYBOX_WATCHDOG='y',
        USER_BUSYBOX_WHOIS='y',
        USER_BUSYBOX_NSLOOKUP='y',
        USER_BUSYBOX_TELNET=['y', 'n'],
        USER_BUSYBOX_FEATURE_TELNET_TTYPE=['y', 'n'],
        USER_BUSYBOX_TFTP='y',
        USER_BUSYBOX_FEATURE_TFTP_GET='y',
        USER_BUSYBOX_FEATURE_TFTP_PUT='y',
        USER_BUSYBOX_UDHCPC='y',
        USER_BUSYBOX_FEATURE_UDHCPC_ARPING='y',
        USER_BUSYBOX_FEATURE_UDHCP_RFC3397='y',
        USER_BUSYBOX_FEATURE_UDHCP_8021Q='y',
        USER_BUSYBOX_WGET='y',
        USER_BUSYBOX_FEATURE_WGET_STATUSBAR='y',
        USER_BUSYBOX_FEATURE_WGET_AUTHENTICATION='y',
        USER_BUSYBOX_FEATURE_WGET_TIMEOUT='y',
        USER_BUSYBOX_IOSTAT='y',
        USER_BUSYBOX_LSOF='y',
        USER_BUSYBOX_TOP='y',
        USER_BUSYBOX_FEATURE_TOP_CPU_USAGE_PERCENTAGE='y',
        USER_BUSYBOX_FEATURE_TOP_CPU_GLOBAL_PERCENTS='y',
        USER_BUSYBOX_FEATURE_TOP_SMP_CPU='y',
        USER_BUSYBOX_FEATURE_TOP_SMP_PROCESS='y',
        USER_BUSYBOX_UPTIME='y',
        USER_BUSYBOX_FREE='y',
        USER_BUSYBOX_FUSER='y',
        USER_BUSYBOX_KILL='y',
        USER_BUSYBOX_KILLALL='y',
        USER_BUSYBOX_PIDOF='y',
        USER_BUSYBOX_PS='y',
        USER_BUSYBOX_FEATURE_PS_LONG='y',
        USER_BUSYBOX_RENICE='y',
        USER_BUSYBOX_LOGGER='y',
        USER_BUSYBOX_WHOAMI='y',
        USER_BUSYBOX_SETSID='y',
        )
    )

groups['busybox_big'] = dict(
    user = dict(
        USER_BUSYBOX_LONG_OPTS='y',
        USER_BUSYBOX_FEATURE_DATE_NANO='y',
        USER_BUSYBOX_HOSTID='y',
        USER_BUSYBOX_FEATURE_TEST_64='y',
        USER_BUSYBOX_BASE64='y',
        USER_BUSYBOX_CAL='y',
        USER_BUSYBOX_CKSUM='y',
        USER_BUSYBOX_COMM='y',
        USER_BUSYBOX_FEATURE_DD_IBS_OBS='y',
        USER_BUSYBOX_EXPAND='y',
        USER_BUSYBOX_EXPR_MATH_SUPPORT_64='y',
        USER_BUSYBOX_FOLD='y',
        USER_BUSYBOX_FEATURE_FANCY_HEAD='y',
        USER_BUSYBOX_LOGNAME='y',
        USER_BUSYBOX_FEATURE_LS_COLOR='y',
        USER_BUSYBOX_FEATURE_LS_COLOR_IS_DEFAULT='y',
        USER_BUSYBOX_NOHUP='y',
        USER_BUSYBOX_OD='y',
        USER_BUSYBOX_PRINTENV='y',
        USER_BUSYBOX_FEATURE_READLINK_FOLLOW='y',
        USER_BUSYBOX_REALPATH='y',
        USER_BUSYBOX_SHA1SUM='y',
        USER_BUSYBOX_SHA256SUM='y',
        USER_BUSYBOX_SHA512SUM='y',
        USER_BUSYBOX_SHA3SUM='y',
        USER_BUSYBOX_FEATURE_FANCY_SLEEP='y',
        USER_BUSYBOX_FLOAT_DURATION='y',
        USER_BUSYBOX_FEATURE_SORT_BIG='y',
        USER_BUSYBOX_SPLIT='y',
        USER_BUSYBOX_FEATURE_SPLIT_FANCY='y',
        USER_BUSYBOX_SUM='y',
        USER_BUSYBOX_TAC='y',
        USER_BUSYBOX_FEATURE_TEE_USE_BLOCK_IO='y',
        USER_BUSYBOX_UNEXPAND='y',
        USER_BUSYBOX_FEATURE_WC_LARGE='y',
        USER_BUSYBOX_WHOAMI='y',
        USER_BUSYBOX_FEATURE_PRESERVE_HARDLINKS='y',
        USER_BUSYBOX_FEATURE_MD5_SHA1_SUM_CHECK='y',
        USER_BUSYBOX_DIFF='y',
        USER_BUSYBOX_FEATURE_DIFF_LONG_OPTIONS='y',
        USER_BUSYBOX_FEATURE_DIFF_DIR='y',
        USER_BUSYBOX_ED='y',
        USER_BUSYBOX_FEATURE_FIND_PRINT0='y',
        USER_BUSYBOX_FEATURE_FIND_MMIN='y',
        USER_BUSYBOX_FEATURE_FIND_XDEV='y',
        USER_BUSYBOX_FEATURE_FIND_MAXDEPTH='y',
        USER_BUSYBOX_FEATURE_FIND_INUM='y',
        USER_BUSYBOX_FEATURE_FIND_EXEC='y',
        USER_BUSYBOX_FEATURE_FIND_USER='y',
        USER_BUSYBOX_FEATURE_FIND_GROUP='y',
        USER_BUSYBOX_FEATURE_FIND_NOT='y',
        USER_BUSYBOX_FEATURE_FIND_DEPTH='y',
        USER_BUSYBOX_FEATURE_FIND_PAREN='y',
        USER_BUSYBOX_FEATURE_FIND_PRUNE='y',
        USER_BUSYBOX_FEATURE_FIND_DELETE='y',
        USER_BUSYBOX_FEATURE_FIND_PATH='y',
        USER_BUSYBOX_FEATURE_FIND_REGEX='y',
        USER_BUSYBOX_FEATURE_XARGS_SUPPORT_CONFIRMATION='y',
        USER_BUSYBOX_FEATURE_XARGS_SUPPORT_TERMOPT='y',
        USER_BUSYBOX_FEATURE_XARGS_SUPPORT_ZERO_TERM='y',
        USER_BUSYBOX_BLKID='y',
        USER_BUSYBOX_FEATURE_BLKID_TYPE='y',
        USER_BUSYBOX_FINDFS='y',
        USER_BUSYBOX_GETOPT='y',
        USER_BUSYBOX_FEATURE_GETOPT_LONG='y',
        USER_BUSYBOX_LOSETUP=['y','n'],
        USER_BUSYBOX_MKSWAP='y',
        USER_BUSYBOX_FEATURE_MKSWAP_UUID='y',
        USER_BUSYBOX_FEATURE_MOUNT_FAKE='y',
        USER_BUSYBOX_FEATURE_MOUNT_VERBOSE='y',
        USER_BUSYBOX_FEATURE_MOUNT_HELPERS='y',
        USER_BUSYBOX_FEATURE_MOUNT_LABEL='y',
        USER_BUSYBOX_FEATURE_MOUNT_CIFS='y',
        USER_BUSYBOX_PIVOT_ROOT='y',
        USER_BUSYBOX_RDEV='y',
        USER_BUSYBOX_READPROFILE='y',
        USER_BUSYBOX_SETARCH='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_EXT='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_BTRFS='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_REISERFS='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_FAT='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_EXFAT='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_HFS='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_JFS='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_XFS='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_NILFS='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_NTFS='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_ISO9660='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_UDF='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_LUKS='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_LINUXSWAP='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_CRAMFS='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_ROMFS='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_SQUASHFS='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_SYSV='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_OCFS2='y',
        USER_BUSYBOX_FEATURE_VOLUMEID_LINUXRAID='y',
        USER_BUSYBOX_LESS='y',
        USER_BUSYBOX_FEATURE_LESS_BRACKETS='y',
        USER_BUSYBOX_FEATURE_LESS_FLAGS='y',
        USER_BUSYBOX_FEATURE_LESS_MARKS='y',
        USER_BUSYBOX_FEATURE_LESS_REGEXP='y',
        USER_BUSYBOX_FEATURE_LESS_WINCH='y',
        USER_BUSYBOX_FEATURE_LESS_ASK_TERMINAL='y',
        USER_BUSYBOX_FEATURE_LESS_DASHCMD='y',
        USER_BUSYBOX_FEATURE_LESS_LINENUMS='y',
        USER_BUSYBOX_SETSERIAL='y',
        USER_BUSYBOX_ADJTIMEX='y',
        USER_BUSYBOX_CHRT='y',
        USER_BUSYBOX_DC='y',
        USER_BUSYBOX_UUDECODE='y',
        USER_BUSYBOX_UUENCODE='y',
        USER_BUSYBOX_EJECT='y',
        USER_BUSYBOX_FEATURE_EJECT_SCSI='y',
        USER_BUSYBOX_FEATURE_DC_LIBM='y',
        USER_BUSYBOX_IONICE='y',
        USER_BUSYBOX_INOTIFYD='y',
        USER_BUSYBOX_HDPARM='y',
        USER_BUSYBOX_FEATURE_HDPARM_GET_IDENTITY='y',
        USER_BUSYBOX_FEATURE_HDPARM_HDIO_SCAN_HWIF='y',
        USER_BUSYBOX_FEATURE_HDPARM_HDIO_UNREGISTER_HWIF='y',
        USER_BUSYBOX_FEATURE_HDPARM_HDIO_DRIVE_RESET='y',
        USER_BUSYBOX_FEATURE_HDPARM_HDIO_TRISTATE_HWIF='y',
        USER_BUSYBOX_FEATURE_HDPARM_HDIO_GETSET_DMA='y',
        USER_BUSYBOX_MOUNTPOINT='y',
        USER_BUSYBOX_RX='y',
        USER_BUSYBOX_SETSID='y',
        USER_BUSYBOX_STRINGS='y',
        USER_BUSYBOX_TASKSET='y',
        USER_BUSYBOX_FEATURE_TASKSET_FANCY='y',
        USER_BUSYBOX_TIME='y',
        USER_BUSYBOX_TTYSIZE='y',
        USER_BUSYBOX_NAMEIF='y',
        USER_BUSYBOX_FEATURE_NAMEIF_EXTENDED='y',
        USER_BUSYBOX_HOSTNAME='y',
        USER_BUSYBOX_FEATURE_TFTP_BLOCKSIZE='y',
        USER_BUSYBOX_FEATURE_TFTP_PROGRESS_BAR='y',
        USER_BUSYBOX_MPSTAT='y',
        USER_BUSYBOX_NMETER='y',
        USER_BUSYBOX_PMAP='y',
        USER_BUSYBOX_POWERTOP='y',
        USER_BUSYBOX_PSTREE='y',
        USER_BUSYBOX_PWDX='y',
        USER_BUSYBOX_SMEMCAP='y',
        USER_BUSYBOX_FEATURE_TOPMEM='y',
        USER_BUSYBOX_KILLALL5='y',
        USER_BUSYBOX_PGREP='y',
        USER_BUSYBOX_FEATURE_PIDOF_SINGLE='y',
        USER_BUSYBOX_FEATURE_PIDOF_OMIT='y',
        USER_BUSYBOX_PKILL='y',
        USER_BUSYBOX_FEATURE_PS_WIDE='y',
        USER_BUSYBOX_BB_SYSCTL='y',
        USER_BUSYBOX_FEATURE_SHOW_THREADS='y',
        USER_BUSYBOX_WATCH='y',
        USER_BUSYBOX_FEATURE_SH_MATH='y',
        USER_BUSYBOX_FEATURE_SH_MATH_64='y',
        USER_BUSYBOX_UNZIP='y',
        USER_BUSYBOX_FEATURE_UNZIP_CDF='y'
        )
    )

groups['busybox_ash'] = dict(
    user = dict(
        USER_BUSYBOX_ASH='y',
        USER_BUSYBOX_ASH_BASH_COMPAT='y',
        USER_BUSYBOX_ASH_JOB_CONTROL='y',
        USER_BUSYBOX_ASH_ALIAS='y',
        USER_BUSYBOX_ASH_GETOPTS='y',
        USER_BUSYBOX_ASH_ECHO='y',
        USER_BUSYBOX_ASH_PRINTF='y',
        USER_BUSYBOX_ASH_TEST='y',
        USER_BUSYBOX_ASH_OPTIMIZE_FOR_SIZE='y',
        USER_BUSYBOX_ASH_INTERNAL_GLOB='y',
        USER_BUSYBOX_ASH_RANDOM_SUPPORT='y',
        USER_BUSYBOX_SHELL_ASH='y',
        USER_BUSYBOX_BASH_IS_ASH='y',
        USER_BUSYBOX_FEATURE_SH_MATH='y',
        USER_BUSYBOX_FEATURE_SH_MATH_64='y',
        USER_BUSYBOX_FEATURE_SH_EXTRA_QUIET='y',
        USER_OTHER_SH='y'
        )
    )

groups['busybox_ipv6'] = dict(
    user = dict(
        USER_BUSYBOX_FEATURE_IPV6='y',
        USER_BUSYBOX_FEATURE_PREFER_IPV4_ADDRESS='y',
        )
    )

groups['varlog_mount'] = dict(
    linux = dict(
        BLK_DEV_LOOP='y',
        BLK_DEV_LOOP_MIN_COUNT='8',
        EXT4_FS='y',
        EXT4_USE_FOR_EXT2='y',
        ),
    user = dict(
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_MKFS_EXT2='y',
        USER_UTIL_LINUX_LOSETUP='y',
        USER_BUSYBOX_CHATTR='y',
        USER_BUSYBOX_LSATTR='y',
        )
    )

groups['e2fsprogs_ext2'] = dict(
    user = dict(
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_E2FSCK='y',
        USER_E2FSPROGS_FSCK='y',
        USER_E2FSPROGS_FSCK_EXT2='y',
        USER_E2FSPROGS_MKFS_EXT2='y',
        )
    )

groups['e2fsprogs_all'] = dict(
    user = dict(
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_BADBLOCKS='y',
        USER_E2FSPROGS_BLKID='y',
        USER_E2FSPROGS_CHATTR='y',
        USER_E2FSPROGS_DEBUGFS='y',
        USER_E2FSPROGS_DUMPE2FS='y',
        USER_E2FSPROGS_E2FREEFRAG='y',
        USER_E2FSPROGS_E2FSCK='y',
        USER_E2FSPROGS_E2LABEL='y',
        USER_E2FSPROGS_E2IMAGE='y',
        USER_E2FSPROGS_E2UNDO='y',
        USER_E2FSPROGS_FILEFRAG='y',
        USER_E2FSPROGS_FINDFS='y',
        USER_E2FSPROGS_FSCK='y',
        USER_E2FSPROGS_FSCK_EXT2='y',
        USER_E2FSPROGS_FSCK_EXT3='y',
        USER_E2FSPROGS_FSCK_EXT4='y',
        USER_E2FSPROGS_LOGSAVE='y',
        USER_E2FSPROGS_LSATTR='y',
        USER_E2FSPROGS_MKE2FS='y',
        USER_E2FSPROGS_MKFS_EXT2='y',
        USER_E2FSPROGS_MKFS_EXT3='y',
        USER_E2FSPROGS_MKFS_EXT4='y',
        USER_E2FSPROGS_MKLOST_FOUND='y',
        USER_E2FSPROGS_RESIZE2FS='y',
        USER_E2FSPROGS_TUNE2FS='y',
        )
    )

groups['qemu_x86'] = dict(
    user = dict(
        USER_QEMU='y',
        LIB_LIBPIXMAN='y',
        USER_QEMU_I386_SOFTMMU='y',
        USER_QEMU_X86_64_SOFTMMU='y'
        )
    )

groups['frrouting'] = dict(
    user = dict(
        USER_FRROUTING_ZEBRA='y',
        USER_FRROUTING_BGPD='y',
        USER_FRROUTING_RIPD='y',
        USER_FRROUTING_RIPNGD='y',
        USER_FRROUTING_OSPFD='y',
        USER_FRROUTING_OSPF6D='y',
        USER_FRROUTING_ISISD='y',
        USER_FRROUTING_NHRPD='y',
        ),
    linux = dict(
        TCP_MD5SIG='y', # Required by BGP
        )
    )

# base pppd included on legacy devices
groups['pppd'] = dict(
    user = dict(
        USER_PPPD_PPPD_PPPD='y',
        USER_PPPD_WITH_PAM='y',
        USER_PPPD_WITH_TACACS='y',
        USER_PPPD_WITH_RADIUS='y',
        USER_PPPD_WITH_PPPOA='y',
        USER_PPPD_WITH_PPTP='y',
        USER_PPPD_NO_AT_REDIRECTION='y'
        )
    )

groups['pppd_pppoe'] = dict(
    user = dict(
        USER_PPPD_WITH_DYNAMIC_PLUGINS='y',
        USER_PPPD_WITH_MPPE='y',
        USER_PPPD_WITH_PPPOE='y'
        )
    )

groups['user_common'] = dict(
    include = [
        'busybox',
        'busybox_ash',
        'busybox_httpd',
        'busybox_ipv6',
        'init_overlay',
        'pppd',
        'ipv6',
        'tcpdump',
        'config_dnsmasq2',
        'config_pstore',
        ],
    user = dict(
        LIB_FLEX='y',
        LIB_OPENSSL='y',
        LIB_LIBGMP='y',
        LIB_LIBNET='y',
        LIB_LIBPAM='y',
        LIB_LIBPAM_PAM_DENY='y',
        LIB_LIBPAM_PAM_ENV='y',
        LIB_LIBPAM_PAM_MOTD='y',
        LIB_LIBPAM_PAM_UNIX='y',
        LIB_LIBPAM_PAM_WARN='y',
        USER_PAM_TACACS='y',
        USER_PAM_RADIUS='y',
        USER_PAM_GOOGLE_AUTHENTICATOR='y',
        LIB_ZLIB='y',
        LIB_LIBBZ2='y',
        LIB_TERMCAP='y',
        LIB_TERMCAP_SHARED='y',
        LIB_EXPAT='y',
        LIB_LIBIDN2='y',
        LIB_OSIP2='y',
        LIB_GETTEXT='y',
        LIB_GLIB='y',
        LIB_JSON_C='y',
        LIB_LIBCURL='y',
        LIB_LIBCURL_CURL='y',
        LIB_LIBFFI='y',
        LIB_LIBMNL='y',
        LIB_NETFILTER_CONNTRACK='y',
        LIB_NETFILTER_CTHELPER='y',
        LIB_NETFILTER_CTTIMEOUT='y',
        LIB_NETFILTER_QUEUE='y',
        LIB_NFNETLINK='y',
        LIB_LIBNL='y',
        LIB_LIBUBOX='y',
        LIB_LIBPCRE='y',
        LIB_LIBTIRPC='y',
        PROP_CONFIG_WEBUI='y',
        PROP_CONFIG_RESTAPI='y',
        PROP_CONFIG_ACTIOND='y',
        PROP_CONFIG_FINDME=['y', 'n'],
        PROP_SCHEDULED_SCRIPT='y',
        USER_BUSYBOX_FEATURE_SORT_BIG='y',
        USER_INIT_INIT='y',
        USER_INIT_RUN_CONFIGRC=['n', 'y'],
        USER_GETTYD_GETTYD='y',
        USER_FCRON='y',
        USER_FLASHW_FLASHW='y',
        USER_SETMAC_SETMAC='y',
        USER_UBOOT_ENVTOOLS=['y','n'],
        USER_NETFLASH_NETFLASH=['y','n'],
        USER_NETFLASH_WITH_FTP=['y','n'],
        USER_NETFLASH_WITH_CGI=['y','n'],
        USER_NETFLASH_HARDWARE=['y','n'],
        USER_NETFLASH_DECOMPRESS=['y','n'],
        USER_FLATFSD_FLATFSD='y',
        USER_FLATFSD_EXTERNAL_INIT='y',
        USER_BRCTL_BRCTL='y',
        USER_DISCARD_DISCARD='y',
        USER_DISCARD_NO_INSTALL='y',
        USER_EBTABLES_EBTABLES='y',
        USER_FTP_FTP_FTP='y',
        USER_INETD_INETD='y',
        USER_IPROUTE2='y',
        USER_IPROUTE2_BRIDGE='y',
        USER_IPROUTE2_IP_IP='y',
        USER_IPUTILS_IPUTILS='y',
        USER_IPUTILS_PING='y',
        USER_IPUTILS_PING6='y',
        USER_IPUTILS_TRACEROUTE6='y',
        USER_IPUTILS_TRACEPATH='y',
        USER_IPUTILS_TRACEPATH6='y',
        USER_SMTP_SMTPCLIENT='y',
        USER_NETCAT_NETCAT='y',
        USER_NTPD_NTPD='y',
        USER_NTPD_NTPDATE='y',
        USER_NTPD_NTPQ='y',
        USER_OPENSSL_APPS='y',
        USER_PPTPD_PPTPCTRL='y',
        USER_PPTPD_PPTPD='y',
        USER_RSYNC_RSYNC='y',
        USER_STUNNEL_STUNNEL='y',
        USER_SSH_SSH='y',
        USER_SSH_SSHD='y',
        USER_SSH_SSHD_CUSTOM_CONFIG=['y','n'],
        USER_SSH_SSHKEYGEN='y',
        USER_SSH_SCP='y',
        USER_SSH_SFTP='y',
        USER_SSH_SFTP_SERVER='y',
        USER_TCPBLAST_TCPBLAST='y',
        USER_TELNETD_TELNETD=['y', 'n'],
        USER_TRACEROUTE_TRACEROUTE='y',
        USER_NET_TOOLS_ARP='y',
        USER_NET_TOOLS_HOSTNAME='y',
        USER_NET_TOOLS_IFCONFIG='y',
        USER_NET_TOOLS_NETSTAT='y',
        USER_NET_TOOLS_ROUTE='y',
        USER_NET_TOOLS_MII_TOOL='y',
        USER_CHAT_CHAT='y',
        USER_CPU_CPU='y',
        USER_HASERL_HASERL='y',
        USER_HD_HD='y',
        USER_LEDCMD_LEDCMD=['y', 'n'],
        USER_MAWK_AWK='y',
        USER_SHADOW_UTILS='y',
        USER_SHADOW_PAM='y',
        USER_SHADOW_LOGIN='y',
        USER_STRACE='y',
        USER_TIP_TIP='y',
        USER_RAMIMAGE_NONE='y',
        POOR_ENTROPY=['y', 'n'],
        USER_KMOD='y',
        USER_KMOD_LIBKMOD='y',
        USER_KMOD_TOOLS='y',
        USER_NETIFD='y',
        USER_NUTTCP='y',
        USER_READLINE='y',
        USER_SYSKLOGD='y',
        USER_SYSKLOGD_LOG_PRIORITY='y',
        USER_UBUS='y',
        USER_UCI='y',
        USER_UDEV='y',
        USER_UTIL_LINUX='y',
        USER_UTIL_LINUX_LIBBLKID='y',
        USER_UTIL_LINUX_LIBUUID='y',
        LIB_LIBLDAP='y',
        USER_PAM_LDAP='y',
        )
    )

groups['user_minimal'] = dict(
    include = [
        'busybox_ash',
        ],
    user = dict(
        USER_SYSKLOGD='y',
        USER_SYSKLOGD_LOG_PRIORITY='y',
        USER_UTIL_LINUX='y',
        USER_UTIL_LINUX_LIBUUID='y',
        USER_READLINE='y',
        USER_BUSYBOX='y',
        USER_BUSYBOX_LFS='y',
        USER_BUSYBOX_FEATURE_SORT_BIG='y',
        USER_BUSYBOX_FLOCK='n',
        USER_BUSYBOX_HEXDUMP='y',
        USER_BUSYBOX_PIVOT_ROOT='y',
        USER_BUSYBOX_LESS='y',
        USER_BUSYBOX_FEATURE_LESS_BRACKETS='y',
        USER_BUSYBOX_FEATURE_LESS_FLAGS='y',
        USER_BUSYBOX_FEATURE_LESS_MARKS='y',
        USER_BUSYBOX_FEATURE_LESS_REGEXP='y',
        USER_BUSYBOX_FEATURE_LESS_WINCH='y',
        USER_BUSYBOX_FEATURE_LESS_ASK_TERMINAL='y',
        USER_BUSYBOX_FEATURE_LESS_DASHCMD='y',
        USER_BUSYBOX_FEATURE_LESS_LINENUMS='y',
        USER_BUSYBOX_INCLUDE_SUSv2='y',
        USER_BUSYBOX_SHOW_USAGE='y',
        USER_BUSYBOX_FEATURE_VERBOSE_USAGE='y',
        USER_BUSYBOX_FEATURE_COMPRESS_USAGE='y',
        USER_BUSYBOX_LONG_OPTS='y',
        USER_BUSYBOX_FEATURE_DEVPTS='y',
        USER_BUSYBOX_FEATURE_EDITING='y',
        USER_BUSYBOX_FEATURE_TAB_COMPLETION='y',
        USER_BUSYBOX_FEATURE_NON_POSIX_CP='y',
        USER_BUSYBOX_FEATURE_SKIP_ROOTFS='y',
        USER_BUSYBOX_MONOTONIC_SYSCALL='y',
        USER_BUSYBOX_IOCTL_HEX2STR_ERROR='y',
        USER_BUSYBOX_FEATURE_SEAMLESS_XZ='y',
        USER_BUSYBOX_FEATURE_SEAMLESS_LZMA='y',
        USER_BUSYBOX_FEATURE_SEAMLESS_BZ2='y',
        USER_BUSYBOX_FEATURE_SEAMLESS_GZ='y',
        USER_BUSYBOX_GUNZIP='y',
        USER_BUSYBOX_GZIP='y',
        USER_BUSYBOX_FEATURE_GZIP_LONG_OPTIONS='y',
        USER_BUSYBOX_TAR='y',
        USER_BUSYBOX_FEATURE_TAR_CREATE='y',
        USER_BUSYBOX_FEATURE_TAR_FROM='y',
        USER_BUSYBOX_FEATURE_TAR_NOPRESERVE_TIME='y',
        USER_BUSYBOX_FEATURE_TAR_GNU_EXTENSIONS='y',
        USER_BUSYBOX_BASENAME='y',
        USER_BUSYBOX_CAT='y',
        USER_BUSYBOX_DATE='y',
        USER_BUSYBOX_FEATURE_DATE_ISOFMT='y',
        USER_BUSYBOX_FEATURE_DATE_COMPAT='y',
        USER_BUSYBOX_FEATURE_DATE_NANO='y',
        USER_BUSYBOX_ID='y',
        USER_BUSYBOX_GROUPS='y',
        USER_BUSYBOX_TEST='y',
        USER_BUSYBOX_TOUCH='y',
        USER_BUSYBOX_FEATURE_TOUCH_SUSV3='y',
        USER_BUSYBOX_TR='y',
        USER_BUSYBOX_CHGRP='y',
        USER_BUSYBOX_CHMOD='y',
        USER_BUSYBOX_CHOWN='y',
        USER_BUSYBOX_FEATURE_CHOWN_LONG_OPTIONS='y',
        USER_BUSYBOX_CHROOT='y',
        USER_BUSYBOX_CP='y',
        USER_BUSYBOX_FEATURE_CP_LONG_OPTIONS='y',
        USER_BUSYBOX_CUT='y',
        USER_BUSYBOX_DD='y',
        USER_BUSYBOX_DF='y',
        USER_BUSYBOX_FEATURE_DF_FANCY='y',
        USER_BUSYBOX_DIRNAME='y',
        USER_BUSYBOX_DU='y',
        USER_BUSYBOX_FEATURE_DU_DEFAULT_BLOCKSIZE_1K='y',
        USER_BUSYBOX_ECHO='y',
        USER_BUSYBOX_FEATURE_FANCY_ECHO='y',
        USER_BUSYBOX_ENV='y',
        USER_BUSYBOX_EXPR='y',
        USER_BUSYBOX_EXPR_MATH_SUPPORT_64='y',
        USER_BUSYBOX_FALSE='y',
        USER_BUSYBOX_FSYNC='y',
        USER_BUSYBOX_HEAD='y',
        USER_BUSYBOX_LN='y',
        USER_BUSYBOX_LS='y',
        USER_BUSYBOX_FEATURE_LS_FILETYPES='y',
        USER_BUSYBOX_FEATURE_LS_FOLLOWLINKS='y',
        USER_BUSYBOX_FEATURE_LS_RECURSIVE='y',
        USER_BUSYBOX_FEATURE_LS_SORTFILES='y',
        USER_BUSYBOX_FEATURE_LS_TIMESTAMPS='y',
        USER_BUSYBOX_FEATURE_LS_USERNAME='y',
        USER_BUSYBOX_MD5SUM='y',
        USER_BUSYBOX_MKDIR='y',
        USER_BUSYBOX_MKFIFO='y',
        USER_BUSYBOX_MKNOD='y',
        USER_BUSYBOX_MV='y',
        USER_BUSYBOX_NICE='y',
        USER_BUSYBOX_PRINTF='y',
        USER_BUSYBOX_PWD='y',
        USER_BUSYBOX_READLINK='y',
        USER_BUSYBOX_FEATURE_READLINK_FOLLOW='y',
        USER_BUSYBOX_RM='y',
        USER_BUSYBOX_RMDIR='y',
        USER_BUSYBOX_SLEEP='y',
        USER_BUSYBOX_SORT='y',
        USER_BUSYBOX_STAT='y',
        USER_BUSYBOX_FEATURE_STAT_FORMAT='y',
        USER_BUSYBOX_STTY='y',
        USER_BUSYBOX_SYNC='y',
        USER_BUSYBOX_TAIL='y',
        USER_BUSYBOX_FEATURE_FANCY_TAIL='y',
        USER_BUSYBOX_TEE='y',
        USER_BUSYBOX_TRUE='y',
        USER_BUSYBOX_TTY='y',
        USER_BUSYBOX_UNAME='y',
        USER_BUSYBOX_UNAME_OSNAME='Digi Accelerated Linux',
        USER_BUSYBOX_UNIQ='y',
        USER_BUSYBOX_USLEEP='y',
        USER_BUSYBOX_WC='y',
        USER_BUSYBOX_YES='y',
        USER_BUSYBOX_FEATURE_HUMAN_READABLE='y',
        USER_BUSYBOX_CLEAR='y',
        USER_BUSYBOX_RESET='y',
        USER_BUSYBOX_MKTEMP='y',
        USER_BUSYBOX_WHICH='y',
        USER_BUSYBOX_VI='y',
        USER_BUSYBOX_FEATURE_VI_8BIT='y',
        USER_BUSYBOX_FEATURE_VI_COLON='y',
        USER_BUSYBOX_FEATURE_VI_YANKMARK='y',
        USER_BUSYBOX_FEATURE_VI_SEARCH='y',
        USER_BUSYBOX_FEATURE_VI_USE_SIGNALS='y',
        USER_BUSYBOX_FEATURE_VI_DOT_CMD='y',
        USER_BUSYBOX_FEATURE_VI_READONLY='y',
        USER_BUSYBOX_FEATURE_VI_SETOPTS='y',
        USER_BUSYBOX_FEATURE_VI_SET='y',
        USER_BUSYBOX_FEATURE_VI_WIN_RESIZE='y',
        USER_BUSYBOX_FEATURE_VI_ASK_TERMINAL='y',
        USER_BUSYBOX_CMP='y',
        USER_BUSYBOX_SED='y',
        USER_BUSYBOX_FEATURE_ALLOW_EXEC='y',
        USER_BUSYBOX_FIND='y',
        USER_BUSYBOX_FEATURE_FIND_MTIME='y',
        USER_BUSYBOX_FEATURE_FIND_PERM='y',
        USER_BUSYBOX_FEATURE_FIND_TYPE='y',
        USER_BUSYBOX_FEATURE_FIND_NEWER='y',
        USER_BUSYBOX_FEATURE_FIND_SIZE='y',
        USER_BUSYBOX_FEATURE_FIND_LINKS='y',
        USER_BUSYBOX_FEATURE_FIND_EXEC='y',
        USER_BUSYBOX_FEATURE_FIND_NOT='y',
        USER_BUSYBOX_FEATURE_FIND_PATH='y',
        USER_BUSYBOX_FEATURE_FIND_MAXDEPTH='y',
        USER_BUSYBOX_GREP='y',
        USER_BUSYBOX_EGREP='y',
        USER_BUSYBOX_FGREP='y',
        USER_BUSYBOX_FEATURE_GREP_CONTEXT='y',
        USER_BUSYBOX_XARGS='y',
        USER_BUSYBOX_HALT='y',
        USER_BUSYBOX_REBOOT='y',
        USER_BUSYBOX_POWEROFF='y',
        USER_BUSYBOX_USE_BB_CRYPT='y',
        USER_BUSYBOX_USE_BB_CRYPT_SHA='y',
        USER_BUSYBOX_DMESG='y',
        USER_BUSYBOX_FREERAMDISK='y',
        USER_BUSYBOX_MORE='y',
        USER_BUSYBOX_MOUNT='y',
        USER_BUSYBOX_FEATURE_MOUNT_NFS='y',
        USER_BUSYBOX_FEATURE_MOUNT_FLAGS='y',
        USER_BUSYBOX_RDATE='y',
        USER_BUSYBOX_UMOUNT='y',
        USER_BUSYBOX_FEATURE_UMOUNT_ALL='y',
        USER_BUSYBOX_FEATURE_MOUNT_LOOP='y',
        USER_BUSYBOX_FEATURE_MOUNT_LOOP_CREATE='y',
        USER_BUSYBOX_TIME='y',
        USER_BUSYBOX_TIMEOUT='y',
        USER_BUSYBOX_VOLNAME='y',
        USER_BUSYBOX_WATCHDOG='y',
        USER_BUSYBOX_WHOIS='y',
        USER_BUSYBOX_IOSTAT='y',
        USER_BUSYBOX_LSOF='y',
        USER_BUSYBOX_TOP='y',
        USER_BUSYBOX_FEATURE_TOP_CPU_USAGE_PERCENTAGE='y',
        USER_BUSYBOX_FEATURE_TOP_CPU_GLOBAL_PERCENTS='y',
        USER_BUSYBOX_FEATURE_TOP_SMP_CPU='y',
        USER_BUSYBOX_FEATURE_TOP_SMP_PROCESS='y',
        USER_BUSYBOX_UPTIME='y',
        USER_BUSYBOX_FREE='y',
        USER_BUSYBOX_FUSER='y',
        USER_BUSYBOX_KILL='y',
        USER_BUSYBOX_KILLALL='y',
        USER_BUSYBOX_PIDOF='y',
        USER_BUSYBOX_PS='y',
        USER_BUSYBOX_FEATURE_PS_LONG='y',
        USER_BUSYBOX_RENICE='y',
        USER_BUSYBOX_LOGGER='y',
        LIB_ZLIB='y',
        LIB_TERMCAP='y',
        LIB_TERMCAP_SHARED='y',
        USER_INIT_INIT='y',
        USER_INIT_CONSOLE_SH='y',
        USER_GETTYD_GETTYD='y',
        USER_FLATFSD_FLATFSD='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_FLATFSD_EXTERNAL_INIT='y',
        USER_MAWK_AWK='y',
        POOR_ENTROPY=['y', 'n'],
        USER_UDEV='n',
        USER_RAMIMAGE_NONE='y',
        )
    )

groups['user_firewall'] = dict(
    include = [
        'hostapd',
        ],
    linux = dict(
        NETFILTER_XT_SET='m',
        IP_SET='m',
        IP_SET_MAX=256,
        IP_SET_HASH_IP='m',
        IP_SET_HASH_NET='m',
        IP_SET_HASH_NETIFACE='m',
        ),
    user = dict(
        USER_IPSET_IPSET='y',
        USER_IPTABLES_IPTABLES='y',
        USER_CONNTRACK_CONNTRACK='y',
        USER_ARPTABLES_ARPTABLES='y',
        )
    )

groups['busybox_httpd'] = dict(
    user = dict(
        USER_STUNNEL_STUNNEL='y',
        USER_BUSYBOX_PAM='y',
        USER_BUSYBOX_HTTPD='y',
        USER_BUSYBOX_FEATURE_HTTPD_RANGES='y',
        USER_BUSYBOX_FEATURE_HTTPD_SETUID='y',
        USER_BUSYBOX_FEATURE_HTTPD_BASIC_AUTH='y',
        USER_BUSYBOX_FEATURE_HTTPD_CGI='y',
        USER_BUSYBOX_FEATURE_HTTPD_CONFIG_WITH_SCRIPT_INTERPR='y',
        USER_BUSYBOX_FEATURE_HTTPD_SET_REMOTE_PORT_TO_ENV='y',
        USER_BUSYBOX_FEATURE_HTTPD_ENCODE_URL_STR='y',
        USER_BUSYBOX_FEATURE_HTTPD_ERROR_PAGES='y',
        USER_BUSYBOX_FEATURE_HTTPD_PROXY='y',
        USER_BUSYBOX_FEATURE_HTTPD_ETAG='y',
        USER_BUSYBOX_FEATURE_HTTPD_LAST_MODIFIED='y',
        USER_BUSYBOX_FEATURE_HTTPD_DATE='y',
        USER_BUSYBOX_FEATURE_HTTPD_ACL_IP='y',
        )
    )

groups['flashrom'] = dict(
    linux = dict(
        DEVMEM='y',
        ),
    user = dict(
        USER_FLASHROM='y',
        )
    )

groups['hostapd'] = dict(
    user = dict(
        USER_HOSTAPD_HOSTAPD='y',
        USER_HOSTAPD_HOSTAPD_CLI='y',
        )
    )

groups['init_overlay'] = dict(
    linux = dict(
        OVERLAY_FS=['y', 'n'],
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW=['y', 'n'],
        ),
    user = dict(
        USER_INIT_INIT='y',
        USER_INIT_OVERLAY='y',
        USER_BUSYBOX_PIVOT_ROOT='y',
        USER_BUSYBOX_CHROOT='y',
        )
    )

groups['config_iperf'] = dict(
    user = dict(
        C_PLUS_PLUS='y',
        USER_IPERF_IPERF='y',
        )
    )

groups['config_dnsmasq2'] = dict(
    user = dict(
        USER_DNSMASQ2_DNSMASQ2='y',
        PROP_WEBFILTER='y',
    )
)

groups['config_hotspot'] = dict(
    user = dict(
        USER_COOVACHILLI='y',
    )
)

groups['config_watchdog_advanced'] = dict(
    user = dict(
        LIB_LIBUEV='y',
        LIB_LIBITE='y',
        LIB_LIBCONFUSE='y',
        USER_WATCHDOG_ADVANCED='y',
    )
)

groups['config_imx_temp_scaling'] = dict(
    user = dict(
        PROP_IMX_TEMP_SCALING='y',
        ),
    linux = dict(
        HWMON='y',
        THERMAL='y',
        THERMAL_HWMON='y',
        THERMAL_OF='y',
        IMX_THERMAL='y',
        )
    )

groups['snort'] = dict(
    user = dict(
        USER_SNORT_SNORT='y',
        USER_SNORTRULES='y',
        LIB_DAQ='y',
        LIB_LIBDNET='y',
        )
    )

groups['ipv6'] = dict(
    include = [
        'linux_ip6tables',
        'linux_ipv6',
        ],
    user = dict(
        USER_IPTABLES_IP6TABLES='y',
        USER_PPPD_WITH_IPV6='y',
        USER_ODHCP6C='y',
        )
    )

groups['netflow'] = dict(
    modules = dict(
        MODULES_IPT_NETFLOW='y',
        MODULES_IPT_NETFLOW_DIRECTION='y',
        MODULES_IPT_NETFLOW_SAMPLER='y',
        MODULES_IPT_NETFLOW_SAMPLER_HASH='y',
        MODULES_IPT_NETFLOW_NAT='y',
        MODULES_IPT_NETFLOW_PHYSDEV='y',
        MODULES_IPT_NETFLOW_MAC='y',
        )
    )

groups['openswan'] = dict(
    user = dict(
        USER_OPENSWAN='y',
        USER_OPENSWAN_PLUTO_PLUTO='y',
        USER_OPENSWAN_PLUTO_WHACK='y',
        USER_OPENSWAN_PROGRAMS_LWDNSQ='y',
        USER_OPENSWAN_CONFDIR='/etc/config',
        USER_OPENSWAN_UTILS_RANBITS='y',
        USER_OPENSWAN_UTILS_RSASIGKEY='y',
        )
    )

groups['openswan_klips'] = dict(
    include = [
        'openswan',
        ],
    linux = dict(
        KLIPS='m',
        KLIPS_ESP='y',
        KLIPS_AH='y',
        KLIPS_IPCOMP='y',
        KLIPS_DEBUG='y',
        KLIPS_IF_MAX='4',
        KLIPS_IF_NUM='2',
        ),
    user = dict(
        USER_OPENSWAN_KLIPS_EROUTE='y',
        USER_OPENSWAN_KLIPS_KLIPSDEBUG='y',
        USER_OPENSWAN_KLIPS_SPI='y',
        USER_OPENSWAN_KLIPS_SPIGRP='y',
        USER_OPENSWAN_KLIPS_TNCFG='y',
        )
    )

groups['openswan_klips_ocf'] = dict(
    include = [
        'openswan_klips',
        ],
    linux = dict(
        KLIPS_OCF='y',
        )
    )

groups['openswan_netkey'] = dict(
    include = [
        'linux_netkey',
        'linux_netkey_ipv6',
        'openswan',
        ],
    user = dict(
        )
    )

groups['strongswan'] = dict(
    user = dict(
        LIB_LIBLDNS='y',
        LIB_LIBUNBOUND='y',
        USER_STRONGSWAN='y',
        )
    )

groups['config_aircrack_ng'] = dict(
        user = dict(
            USER_AIRCRACK_NG='y',
            USER_AIRCRACK_NG_AIRODUMP='y',
            USER_AIRCRACK_NG_OSDEP='y',
            )
        )

groups['strongswan_netkey'] = dict(
    include = [
        'linux_netkey',
        'linux_netkey_ipv6',
        'strongswan',
        ],
    user = dict(
        )
    )

groups['tcpdump'] = dict(
    user = dict(
        LIB_LIBPCAP='y',
        LIB_LIBPCAP_STATIC='y',
        USER_TCPDUMP_TCPDUMP='y',
        )
    )

groups['usb'] = dict(
    include = [
        'linux_usb',
        ],
    user = dict(
        LIB_LIBUSB_COMPAT='y',
        LIB_LIBUSB='y',
        USER_BUSYBOX_LSUSB=['y','n'],
        )
    )

groups['linux_hpet'] = dict(
    linux = dict(
        HPET='y',
        HPET_MMAP='y',
        HPET_MMAP_DEFAULT='y',
        )
    )

groups['virtualisation_extras'] = dict(
    include = [
        'linux_bridge_netfilter',
        ],
    linux = dict(
        MACVLAN='m',
        MACVTAP='m',
        LIBCRC32C='y',
        BALLOON_COMPACTION='y',
        PCI_STUB='y',
        NET_FOU='y',
        VXLAN='y',
        GENEVE='y'
    ),
    user = dict(
        USER_LIBVIRT='y',
        LIB_LIBXML2='y',
        #USER_OVS='y',
        USER_BUSYBOX_INSTALL='y',
        USER_BUSYBOX_FEATURE_INSTALL_LONG_OPTIONS='y',
        USER_WEBSOCKIFY='y',
        USER_NOVNC='y',
        USER_NOVNC_ROMFSDIR='/home/httpd/novnc',
        USER_SHELLINABOX='y'
        )
    )

# Dependencies for features under prop/config
groups['config'] = dict(
    include = [
        'linux_bridge',
        'linux_common',
        'user_common',
        'config_firewall',
        ],
    user = dict(
        PROP_CONFIG='y',
        PROP_CONFIG_CRYPTBACKUP='y',
        LIB_MUSL_HAS_TZ_FILE='y',
        LIB_MUSL_TZ_FILE_PATH='/var/run/TZ',
        LIB_YAJL='y',
        USER_BUSYBOX_FLOCK='y',
        USER_RESOLVEIP_RESOLVEIP='y',
        USER_JERRYSCRIPT='y',
        PROP_CONFIG_ALLOW_SHELL=['y', 'n'],
        )
    )

groups['config_aview'] = dict(
    user = dict(
        PROP_CONFIG_AVIEW='y',
        PROP_ACCNS_CERTIFICATES='y',
        )
    )

groups['config_samba'] = dict(
    user = dict(
        LIB_LIBKRB5='y',
        LIB_GNUTLS='y',
        LIB_LIBNETTLE='y',
        USER_SAMBA='y',
        USER_SAMBA_SMBD='y',
        USER_SAMBA_NMBD='y',
        )
    )

groups['config_factory'] = dict(
    include = [
        'linux_common',
        'linux_iptables_ipv4',
        'user_firewall',
        'busybox',
        'busybox_ash',
        'init_overlay',
        'tcpdump',
        'config_dnsmasq2',
        'config_cellular',
        'config_samba',
        ],
    linux = dict(
        IP_NF_ARPTABLES='m',
        IP_NF_ARPFILTER='m',
        IP_NF_ARP_MANGLE='m',
        ),
    user = dict(
        LIB_MUSL_HAS_TZ_FILE='y',
        LIB_MUSL_TZ_FILE_PATH='/etc/TZ',
        LIB_OPENSSL='y',
        LIB_LIBGMP='y',
        LIB_LIBNET='y',
        LIB_ZLIB='y',
        LIB_LIBBZ2='y',
        LIB_TERMCAP='y',
        LIB_TERMCAP_SHARED='y',
        LIB_EXPAT='y',
        LIB_GETTEXT='y',
        LIB_GLIB='y',
        LIB_LIBFFI='y',
        LIB_LIBMNL='y',
        LIB_NFNETLINK='y',
        LIB_LIBNL='y',
        LIB_LIBPCRE='y',
        LIB_LIBCURL='y',
        LIB_LIBCURL_CURL='y',
        LIB_LIBIDN2='y',
        USER_INIT_INIT='y',
        USER_INIT_RUN_CONFIGRC='y',
        USER_GETTYD_GETTYD='y',
        USER_FLASHW_FLASHW='y',
        USER_SETMAC_SETMAC='y',
        USER_UBOOT_ENVTOOLS='y',
        USER_NETFLASH_NETFLASH='y',
        USER_NETFLASH_WITH_FTP='y',
        USER_NETFLASH_WITH_CGI='y',
        USER_NETFLASH_HARDWARE='y',
        USER_NETFLASH_DECOMPRESS='y',
        USER_FLATFSD_FLATFSD='y',
        USER_FLATFSD_EXTERNAL_INIT='y',
        USER_DISCARD_DISCARD='y',
        USER_DISCARD_NO_INSTALL='y',
        USER_EBTABLES_EBTABLES='y',
        USER_FTP_FTP_FTP='y',
        USER_INETD_INETD='y',
        USER_IPROUTE2='y',
        USER_IPROUTE2_IP_IP='y',
        USER_IPUTILS_IPUTILS='y',
        USER_IPUTILS_PING='y',
        USER_IPUTILS_PING6='y',
        USER_IPUTILS_TRACEROUTE6='y',
        USER_IPUTILS_TRACEPATH='y',
        USER_IPUTILS_TRACEPATH6='y',
        USER_NETCAT_NETCAT='y',
        USER_NTPD_NTPD='y',
        USER_NTPD_NTPDATE='y',
        USER_NTPD_NTPQ='y',
        USER_SSH_SSH='y',
        USER_SSH_SSHD='y',
        USER_SSH_SSHKEYGEN='y',
        USER_SSH_SCP='y',
        USER_SSH_SFTP='y',
        USER_SSH_SFTP_SERVER='y',
        USER_TCPBLAST_TCPBLAST='y',
        USER_TELNETD_TELNETD='y',
        USER_TRACEROUTE_TRACEROUTE='y',
        USER_NET_TOOLS_ARP='y',
        USER_NET_TOOLS_HOSTNAME='y',
        USER_NET_TOOLS_IFCONFIG='y',
        USER_NET_TOOLS_NETSTAT='y',
        USER_NET_TOOLS_ROUTE='y',
        USER_NET_TOOLS_MII_TOOL='y',
        USER_CHAT_CHAT='y',
        USER_CPU_CPU='y',
        USER_HD_HD='y',
        USER_LEDCMD_LEDCMD='y',
        USER_MAWK_AWK='y',
        USER_STRACE='y',
        USER_TIP_TIP='y',
        USER_RAMIMAGE_NONE='y',
        POOR_ENTROPY='y',
        USER_KMOD='y',
        USER_KMOD_LIBKMOD='y',
        USER_KMOD_TOOLS='y',
        USER_READLINE='y',
        USER_UDEV='y',
        USER_UTIL_LINUX='y',
        USER_UTIL_LINUX_LIBBLKID='y',
        PROP_FACTORY_FACTORY='y',
        USER_EMCTEST_EMCECHO='y',
        USER_INIT_DEFAULT_ENTRIES='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_DNSMASQ_DNSMASQ='y',
        USER_SSH_SSHD_CONFIG='y',
        USER_SSH_GEN_KEYS='y',
        USER_TFTP_HPA='y',
        USER_EXPECT_EXPECT='y',
        USER_BUSYBOX_PASSWD='y',
        USER_BUSYBOX_FEATURE_DEFAULT_PASSWD_ALGO='des',
        USER_BUSYBOX_UDHCPD='y',
        USER_BUSYBOX_DHCPRELAY='y',
        USER_BUSYBOX_FLOCK='y',
        USER_BUSYBOX_DUMPLEASES='y',
        USER_BUSYBOX_FEATURE_UDHCPD_WRITE_LEASES_EARLY='y',
        USER_BUSYBOX_DHCPD_LEASES_FILE='/var/lib/misc/udhcpd.leases',
        USER_BUSYBOX_FEATURE_SH_MATH='y',
        USER_BUSYBOX_FEATURE_SH_MATH_64='y',
        USER_BUSYBOX_SYSLOGD='y',
        USER_BUSYBOX_FEATURE_ROTATE_LOGFILE='y',
        USER_BUSYBOX_FEATURE_REMOTE_LOG='y',
        USER_BUSYBOX_FEATURE_SYSLOGD_CFG='y',
        USER_BUSYBOX_FEATURE_SYSLOGD_READ_BUFFER_SIZE='256',
        USER_BUSYBOX_FEATURE_KMSG_SYSLOG='y',
        USER_BUSYBOX_KLOGD='y',
        USER_BUSYBOX_FEATURE_KLOGD_KLOGCTL='y',
        USER_BUSYBOX_SEQ='y',
        USER_I2C_TOOLS='y',
        )
    )

groups['config_cellular_awusb'] = dict(
    include = [
        'config_cellular_small',
        'linux_usb',
        ],
    user = dict(
        LIB_DBUS='y',
        LIB_DBUS_GLIB='y',
        USER_LIBQMI_QMICLI='y',
        USER_LIBMBIM='y',
        USER_MODEMMANAGER_ALLPLUGINS='y',
        USER_BUSYBOX_MICROCOM='y',
        LIB_LIBUSB_COMPAT='y',
        LIB_LIBUSB='y',
        USER_BUSYBOX_LSUSB='n',
        USER_USBUTILS='y',
        )
    )

groups['config_cellular'] = dict(
    include = [
        'config_cellular_usb',
        ],
    user = dict(
        USER_LIBQMI_QMICLI='y',
        USER_LIBMBIM='y',
        USER_MODEMMANAGER_ALLPLUGINS='y',
        USER_BUSYBOX_MICROCOM='y',
        )
    )

groups['config_cellular_usb'] = dict(
    include = [
        'usb',
        'config_cellular_small',
        ],
    )

groups['config_cellular_small'] = dict(
    user = dict(
        LIB_DBUS='y',
        LIB_DBUS_GLIB='y',
        USER_LIBQMI='y',
        USER_MODEMMANAGER='y',
        USER_MOBILE_BROADBAND_PROVIDER_INFO='y',
        USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_FULL='y',
        PROP_QCDM='y',
        PROP_QCDM_CLEARTEXT=['y','n'],
        )
    )

groups['config_firewall'] = dict(
    include = [
        'linux_iptables',
        'linux_ip6tables',
        'linux_arptables',
        'user_firewall',
        ],
    )

groups['config_serial_small'] = dict(
    user = dict(
        USER_SHELLINABOX='y',
        PROP_CONFIG_XMODEM='y',
        PROP_CONFIG_SERIALD='y',
        PROP_CONFIG_SERIALD_RAW_TCP=['y', 'n'],
        )
    )

groups['config_serial'] = dict(
    include = [
        'config_serial_small'
        ],
    user = dict(
        PROP_CONFIG_SERIALD_AUTOCONNECT='y',
        PROP_CONFIG_SERIALD_DATAFRAMING='y',
        PROP_CONFIG_SERIALD_UDPSERIAL='y',
        PROP_CONFIG_SERIALD_PPP_DIALIN='y',
        )
    )

groups['gnss_software'] = dict(
    user = dict(
        PROP_CONFIG_LOCATION='y',
    )
)

groups['gnss_modem'] = dict(
    include = [ 'gnss_software' ],
    user = dict(
        PROP_CONFIG_LOCATION_MODEM='y',
    )
)

groups['gnss_hardware'] = dict(
    include = [ 'gnss_software' ],
    user = dict(
        PROP_CONFIG_LOCATION_GNSS='y',
        USER_GPSD='y',
        )
    )

groups['config_ddns'] = dict(
    user = dict(
        PROP_CONFIG_DDNSD='y',
        )
    )

groups['config_hostnamesniffer'] = dict(
    user = dict(
        PROP_CONFIG_HOSTNAMESNIFFER='y',
        LIB_LIBPCAP='y',
        LIB_UDNS='y'
        )
    )

groups['config_intelliflow'] = dict(
    user = dict(
        PROP_INTELLIFLOW='y',
        )
    )

groups['config_cloud'] = dict(
    user = dict(
        PROP_CONFIG_CC_ACL='y',
        USER_SOCAT='y',
        )
    )

groups['config_python'] = dict(
    user = dict(
        USER_PYTHON='y',
        PROP_DIGI_DIGIDEVICE='y',
        USER_PYTHON_REMOVE_SOURCE='y',
        USER_PYTHON_MODULES_PAHO_MQTT='y',
        USER_PYTHON_MODULES_PIP='y',
        USER_PYTHON_MODULES_TACACS_PLUS='y',
        LIB_LIBFFI='y',
        )
    )

groups['config_powerman'] = dict(
    user = dict(
        USER_POWERMAN_POWERMAN='y',
        USER_POWERMAN_HTTPPOWER='y',
        )
    )

groups['config_suspend'] = dict(
    linux = dict(
        SUSPEND='y',
        ),
    user = dict(
        PROP_SUSPEND='y',
        )
    )

groups['config_xbeegw'] = dict(
    include = [
        'config_python',
        'config_ledcmd_wrapper',
        'config_modbus_gateway_xbee',
    ],
    user = dict(
        PROP_XBEE='y',
        PROP_XBEE_PROFILES='y',
        PROP_XBEE_DRM='y',
        PROP_XBEE_NETWORK_MANAGER='y',
        PROP_XBEE_STATUS='y',
        USER_PYTHON_MODULES_PYMODBUS='y'
        )
    )

groups['config_xbeegw_bluetooth'] = dict(
    user = dict(
        PROP_XBEE_BLUETOOTH='y',
        PROP_XBEE_BLUETOOTH_CLI='y'
        )
    )


groups['config_power_management'] = dict(
    user = dict(
        PROP_POWER_MANAGEMENT='y',
        ),
    linux = dict(
        CPU_FREQ_GOV_PERFORMANCE='y',
        CPU_FREQ_DEFAULT_GOV_PERFORMANCE='y',
        )
    )

groups['config_gov_powersave'] = dict(
    include = [
        'config_power_management'
    ],
    user = dict(
        PROP_CPU_FREQ_GOV_POWERSAVE='y',
        ),
    linux = dict(
        CPU_FREQ_GOV_POWERSAVE='y',
        )
    )

groups['config_gov_ondemand'] = dict(
    include = [
        'config_power_management'
    ],
    user = dict(
        PROP_CPU_FREQ_GOV_ONDEMAND='y',
        ),
    linux = dict(
        CPU_FREQ_GOV_ONDEMAND='y',
        CPU_FREQ_GOV_COMMON='y',
        CPU_FREQ_GOV_ATTR_SET='y',
        )
    )

groups['config_gov_userspace'] = dict(
    include = [
        'config_power_management'
    ],
    user = dict(
        PROP_CPU_FREQ_GOV_USERSPACE='y',
        ),
    linux = dict(
        CPU_FREQ_GOV_USERSPACE='y',
        )
    )

groups['config_cli'] = dict(
    user = dict(
        PROP_CONFIG_CLI='y',
        LIB_TINYRL='y',
        LIB_TINYRL_DISABLE_BELL='y',
        )
    )

groups['config_cli_legacy'] = dict(
    user = dict(
        PROP_CONFIG_CLI_LEGACY='y',
        PROP_CONFIG_CLI_LEGACY_SHELL_COMMANDS='y',
        LIB_TINYRL='y',
        LIB_TINYRL_DISABLE_BELL='y',
        )
    )

groups['config_cli_legacy_analyzer'] = dict(
    user = dict(
        PROP_CONFIG_CLI_ANALYZER='y',
        USER_BUSYBOX_LESS='y',
        USER_BUSYBOX_FEATURE_LESS_MAXLINES='9999999',
        USER_BUSYBOX_FEATURE_LESS_BRACKETS='y',
        USER_BUSYBOX_FEATURE_LESS_FLAGS='y',
        USER_BUSYBOX_FEATURE_LESS_MARKS='y',
        USER_BUSYBOX_FEATURE_LESS_REGEXP='y',
        USER_BUSYBOX_FEATURE_LESS_WINCH='y',
        USER_BUSYBOX_FEATURE_LESS_ASK_TERMINAL='y',
        USER_BUSYBOX_FEATURE_LESS_DASHCMD='y',
        USER_BUSYBOX_FEATURE_LESS_LINENUMS='y',
        USER_TCPREPLAY='y',
        )
    )

groups['config_stdcpp'] = dict(
    user = dict(
        LIB_INSTALL_LIBATOMIC='y',
        LIB_INSTALL_LIBSTDCPLUS='y',
        )
    )

groups['config_static_stdcpp'] = dict(
    user = dict(
        LIB_INSTALL_LIBATOMIC='n',
        LIB_INSTALL_LIBSTDCPLUS='n',
        LIB_STATIC_LIBSTDCPLUS='y',
        )
    )

groups['config_stlport'] = dict(
    user = dict(
        LIB_STLPORT='y',
        )
    )

groups['config_stlport_shared'] = dict(
    user = dict(
        LIB_STLPORT='y',
        LIB_STLPORT_SHARED='y',
        )
    )

groups['config_security_dac_n'] = dict(
    linux = dict(
        DEFAULT_SECURITY_DAC='n',
        )
    )

groups['config_security_dac_y'] = dict(
    linux = dict(
        DEFAULT_SECURITY_DAC='y',
        LSM=['yama,loadpin,safesetid,integrity','yama,loadpin,safesetid,integrity,apparmor,selinux,smack,tomoyo'],
        )
    )

groups['config_apparmor'] = dict(
    linux = dict(
        SECURITY='y',
        SECURITY_APPARMOR='y',
        SECURITY_APPARMOR_HASH='y',
        SECURITY_APPARMOR_HASH_DEFAULT='y',
        INTEGRITY='y',
        INTEGRITY_AUDIT='y',
        DEFAULT_SECURITY_APPARMOR='y',
        LSM='yama,loadpin,safesetid,integrity,apparmor,selinux,smack,tomoyo',
        ),
    user = dict(
        USER_APPARMOR='y',
        C_PLUS_PLUS='y',
        )
    )

groups['config_nagios'] = dict(
    user = dict(
        PROP_CONFIG_NAGIOS_CLIENT='y'
        )
    )

groups['config_netflow'] = dict(
    include = [
        'netflow',
        'config_hostnamesniffer'
        ],
    modules = dict(
        MODULES_IPT_NETFLOW_PROBE_ID='y',
        MODULES_IPT_NETFLOW_DOMAINS='y',
        )
    )

groups['config_vrrp'] = dict(
    user = dict(
        USER_KEEPALIVED='y',
        USER_KEEPALIVED_MINIMAL='y',
        ),
    linux = dict(
        MACVLAN='m',
        )
    )

groups['config_frrouting'] = dict(
    include = [
        'frrouting',
        ],
    user = dict(
        USER_BUSYBOX_SEQ='y',
        )
    )

groups['config_virt'] = dict(
    include = [
        'qemu_x86',
        'virtualisation_extras',
        ],
    )

groups['config_bluetooth_scanner'] = dict(
    user = dict(
        # Requires config_bluetooth_bluez or config_bluetoogh_bluegiga
        PROP_CONFIG_BLUETOOTH_BLUETOOTH_SCANNER='y',
        )
    )

groups['config_bluetooth_bluez'] = dict(
    user = dict(
        USER_BLUEZ_BLUETOOTHD='y',
        USER_BLUEZ_BLUETOOTHCTRL='y',
        USER_BLUEZ_TOOLS='y',
        USER_BLUEZ_TOOLS_DEPRECATED='y',
        )
    )

groups['config_bluetooth_bluegiga'] = dict(
    user = dict(
        PROP_LIBBGAPI='y',
        )
    )

groups['config_scep_client'] = dict(
        user = dict(
            USER_SCEP_CLIENT='y',
            )
        )

groups['config_analyzer'] = dict(
        user = dict(
            PROP_ANALYZER='y',
            )
        )

groups['config_wireless'] = dict(
    user = dict(
        USER_BUSYBOX_READLINK='y',
        USER_BUSYBOX_FEATURE_READLINK_FOLLOW='y',
        PROP_CONFIG_WEBUI_PORTAL='y',
        )
    )

groups['config_multicast'] = dict(
    linux = dict(
        IP_MROUTE='y',
        IP_MROUTE_MULTIPLE_TABLES='y',
        IP_PIMSM_V1='y',
        IP_PIMSM_V2='y',
        ),
    user = dict(
        USER_SMCROUTE='y',
        )
    )

groups['config_openvpn'] = dict(
    user = dict(
        USER_OPENVPN_OPENVPN='y',
        LIB_LIBLZO='y',
        )
    )

groups['config_iptunnel'] = dict(
    linux = dict(
        NF_CT_PROTO_GRE='y',
        NET_IPGRE_DEMUX='y',
        NET_IP_TUNNEL='y',
        NET_IPGRE='y',
        ),
    user = dict(
        PROP_CONFIG_IPTUNNEL='y',
        )
    )

groups['config_l2tp'] = dict(
    linux = dict(
        L2TP='y',
        ),
    )

groups['config_l2tpeth'] = dict(
    include = [
        'config_l2tp',
        ],
    linux = dict(
        L2TP_V3='y',
        L2TP_IP='y',
        L2TP_ETH='y',
        ),
    user = dict(
        PROP_CONFIG_L2TPETH='y',
        )
    )

groups['config_pppol2tp'] = dict(
    include = [
        'config_l2tp',
        ],
    linux = dict(
        PPPOL2TP='y',
        ),
    user = dict(
        USER_PPPD_WITH_PPPOL2TP='y',
        USER_XL2TPD_XL2TPD='y',
        PROP_CONFIG_PPPOL2TP='y',
        )
    )

groups['config_pppoe_server'] = dict(
    user = dict(
        USER_RP_PPPOE='y',
        PROP_CONFIG_PPPOE_SERVER='y',
        )
    )

groups['config_bonding'] = dict(
    linux = dict(
        BONDING='y',
        ),
    user = dict(
        PROP_CONFIG_BONDING='y',
        )
    )


groups['config_snmp'] = dict(
    user = dict(
        USER_NETSNMP_SNMPD='y',
        USER_NETSNMP_V1='n',
        USER_NETSNMP_V2C=[ 'y', 'n' ],
        USER_NETSNMP_ADDMIBS='y',
        USER_NETSNMP_ADDITIONALMIBS="accelerated",
        USER_NETSNMP_ADDITIONAL_MIB_DIRS="$$(ROOTDIR)/prop/config/snmp",
        )
    )

groups['config_qos'] = dict(
    linux = dict(
        NET_SCHED='y',
        NET_SCH_CBQ='y',
        NET_SCH_HTB='y',
        NET_SCH_HFSC='y',
        NET_SCH_PRIO='y',
        NET_SCH_SFB='y',
        NET_SCH_SFQ='y',
        NET_SCH_TBF='y',
        NET_SCH_QFQ='y',
        NET_SCH_CODEL='y',
        NET_SCH_FQ_CODEL='y',
        NET_SCH_FQ='y',
        NET_SCH_PLUG='y',
        NET_SCH_DEFAULT='y',
        DEFAULT_PFIFO_FAST='y',
        NET_CLS_BASIC='y',
        NET_CLS_TCINDEX='y',
        NET_CLS_ROUTE4='y',
        NET_CLS_U32='y',
        NET_CLS_RSVP='y',
        NET_CLS_RSVP6='y',
        NET_CLS_FLOW='y',
        NET_CLS_BPF='y',
        NET_CLS_FLOWER='y',
        NET_CLS_MATCHALL='y',
        NET_EMATCH='y',
        NET_EMATCH_CMP='y',
        NET_EMATCH_NBYTE='y',
        NET_EMATCH_U32='y',
        NET_EMATCH_META='y',
        NET_EMATCH_STACK=32,
        ),
    user = dict(
        USER_IPROUTE2_TC_TC='y',
        )
    )

groups['config_digimdns'] = dict(
    user = dict(
        PROP_DIGIMDNS='y',
        )
    )

groups['config_itxpt'] = dict(
    user = dict(
        PROP_CONFIG_ITXPT='y',
        )
    )

groups['config_mqtt_broker'] = dict(
    user = dict(
        PROP_CONFIG_MQTT_BROKER='y',
        )
    )

groups['config_ledcmd_wrapper'] = dict(
    user = dict(
        PROP_CONFIG_LEDCMD_WRAPPER='y'
    )
)

groups['config_all_firmware'] = dict(
    user = dict(
        USER_LINUX_FIRMWARE_ALL_FIRMWARE='y',
    )
)

groups['config_encrypted_configfs'] = dict(
    linux = dict(
        MD='y',
        BLK_DEV_DM='y',
        DM_CRYPT='y',
        CRYPTO_USER_API_SKCIPHER='y',
        CRYPTO_USER_API_AEAD='y',
        CRYPTO_XTS='y',
        ),
    user = dict(
        LIB_LIBAIO='y',
        LIB_LIBDEVMAPPER='y',
        LIB_LIBARGON2='y',
        USER_E2FSPROGS_BLKID='n',
        USER_UTIL_LINUX_LIBBLKID='y',
        USER_CRYPTSETUP='y',
        PROP_CONFIGFS_TOOL='y',
        )
    )

groups['config_encrypted_loop_configfs'] = dict(
    include = [
        'config_encrypted_configfs',
        ],
    linux = dict(
        BLK_DEV_LOOP='y',
        BLK_DEV_LOOP_MIN_COUNT='8',
        ),
    user = dict(
        USER_UTIL_LINUX_FALLOCATE='y',
        USER_UTIL_LINUX_LOSETUP='y',
        USER_BUSYBOX_LOSETUP='n',
        USER_BUSYBOX_FEATURE_XARGS_SUPPORT_REPL_STR='y',
        PROP_CONFIGFS_LOOP_DEVICE='y',
        )
    )

groups['config_rtc'] = dict(
    user = dict(
        USER_HWCLOCK_HWCLOCK='y',
        )
    )

groups['config_custdefcfg'] = dict(
    user = dict(
        PROP_CONFIG_INIT_D_CUSTOM_DEFAULT_CONFIG='y',
        )
    )

groups['config_base_features'] = dict(
    include = [
        'config_cli',
        'config_cloud',
        'config_aview',
        'config_frrouting',
        'config_nagios',
        'config_vrrp',
        'config_ddns',
        'config_netflow',
        'config_intelliflow',
        'config_snmp',
        'config_multicast',
        'config_openvpn',
        'config_qos',
        'config_iptunnel',
        'config_l2tpeth',
        'config_pppol2tp',
        'pppd_pppoe',
        'config_pppoe_server',
        'config_digimdns',
        'config_iperf',
        'config_analyzer',
        'config_custdefcfg',
        'gnss_software',
        'config_apparmor',
        'config_lxc',
        'varlog_mount',
        ],
    )

groups['config_python_live_image'] = dict(
    include = [
        'config_python',
        ],
    user = dict(
        USER_PYTHON_LIVE_IMAGE='y',
        ),
    )

groups['config_nemo'] = dict(
    user = dict(
        PROP_CONFIG_NEMO='y',
        ),
    )

groups['config_duplicate_flash_image'] = dict(
    user = dict(
        PROP_CONFIG_RUNT_FIRMWARE_ALT_BOOT='y',
        PROP_DUPLICATE_FLASH_IMAGE='y',
        ),
    )

groups['config_usb_serial_ch341'] = dict(
    linux = dict(
        USB_SERIAL_CH341='y',
        ),
    )

groups['config_modbus_gateway'] = dict(
    user = dict(
        PROP_MODBUS_GATEWAY='y',
        ),
    )

groups['config_modbus_gateway_xbee'] = dict(
    include = [
        'config_modbus_gateway',
        ],
    user = dict(
        PROP_MODBUS_GATEWAY_XBEE='y',
        ),
    )

groups['config_app_monitor'] = dict(
    user = dict(
        PROP_APP_MONITOR='y',
        LIB_LIBUBOX='y',
        USER_UBUS='y',
        ),
    )

groups['config_app_monitor_python'] = dict(
    include = [
        'config_app_monitor',
        ],
    user = dict(
        PROP_APP_MONITOR_PYTHON_BINDINGS='y',
        PROP_DIGI_DIGIDEVICE='y',
        USER_PYTHON_MODULES_UBUS='y',
        ),
    )

groups['config_lxc'] = dict(
    linux = dict(
        SWAP='y',
        POSIX_MQUEUE='y',
        CGROUPS='y',
        MEMCG='y',
        BLK_CGROUP='y',
        CGROUP_SCHED='y',
        FAIR_GROUP_SCHED='y',
        CGROUP_PIDS='y',
        CGROUP_FREEZER='y',
        CGROUP_DEVICE='y',
        CGROUP_CPUACCT='y',
        NAMESPACES='y',
        UTS_NS='y',
        IPC_NS='y',
        USER_NS='y',
        PID_NS='y',
        NET_NS='y',
        VETH='y',
        ),
    user = dict(
        USER_BUSYBOX_UNXZ='y',
        USER_BUSYBOX_XZ='y',
        USER_LXC=[ 'y', 'n' ],
        USER_TAR_TAR='y',
        ),
    )

groups['disable_lxc'] = dict(
    user = dict(
        USER_LXC='n',
        )
    )

groups['config_realport'] = dict(
    modules = dict(
        MODULES_N_REALPORT='m'
        ),
    user = dict(
        PROP_CONFIG_REALPORT='y'
        )
    )

groups['config_realport_imx6'] = dict(
    include = [
        'config_realport',
        ],
    linux = dict(
        SERIAL_IMX6_DIGI_EXTENSIONS='y',
        ),
    )

groups['config_realport_tx54'] = dict(
    include = [
        'config_realport',
        ],
    linux = dict(
        SERIAL_CP210X_DIGI_REALPORT_EXTENSIONS='y',
        ),
    )

groups['config_nfs_client'] = dict(
    linux = dict(
        NETWORK_FILESYSTEMS='y',
        NFS_FS='y',
        NFS_V2='y',
        NFS_V3='y',
        NFS_V3_ACL='y',
        NFS_V4='y',
        NFS_USE_KERNEL_DNS='y',
        NFS_DISABLE_UDP_SUPPORT='y',
        GRACE_PERIOD='y',
        LOCKD='y',
        LOCKD_V4='y',
        NFS_ACL_SUPPORT='y',
        NFS_COMMON='y',
        SUNRPC='y',
        SUNRPC_GSS='y',
        RPCSEC_GSS_KRB5='y',
        CRYPTO_CTS='y',
        ),
    user = dict(
        PROP_CONFIG_NFS='y',
        )
    )

groups['config_5400_rm'] = dict(
    include = [
        'config_5401_rm',
        'config_cellular',
        'config_nemo',
        ],
    )

groups['config_5401_rm'] = dict(
    include = [
        'config',
        'config_cli',
        'config_cloud',
        'config_aview',
        'config_serial_small',
        'config_frrouting',
        'config_nagios',
        'config_netflow',
        'config_vrrp',
        'config_ddns',
        'config_qos',
        'config_iptunnel',
        'config_duplicate_flash_image',
        # OLD PRODUCT: ESSENTIAL FEATURES ONLY
        ],
    )

groups['config_connectit'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular',
        'config_nemo',
        'config_serial',
        'config_powerman',
        'config_scep_client',
        ],
    user = dict(
        PROP_CONFIG_SERIAL_EXCLUSIVE_ACCESS='y',
        ),
    )

groups['config_connectit_mini'] = dict(
    include = [
        'config_connectit',
        'config_stdcpp',
        'config_python_live_image',
        'config_ledcmd_wrapper',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        ],
    )

groups['config_ex12'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular',
        'config_nemo',
        'config_stdcpp',
        'config_ledcmd_wrapper',
        'config_serial',
        'config_bonding',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'config_scep_client',
        'config_python_live_image',
        ],
    )

groups['config_ix'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_stdcpp',
        'config_python',
        'config_ledcmd_wrapper',
        'config_cellular',
        'config_nemo',
        'config_imx_temp_scaling',
        'config_duplicate_flash_image',
        'config_scep_client',
        'config_serial',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'config_power_management',
        'config_gov_powersave',
        'config_gov_ondemand',
        'config_gov_userspace',
        ],
    )

groups['config_ix10'] = dict(
    include = [
        'config_ix',
        'gnss_modem',
        'config_mqtt_broker',
        ],
    )

groups['config_ix15'] = dict(
    include = [
        'config_app_monitor_python',
        'config_ix',
        'config_xbeegw',
        'config_rtc',
        'config_suspend',
        'config_mqtt_broker',
        ],
    user = dict(
        PROP_PYINSTALL='y',
        ),
    )

groups['config_ix20'] = dict(
    include = [
        'config_ix10',
        'config_bonding',
        'config_wan_bonding',
        'config_mqtt_broker',
        ],
    )

groups['config_ix20w'] = dict(
    include = [
        'config_ix20',
        'config_wireless',
        'config_restrict_5g_channels_ap_only',
        ],
    )

groups['config_ix30'] = dict(
    include = [
        'config_ix20',
        'config_rtc',
        'config_bonding',
        ],
    user=dict(
        PROP_CONFIG_IOD='y',
        )
    )

groups['config_connectit16'] = dict(
    include = [
        'config_connectit48',
        ]
    )

groups['config_connectit48'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular',
        'config_nemo',
        'config_serial',
        'config_python',
        'config_python_hid',
        'config_ledcmd_wrapper',
        'config_powerman',
        'config_rtc',
        'config_bonding',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'config_scep_client',
        'config_nfs_client',
        ],
    user = dict(
        USER_BUSYBOX_MKFS_VFAT='y',
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_MKFS_EXT4='y',
        USER_E2FSPROGS_FSCK_EXT4='y',
        USER_ETHTOOL_ETHTOOL='y',
        USER_PARTED='y',
        PROP_CONFIG_SERIAL_EXCLUSIVE_ACCESS='y',
        PROP_CONFIG_SERIALD_LED='y',
        PROP_CONFIG_SERIALD_AUTODETECT='y',
        PROP_CONFIG_SERIALD_DATAMATCH='y',
        USER_DMIDECODE='y',
        PROP_CONFIG_STORAGE_USB='y',
        ),
    )

groups['config_virtualdal'] = dict(
    include = [
        'config',
        'config_serial',
        'config_base_features',
        'config_python',
        'config_ledcmd_wrapper',
        'config_rtc',
        'config_bonding',
        'config_scep_client',
        ],
    user = dict(
        LIB_XZ='y',
        USER_BUSYBOX_MKFS_VFAT='y',
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_MKFS_EXT4='y',
        USER_E2FSPROGS_FSCK_EXT4='y',
        USER_ETHTOOL_ETHTOOL='y',
        USER_PARTED='y',
        USER_DMIDECODE='y',
        ),
    )

groups['config_6300_cx'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_stdcpp',
        'config_python',
        'config_python_hid',
        'config_ledcmd_wrapper',
        'config_cellular',
        'config_nemo',
        'config_duplicate_flash_image',
        'config_scep_client',
        'disable_lxc',
        ],
    )

groups['config_ix14-base'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular',
        'config_nemo',
        'config_serial',
        'config_python',
        'config_ledcmd_wrapper',
        'config_rtc',
        ],
    user = dict(
        USER_BLE_PROVISIONING_APP='y',
        PROP_BLUETOOTH_QCA65x4_FW='y',
        USER_PYTHON_MODULES_PYSERIAL='y',
        ),
    )

groups['config_ix14'] = dict(
    include = [
        'config_ix14-base',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'config_scep_client',
        'config_mqtt_broker',
        ]
    )

groups['config_lr54'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular',
        'config_nemo',
        'config_serial',
        'config_stdcpp',
        'config_python',
        'config_python_hid',
        'config_ledcmd_wrapper',
        'config_rtc',
        'config_scep_client',
        'config_bonding',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'config_watchdog_advanced',
        ],
    user = dict(
        USER_PYTHON_MODULES_PYSERIAL='y',
        PROP_DIGI_RESET_BUTTON='y',
        PROP_DIGI_RESET_BUTTON_HOLD_TIME='10',
        PROP_DIGI_RESET_USE_LEDCMD='y',
        PROP_WATCHDOG_ADVANCED='y',
        )
    )

groups['config_6310_dx'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_stdcpp',
        'config_python',
        'config_ledcmd_wrapper',
        'config_cellular',
        'config_nemo',
        'config_bonding',
        'config_duplicate_flash_image',
        'gnss_modem',
        'config_scep_client',
        'disable_lxc',
        ],
    )

groups['config_ex15'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular',
        'config_nemo',
        'config_stdcpp',
        'config_serial',
        'config_bonding',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'gnss_modem',
        'config_scep_client',
        'config_ledcmd_wrapper',
        'config_python_live_image',
        'config_wan_bonding',
        ],
    )

groups['config_ex50'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_serial',
        'config_duplicate_flash_image',
        'config_ledcmd_wrapper',
        'config_scep_client',
        'config_restrict_5g_channels_ap_only',
        'config_python_live_image',
        'config_wan_bonding',
        'config_watchdog_advanced',
        ],
    linux = dict(
        NR_CPUS='4',
        OF_OVERLAY='y',
        STACKPROTECTOR_STRONG='y',
        RANDOMIZE_BASE='y',
        OVERLAY_FS='n',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='n',
        ),
    user = dict(
        LIB_LIBCAP='y',
        LIB_LIBGPIOD='y',
        PROP_LEDCMD_SYSFS='y',
        PROP_CONFIG_XMODEM='y',
        PROP_DIGI_RESET_BUTTON='y',
        PROP_DIGI_RESET_BUTTON_HOLD_TIME='0',
        USER_BUSYBOX_READAHEAD='y',
        USER_BUSYBOX_UNICODE_SUPPORT='y',
        USER_SHELLINABOX='y',
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_LSPCI='y',
        USER_SIGS_SIGS='y',
        PROP_WATCHDOG_ADVANCED='y',
        ),
    )

groups['config_restrict_5g_channels'] = dict(
    user = dict(
        PROP_CONFIG_RESTRICT_5G_CHANNELS='y',
        USER_CRDA_REGDB_UNRESTRICTED_WORLD='y',
        ),
    )

groups['config_restrict_5g_channels_ap_only'] = dict(
    user = dict(
        USER_CRDA_REGDB_UNRESTRICTED_WORLD='y',
    ),
)
groups['config_ex15w'] = dict(
    include = [
        'config_ex15',
        'config_wireless',
        'config_restrict_5g_channels_ap_only',
        ],
    user = dict(
        USER_LINUX_FIRMWARE_QCA988X='y',
        ),
    )

groups['config_tx54'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular',
        'config_nemo',
        'config_wireless',
        'config_serial',
        'config_restrict_5g_channels_ap_only',
        'config_python',
        'config_bluetooth_scanner',
        'config_bluetooth_bluegiga',
        'config_python_hid',
        'config_ledcmd_wrapper',
        'config_aircrack_ng',
        'config_scep_client',
        'config_stdcpp',
        'strongswan_netkey',
        'config_hotspot',
        'config_rtc',
        'config_bonding',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'config_wan_bonding',
        'config_itxpt',
        'config_watchdog_advanced',
        'config_mqtt_broker',
        ],
    user = dict(
        USER_PYTHON_MODULES_PYSERIAL='y',
        PROP_DIGI_RESET_BUTTON='y',
        PROP_DIGI_RESET_BUTTON_HOLD_TIME='10',
        PROP_DIGI_RESET_USE_LEDCMD='y',
        PROP_DIGI_RESET_LEDCMD_LEDS="WIFI1 WWAN1_SIGNAL_YELLOW WWAN2_SERVICE_YELLOW WWAN1_SERVICE_GREEN WWAN2_SIGNAL_YELLOW WWAN1_SIGNAL_GREEN GNSS WWAN2_SERVICE_GREEN WWAN2_SIGNAL_GREEN WWAN1_SERVICE_YELLOW",
        USER_LINUX_FIRMWARE_QCA988X='y',
        PROP_WATCHDOG_ADVANCED='y',
        )
    )

groups['config_tx64'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular',
        'config_nemo',
        'config_wireless',
        'config_serial',
        'config_python',
        'config_restrict_5g_channels_ap_only',
        'config_python_hid',
        'config_ledcmd_wrapper',
        'config_aircrack_ng',
        'config_scep_client',
        'config_hotspot',
        'config_encrypted_loop_configfs',
        'config_rtc',
        'config_bonding',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'config_wan_bonding',
        'config_gov_powersave',
        'config_gov_ondemand',
        'config_gov_userspace',
        'config_itxpt',
        'config_watchdog_advanced',
        'config_mqtt_broker',
        ],
    user = dict(
        USER_BUSYBOX_MKFS_VFAT='y',
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_MKFS_EXT4='y',
        USER_E2FSPROGS_FSCK_EXT4='y',
        USER_GPTFDISK='y',
        USER_PARTED='y',
        USER_DMIDECODE='y',
        USER_PYTHON_MODULES_PYSERIAL='y',
        PROP_CONFIGFS_KEY_ATECC_SLOT=8,
        LIB_OPENSSL3='y',
        LIB_OPENSSL_FIPS='y',
        PROP_WATCHDOG_ADVANCED='y',
        ),
    )

groups['config_anywhereusb'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular_awusb',
        'config_nemo',
        'config_rtc',
        'config_python',
        'config_ledcmd_wrapper',
        'config_scep_client',
        ],
    linux = dict(
        DYNAMIC_DEBUG = 'y',
        GPIO_CDEV = 'y',
        GPIO_CDEV_V1 = 'y',
        OF_OVERLAY='y',
        MQ_IOSCHED_DEADLINE='y',
        STACKPROTECTOR_STRONG='y',
        RANDOMIZE_BASE='y',
        USB_AWUSB_DEFAULT_CLAIM_PORT='y',
        USB_MON='y',
        ),
    user = dict(
        USER_BUSYBOX_READAHEAD='y',
        USER_BUSYBOX_UNICODE_SUPPORT='y',
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_FSCK_EXT4='y',
        USER_E2FSPROGS_MKFS_EXT4='y',
        USER_NETFLASH_NETFLASH='n',
        USER_NETFLASH_WITH_FTP='n',
        USER_NETFLASH_WITH_CGI='n',
        USER_NETFLASH_HARDWARE='n',
        USER_NETFLASH_DECOMPRESS='n',
        USER_NTP_ENABLE_HARDENFILE='y',
        PROP_APP_WATCHDOG='y',
        PROP_AWUSBD='y',
        PROP_MIGRATE_TO_DAL='y',
        USER_LEDCMD_LEDCMD='n',
        PROP_LEDCMD_SYSFS='y',
        USER_SHELLINABOX='y',
        PROP_CONFIG_XMODEM='y',
        USER_MERGECAP='y',
        USER_USBHUBCTRL_USBHUBCTRL='y',
        PROP_CONFIG_RUNT_FIRMWARE_ALT_BOOT_DBY='y',
        PROP_DUPLICATE_FLASH_IMAGE='y',
        ),
    )

groups['config_anywhereusb2i'] = dict(
    include = [
        'config_anywhereusb',
        ],
    linux = dict(
        NR_CPUS='4',
        LEDS_LP5569='y',
        MICREL_PHY='y',
        DP83867_PHY='y',
        RTC_NVMEM='y',
        RTC_DRV_DS1307='y',
        GPIO_PCA953X='y',
        GPIO_PCA953X_IRQ='y',
        WATCHDOG='y',
        WATCHDOG_STOP_ON_REBOOT=0,
        WATCHDOG_HANDLE_BOOT_ENABLED='y',
        ARM_SP805_WATCHDOG='y',
        IMX2_WDT='y',
        MEMORY='y',
        ARM_PL172_MPMC='y',
        SYSFS_PERSISTENT_MEM='y',
        USB_USBFS_MEMORY_MB=256,
        ),
    user = dict(
        PROP_DIGI_HWMON='y',
        PROP_DIGI_RESET_BUTTON='y',
        PROP_DIGI_RESET_BUTTON_HOLD_TIME='10',
        PROP_DIGI_RESET_USE_LEDCMD='n',
        LIB_LIBGPIOD='y',
        PROP_USBLEDD='y',
        ),
    )

groups['config_anywhereusb8'] = dict(
    include = [
        'config_anywhereusb',
        ],
    linux = dict(
        NR_CPUS='4',
        LEDS_AWUSB24='y',
        LEDS_PWM='y',
        LEDS_GPIO='y',
        PWM='y',
        MV88X3310P_PHY='y',
        RTC_NVMEM='y',
        RTC_DRV_DS1307='y',
        GPIO_MPC8XXX='y',
        WATCHDOG='y',
        WATCHDOG_STOP_ON_REBOOT=0,
        WATCHDOG_HANDLE_BOOT_ENABLED='y',
        ARM_SP805_WATCHDOG='y',
        IMX2_WDT='y',
        MEMORY='y',
        ARM_PL172_MPMC='y',
        SYSFS_PERSISTENT_MEM='y',
        USB_AWUSB_HUB_WHITELIST='y',
        USB_USBFS_MEMORY_MB=1024,
        ),
    user = dict(
        PROP_DIGI_HWMON='y',
        PROP_DIGI_RESET_BUTTON='y',
        PROP_DIGI_RESET_BUTTON_HOLD_TIME='10',
        PROP_DIGI_RESET_USE_LEDCMD='n',
        LIB_LIBGPIOD='y',
        PROP_USBLEDD='y',
        PROP_SIM_SIM='y',
        PROP_CONFIG_RUNT_FIRMWARE_ALT_BOOT_DBY='y',
        PROP_DUPLICATE_FLASH_IMAGE='y',
        ),
    )

groups['config_anywhereusb24'] = dict(
    include = [
        'config_anywhereusb8',
        'config_bonding',
        ],
    )

groups['config_connectez8'] = dict(
    include = [
        'config_connectez',
        'config_python_live_image',
        ]
    )

groups['config_connectez'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_serial',
        'config_rtc',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_python',
        'config_ledcmd_wrapper',
        'config_realport',
        'config_scep_client',
        ],
    linux = dict(
        NR_CPUS='4',
        OF_OVERLAY='y',
        MQ_IOSCHED_DEADLINE='y',
        STACKPROTECTOR_STRONG='y',
        RANDOMIZE_BASE='y',
        MICREL_PHY='y',
        DP83867_PHY='y',
        RTC_NVMEM='y',
        RTC_DRV_DS1307='y',
        WATCHDOG='y',
        WATCHDOG_STOP_ON_REBOOT=0,
        WATCHDOG_HANDLE_BOOT_ENABLED='y',
        ARM_SP805_WATCHDOG='y',
        IMX2_WDT='y',
        MEMORY='y',
        ARM_PL172_MPMC='y',
        SYSFS_PERSISTENT_MEM='y',
        OVERLAY_FS='n',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='n',
        ),
    user = dict(
        LIB_LIBCAP='y',
        LIB_LIBGPIOD='y',
        PROP_APP_WATCHDOG='y',
        PROP_MIGRATE_TO_DAL='y',
        PROP_LEDCMD_SYSFS='y',
        PROP_CONFIG_XMODEM='y',
        PROP_DIGI_HWMON='y',
        PROP_DIGI_RESET_USE_LEDCMD='n',
        PROP_DIGI_RESET_NUM_LEDS=2,
        PROP_DIGI_BUTTOND='y',
        PROP_CONFIG_SERIALD_LED='y',
        USER_BUSYBOX_READAHEAD='y',
        USER_BUSYBOX_UNICODE_SUPPORT='y',
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_FSCK_EXT4='y',
        USER_E2FSPROGS_MKFS_EXT4='y',
        USER_NTP_ENABLE_HARDENFILE='y',
        USER_LEDCMD_LEDCMD='n',
        USER_SHELLINABOX='y',
        USER_MERGECAP='y',
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_LSPCI='y',
        USER_PARTED='y',
        USER_SIGS_SIGS='y',
        USER_RS485_RS485='y',
        ),
    )

groups['config_anywhereusb2'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_nemo',
        'config_rtc',
        'config_duplicate_flash_image',
        'config_python',
        'config_ledcmd_wrapper',
        'config_scep_client',
        'linux_usb',
        ],
    linux = dict(
        NR_CPUS='4',
        OF_OVERLAY='y',
        MQ_IOSCHED_DEADLINE='y',
        STACKPROTECTOR_STRONG='y',
        RANDOMIZE_BASE='y',
        MICREL_PHY='y',
        DP83867_PHY='y',
        RTC_NVMEM='y',
        RTC_DRV_DS1307='y',
        WATCHDOG='y',
        WATCHDOG_STOP_ON_REBOOT=0,
        WATCHDOG_HANDLE_BOOT_ENABLED='y',
        ARM_SP805_WATCHDOG='y',
        IMX2_WDT='y',
        MEMORY='y',
        ARM_PL172_MPMC='y',
        SYSFS_PERSISTENT_MEM='y',
        OVERLAY_FS='n',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='n',
        DYNAMIC_DEBUG = 'y',
        GPIO_CDEV = 'y',
        GPIO_CDEV_V1 = 'y',
        USB_AWUSB_DEFAULT_CLAIM_PORT='y',
        USB_MON='y',
        USB_USBFS_MEMORY_MB=256,
        ),
    user = dict(
        LIB_LIBCAP='y',
        LIB_LIBGPIOD='y',
        PROP_APP_WATCHDOG='y',
        PROP_LEDCMD_SYSFS='y',
        PROP_CONFIG_XMODEM='y',
        PROP_DIGI_HWMON='y',
        PROP_DIGI_RESET_USE_LEDCMD='n',
        PROP_DIGI_RESET_NUM_LEDS=2,
        PROP_DIGI_BUTTOND='y',
        USER_BUSYBOX_READAHEAD='y',
        USER_BUSYBOX_UNICODE_SUPPORT='y',
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_FSCK_EXT4='y',
        USER_E2FSPROGS_MKFS_EXT4='y',
        USER_NTP_ENABLE_HARDENFILE='y',
        USER_LEDCMD_LEDCMD='n',
        USER_SHELLINABOX='y',
        USER_MERGECAP='y',
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_LSPCI='y',
        USER_PARTED='y',
        PROP_AWUSBD='y',
        USER_USBHUBCTRL_USBHUBCTRL='y',
        PROP_USBLEDD='y',
        USER_USBUTILS='y',
        LIB_LIBUSB_COMPAT='y',
        LIB_LIBUSB='y',
        PROP_USBBENCH='y',
        ),
    )

groups['config_6335_mx'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular',
        'config_nemo',
        'config_stdcpp',
        'config_python',
        'config_ledcmd_wrapper',
        'config_serial_small',
        'config_bonding',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'config_scep_client',
        'disable_lxc',
        ],
    )

groups['config_6330_mx'] = dict(
    include = [
        'config_6335_mx',
        'config_wireless',
        ],
    )

groups['config_6355_sr'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular',
        'config_nemo',
        'config_stdcpp',
        'config_python',
        'config_python_hid',
        'config_ledcmd_wrapper',
        'config_serial_small',
        'config_bonding',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'config_scep_client',
        'disable_lxc',
        ]
    )

groups['config_6350_sr'] = dict(
    include = [
        'config_6355_sr',
        'config_wireless',
        ]
    )

groups['config_8300'] = dict(
    include = [
        'config',
        'config_aview',
        'config_cli',
        'config_cellular',
        'config_nemo',
        'config_intelliflow',
        'config_netflow',
        'config_wireless',
        'config_vrrp',
        'config_ddns',
        'config_iptunnel',
        'config_bonding',
        ]
    )

groups['config_u115'] = dict(
    include = [
        'config',
        'config_aview',
        'config_cli',
        'config_cellular',
        'config_nemo',
        'config_intelliflow',
        'config_frrouting',
        'config_netflow',
        'config_vrrp',
        'config_ddns',
        'config_serial_small',
        'config_iptunnel',
        'config_cloud',
        'config_bonding',
        ]
    )

groups['config_9400_ua'] = dict(
    include = [
        'config',
        'config_aview',
        'config_cli',
        'config_cellular',
        'config_nemo',
        'config_virt',
        'config_frrouting',
        'config_nagios',
        'config_netflow',
        'config_vrrp',
        'config_ddns',
        'config_rtc',
        'config_bonding',
        'config_scep_client',
        ]
    )

groups['config_sprite'] = dict(
    include = [
        'config',
        'config_aview',
        'config_cli',
        'config_cellular',
        'config_nemo',
        ],
    user = dict(
        PROP_JUNIPER='y',
        PROP_JUNIPER_SPRITE='y',
        )
    )

groups['config_wan_bonding'] = dict(
    user = dict(
        PROP_CONFIG_WAN_BONDING='y',
        )
    )

groups['config_primary_responder'] = dict(
    user = dict(
        USER_TELNETD_TELNETD='n',
        USER_BUSYBOX_TELNET='n',
        USER_BUSYBOX_FEATURE_TELNET_TTYPE='n',
        USER_NETFLASH_STRICT_HW_VERSION_CHECK='y',
        PROP_QCDM_CLEARTEXT='n',
        PROP_CONFIG_SERIALD_RAW_TCP='n',
        PROP_CONFIG_ALLOW_SHELL='n',
        PROP_CONFIG_WPA_V1='n',
        PROP_CONFIG_LOCK_HTTPS_CIPHERS='y',
        PROP_CONFIG_AUTH_PASSWORD_COMPLEX='y',
        USER_NETSNMP_V2C='n',
        USER_SSH_SSHD_CUSTOM_CONFIG='n',
        USER_LXC='y',
        ),
    )

products['AC 5400-RM'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '5400-RM',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_5400_rm',
        'config_5400_rm',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'config_security_dac_y',
        ]
    )

products['AC 5401-RM'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '5401-RM',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_5401_rm',
        'config_5401_rm',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'config_security_dac_y',
        ]
    )

products['Digi Connect IT Mini'] = dict(
    vendor = 'Digi',
    product = 'ConnectIT-Mini',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_connectit_mini',
        'config_connectit_mini',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        ]
    )

products['Digi EX12'] = dict(
    vendor = 'Digi',
    product = 'EX12',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ex12',
        'config_ex12',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        ],
    user = dict(
        PROP_BOOTLOADER_UPDATE='y',
        PROP_BOOTLOADER_UPDATE_IMAGE="http://eng.digi.com/builds/DAL/candidate/21.5/21.5.56.54/Digi/EX12/EX12-21.5.56.54-202105191152-bloader-signed.bin",
        PROP_BOOTLOADER_UPDATE_IMAGE_HASH="f1098f290c807beb92f72bf7862c8484a515ef384cb50123213d542582a0923e",
        )
    )

products['Digi EX12-PR'] = dict(
    vendor = 'Digi',
    product = 'EX12-PR',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ex12',
        'config_ex12',
        'config_primary_responder',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        ],
    user = dict(
        PROP_BOOTLOADER_UPDATE='y',
        PROP_BOOTLOADER_UPDATE_IMAGE="http://eng.digi.com/builds/DAL/candidate/21.5/21.5.56.6/Digi/EX12-PR/EX12-PR-21.5.56.6-202105100031-bloader-signed.bin",
        PROP_BOOTLOADER_UPDATE_IMAGE_HASH="f9588954e620ba757bbda1d39c34d99d57b6a4a21468d7a1dd419a29479abf63",
        )
    )

products['Digi IX10'] = dict(
    vendor = 'Digi',
    product = 'IX10',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix10',
        'config_ix10',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'config_realport_imx6',
        ],
    user = dict(
        PROP_FIBOCOM='y'
        )
    )

products['Digi IX15'] = dict(
    vendor = 'Digi',
    product = 'IX15',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix15',
        'config_ix15',
        'config_xbeegw',
        'config_xbeegw_bluetooth',
        'config_all_firmware',
        'busybox_big',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'config_realport_imx6',
        ]
    )

products['Digi IX15-SX'] = dict(
    vendor = 'Digi',
    product = 'IX15-SX',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix15',
        'config_ix15',
        'config_xbeegw',
        'config_all_firmware',
        'busybox_big',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'config_realport_imx6',
        ]
    )

products['Digi IX20'] = dict(
    vendor = 'Digi',
    product = 'IX20',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix20',
        'config_ix20',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'config_realport_imx6',
        ]
    )

products['Digi IX20-PR'] = dict(
    vendor = 'Digi',
    product = 'IX20-PR',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix20',
        'config_ix20',
        'config_all_firmware',
        'config_primary_responder',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        ],
    user = dict(
        PROP_BOOTLOADER_UPDATE='y',
        PROP_BOOTLOADER_UPDATE_IMAGE="http://eng.digi.com/builds/DAL/candidate/21.2/21.2.39.40/Digi/IX20-PR/IX20-PR-21.2.39.40-202102181430-bloader-signed.bin",
        PROP_BOOTLOADER_UPDATE_IMAGE_HASH="07fa3824cd94b7e705576c42f86fdf0bf8f05470b7a18426ec12196edfa9c98e",
        )
    )

products['Digi IX20W'] = dict(
    vendor = 'Digi',
    product = 'IX20W',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix20w',
        'config_ix20w',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'config_realport_imx6',
        ]
    )

products['Digi IX20W-PR'] = dict(
    vendor = 'Digi',
    product = 'IX20W-PR',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix20w',
        'config_ix20w',
        'config_all_firmware',
        'config_primary_responder',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        ],
    user = dict(
        PROP_BOOTLOADER_UPDATE='y',
        PROP_BOOTLOADER_UPDATE_IMAGE="http://eng.digi.com/builds/DAL/candidate/21.2/21.2.39.40/Digi/IX20W-PR/IX20W-PR-21.2.39.40-202102181430-bloader-signed.bin",
        PROP_BOOTLOADER_UPDATE_IMAGE_HASH="f3cc3deb1bc8906df2f3255d86b99ad68a89ce006bc8da29184a743e509bbcee",
        )
    )

products['Digi IX30'] = dict(
    vendor = 'Digi',
    product = 'IX30',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix30',
        'config_ix30',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'config_realport_imx6',
        ]
    )

products['Digi IX30-PR'] = dict(
    vendor = 'Digi',
    product = 'IX30-PR',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix30',
        'config_ix30',
        'config_all_firmware',
        'config_primary_responder',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        ]
    )

products['Digi AnywhereUSB2'] = dict(
    vendor = 'Digi',
    product = 'AnywhereUSB2',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_anywhereusb2',
        'hardware_nor',
        'hardware_mmc',
        'linux_pci_layerscape',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        'config_anywhereusb2',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        ],
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi AnywhereUSB 2'
        )
    )

products['Digi ConnectEZ1'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ1',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez1',
        'hardware_nor',
        'hardware_mmc',
        'linux_pci_layerscape',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        'config_connectez',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        ],
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ Mini'
        )
    )

products['Digi ConnectEZ2'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ2',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez2_4',
        'hardware_nor',
        'hardware_mmc',
        'linux_pci_layerscape',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        'config_connectez',
        'config_cellular',
        'config_nemo',
        'config_bonding',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        ],
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ 2',
        PROP_CONFIG_SERIALD_DIGI_ALTPIN='y',
        )
    )

products['Digi ConnectEZ4'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ4',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez2_4',
        'hardware_nor',
        'hardware_mmc',
        'hardware_wireless_mwifiex_sdio',
        'linux_pci_layerscape',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        'config_connectez',
        'config_cellular',
        'config_nemo',
        'config_wireless',
        'config_restrict_5g_channels_ap_only',
        'config_bonding',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        ],
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ 4',
        PROP_CONFIG_SERIALD_DIGI_ALTPIN='y',
        )
    )

products['Digi ConnectEZ8'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ8',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez_1046',
        'config_cellular',
        'config_nemo',
        'config_connectez8',
        'config_cellular',
        'config_bonding',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        'exfat',
        'storage',
        'config_nfs_client',
        ],
    linux = dict(
    ),
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ 8',
        PROP_USBLEDD='y',
    )
    )

products['Digi ConnectEZ8W'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ8W',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez_1046',
        'hardware_wireless_ax210',
        'config_cellular',
        'config_nemo',
        'config_wireless',
        'config_restrict_5g_channels_ap_only',
        'config_connectez8',
        'config_cellular',
        'config_nemo',
        'config_bonding',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        'exfat',
        'storage',
        'config_nfs_client',
        ],
    linux = dict(
    ),
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ 8 Wi-Fi',
        PROP_USBLEDD='y',
    )
    )

products['Digi ConnectEZ8M'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ8M',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez_1046',
        'config_cellular',
        'config_nemo',
        'config_connectez8',
        'config_cellular',
        'config_bonding',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        'exfat',
        'storage',
        'config_nfs_client',
        ],
    linux = dict(
    ),
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ 8 MEI',
        PROP_USBLEDD='y',
    )
    )

products['Digi ConnectEZ8IO'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ8IO',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez_1046',
        'config_cellular',
        'config_nemo',
        'config_connectez8',
        'config_cellular',
        'config_nemo',
        'config_bonding',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        'exfat',
        'storage',
        'config_nfs_client',
        ],
    linux = dict(
        IIO='y',
        TI_ADC081C='y',
        GPIO_FABRICATED='y',
        REGULATOR='y',
        REGULATOR_FIXED_VOLTAGE='y',
    ),
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ 8 IO',
        PROP_CONFIG_IOD='y',
        PROP_USBLEDD='y',
    )
    )

products['Digi ConnectEZ8IOS'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ8IOS',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez_1046',
        'config_cellular',
        'config_nemo',
        'config_connectez8',
        'config_cellular',
        'config_bonding',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        'exfat',
        'storage',
        'config_nfs_client',
        ],
    linux = dict(
        NET_DSA='y',
        NET_DSA_TAG_RTL4_A='y',
        NET_DSA_MICROCHIP_KSZ9477='y',
        NET_DSA_MICROCHIP_KSZ9477_SPI='y',
        SPI_FSL_DSPI='y',
        IIO='y',
        TI_ADC081C='y',
        GPIO_FABRICATED='y',
        REGULATOR='y',
        REGULATOR_FIXED_VOLTAGE='y',
    ),
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ 8 IO SW',
        PROP_CONFIG_IOD='y',
        PROP_USBLEDD='y',
    )
    )

products['Digi ConnectEZ16'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ16',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez_1046',
        'config_cellular',
        'config_nemo',
        'config_connectez',
        'config_cellular',
        'config_nemo',
        'config_bonding',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        'exfat',
        'storage',
        'config_nfs_client',
        ],
    linux = dict(
    ),
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ 16',
        PROP_USBLEDD='y',
    )
    )

products['Digi ConnectEZ16M'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ16M',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez_1046',
        'config_cellular',
        'config_nemo',
        'config_connectez',
        'config_cellular',
        'config_nemo',
        'config_bonding',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        'exfat',
        'storage',
        'config_nfs_client',
        ],
    linux = dict(
    ),
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ 16 MEI',
        PROP_USBLEDD='y',
    )
    )

products['Digi ConnectEZ32'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ32',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez_1046',
        'config_cellular',
        'config_nemo',
        'config_connectez',
        'config_cellular',
        'config_nemo',
        'config_bonding',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        'exfat',
        'storage',
        'config_nfs_client',
        ],
    linux = dict(
    ),
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ 32',
        PROP_USBLEDD='y',
    )
    )

products['Digi ConnectEZ32M'] = dict(
    vendor = 'Digi',
    product = 'ConnectEZ32M',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectez_1046',
        'config_cellular',
        'config_nemo',
        'config_connectez',
        'config_cellular',
        'config_nemo',
        'config_bonding',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        'exfat',
        'storage',
        'config_nfs_client',
        ],
    linux = dict(
    ),
    user = dict(
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi Connect EZ 32 MEI',
        PROP_USBLEDD='y',
    )
    )

products['Digi Connect IT 4'] = dict(
    vendor = 'Digi',
    product = 'ConnectIT4',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_connectit4',
        'config_connectit',
        'strongswan_netkey',
        'linux_crypto_arm',
        'config_stdcpp',
        'config_python',
        'config_ledcmd_wrapper',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        ],
    user = dict(
        PROP_CONFIG_SERIALD_AUTODETECT='y',
        PROP_CONFIG_SERIALD_DATAMATCH='y',
        PROP_CONFIG_EVENTD_SMTP='y',
        PROP_CONFIG_EVENTD_SNMP='y',
        USER_NETSNMP_APPS='y',
        USER_NETSNMP_APPS_TRAP='y',
        )
    )

products['Digi Connect IT 16'] = dict(
    vendor = 'Digi',
    product = 'ConnectIT16',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectit16',
        'config_connectit16',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'linux_i2c_nexcom',
        'config_stdcpp',
        'exfat',
        'storage',
        ]
    )

products['Digi Connect IT 48'] = dict(
    vendor = 'Digi',
    product = 'ConnectIT48',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_connectit48',
        'config_connectit48',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'linux_i2c_nexcom',
        'config_stdcpp',
        'exfat',
        'storage',
        ]
    )

products['VirtualDAL'] = dict(
    vendor = 'Digi',
    product = 'VirtualDAL',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_virtualdal',
        'config_virtualdal',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'config_stdcpp',
        ]
    )

products['VirtualDAL-PR'] = dict(
    vendor = 'Digi',
    product = 'VirtualDAL-PR',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_virtualdal',
        'config_virtualdal',
        'config_primary_responder',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'config_stdcpp',
        ]
    )

products['AC 6300-CX'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6300-CX',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_6300_cx',
        'config_6300_cx',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        ]
    )

products['AC 6310-DX'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6310-DX',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_6310_dx',
        'config_6310_dx',
        'strongswan_netkey',
        'linux_crypto_arm',
        ]
    )

products['Digi IX14'] = dict(
    vendor = 'Digi',
    product = 'IX14',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix14',
        'config_ix14',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_arm',
        'config_stdcpp',
        ]
    )

products['Digi EX15'] = dict(
    vendor = 'Digi',
    product = 'EX15',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_ex15',
        'config_ex15',
        'config_all_firmware',
        'strongswan_netkey',
        ]
    )

groups['config_ex15_bloader'] = dict(
    user = dict(
        PROP_BOOTLOADER_UPDATE='y',
        PROP_BOOTLOADER_UPDATE_IMAGE="http://eng.digi.com/builds/DAL/release/21.2.39.67/Digi/EX15-PR/EX15-PR-21.2.39.67-202102280012-bloader-signed.bin",
        PROP_BOOTLOADER_UPDATE_IMAGE_HASH="065df395a3ccafc4a2e33a85c9750a8714fc99194cf25a9ca06f718179906136",
    )
)

products['Digi EX15-PR'] = dict(
    vendor = 'Digi',
    product = 'EX15-PR',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_ex15',
        'config_ex15',
        'config_all_firmware',
        'strongswan_netkey',
        'config_primary_responder',
        'config_ex15_bloader',
        ],
    )

products['Digi EX15W'] = dict(
    vendor = 'Digi',
    product = 'EX15W',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_ex15w',
        'config_ex15w',
        'strongswan_netkey',
        ]
    )

products['Digi EX15W-PR'] = dict(
    vendor = 'Digi',
    product = 'EX15W-PR',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_ex15w',
        'config_ex15w',
        'strongswan_netkey',
        'config_primary_responder',
        'config_ex15_bloader',
        ],
    )

products['Digi LR54'] = dict(
    vendor = 'Digi',
    product = 'LR54',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_lr54',
        'config_lr54',
        'config_all_firmware',
        'strongswan_netkey',
        ],
    user = dict(
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=0,
        PROP_DIGI_RESET_LEDCMD_LEDS="WWAN1_SIGNAL_GREEN WWAN1_SIGNAL_YELLOW WWAN1_SERVICE_GREEN WWAN1_SERVICE_YELLOW SIM1 SIM2",
        )
    )

products['Digi LR54W'] = dict(
    vendor = 'Digi',
    product = 'LR54W',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_lr54',
        'hardware_wireless_mediatek',
        'config_lr54',
        'config_wireless',
        'config_restrict_5g_channels_ap_only',
        'config_all_firmware',
        'strongswan_netkey',
        'config_hotspot',
        'config_aircrack_ng',
        ],
    user = dict(
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=2,
        PROP_LR54_MT76_CALDATA='y',
        PROP_DIGI_RESET_LEDCMD_LEDS="WIFI1 WIFI2 WWAN1_SIGNAL_GREEN WWAN1_SIGNAL_YELLOW WWAN1_SERVICE_GREEN WWAN1_SERVICE_YELLOW SIM1 SIM2",
        )
    )

products['Digi LR54 Migration'] = dict(
    vendor = 'Digi',
    product = 'LR54-Migration',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_lr54_minimal',
        'config_security_dac_y',
        'user_minimal',
        'config_rtc',
        'config_python',
        'config_ledcmd_wrapper',
        ],
    linux = dict(
        DTB_MT7621_LR54_MIGRATION='y',
        INITRAMFS_SOURCE='../initramfs ../vendors/Digi/LR54-Migration/romfs.dev',
        SQUASHFS_XATTR='y',
        NET='y',
        UNIX='y',
        ),
    user = dict(
        USER_UBOOT_ENVTOOLS='y',
        PROP_LEDCMD_SYSFS='y',
        USER_UTIL_LINUX_SWITCH_ROOT='y',
        USER_OPENSSL_APPS='y',
        USER_SSH_SSHKEYGEN='y',
        )
    )

products['Digi TX54 Dual Cellular'] = dict(
    vendor = 'Digi',
    product = 'TX54-Dual-Cellular',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx54',
        'config_tx54',
        'config_realport_tx54',
        ],
    user = dict(
        PROP_5G_SUPPORT="y",
        )
    )

products['Digi TX54 Dual Cellular PR'] = dict(
    vendor = 'Digi',
    product = 'TX54-Dual-Cellular-PR',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx54',
        'config_primary_responder',
        'config_tx54',
        ],
    user = dict(
        PROP_BOOTLOADER_UPDATE='y',
        PROP_BOOTLOADER_UPDATE_IMAGE="http://eng.digi.com/builds/DAL/release/20.11.32.77/Digi/TX54-Dual-Cellular-PR/TX54-Dual-Cellular-PR-20.11.32.77-202011101934-bloader-signed.bin",
        PROP_BOOTLOADER_UPDATE_IMAGE_HASH="7168509934789b930fc5eb40a445497f800c8b31c95120284c4c35e7457a479f",
        PROP_5G_SUPPORT="y",
        )
    )

products['Digi TX54 Dual Wi-Fi'] = dict(
    vendor = 'Digi',
    product = 'TX54-Dual-Wi-Fi',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx54',
        'config_tx54',
        'config_realport_tx54',
        ],
    user = dict(
        PROP_5G_SUPPORT="y",
        )
    )

products['Digi TX54 Single Cellular'] = dict(
    vendor = 'Digi',
    product = 'TX54-Single-Cellular',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx54',
        'config_tx54',
        'config_realport_tx54',
        ],
    user = dict(
        PROP_5G_SUPPORT="y",
        )
    )

products['Digi TX54 Single Cellular PR'] = dict(
    vendor = 'Digi',
    product = 'TX54-Single-Cellular-PR',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx54',
        'config_primary_responder',
        'config_tx54',
        ],
    user = dict(
        PROP_BOOTLOADER_UPDATE='y',
        PROP_BOOTLOADER_UPDATE_IMAGE="http://eng.digi.com/builds/DAL/release/20.11.32.77/Digi/TX54-Single-Cellular-PR/TX54-Single-Cellular-PR-20.11.32.77-202011101934-bloader-signed.bin",
        PROP_BOOTLOADER_UPDATE_IMAGE_HASH="eec6173c29629fbb6cf192eb0907a6730cac3977b1697d157c3daa14f9fbce91",
        PROP_5G_SUPPORT="y",
        )
    )

products['Digi TX54 Migration'] = dict(
    vendor = 'Digi',
    product = 'TX54-Migration',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx54_minimal',
        'config_security_dac_y',
        'user_minimal',
        'config_rtc',
        'config_python',
        'config_ledcmd_wrapper',
        ],
    linux = dict(
        DTB_MT7621_TX54_MIGRATION='y',
        INITRAMFS_SOURCE='../initramfs ../vendors/Digi/TX54-Migration/romfs.dev',
        SQUASHFS_XATTR='y',
        NET='y',
        UNIX='y',
        ),
    user = dict(
        USER_UBOOT_ENVTOOLS='y',
        USER_UTIL_LINUX_SWITCH_ROOT='y',
        PROP_LEDCMD_SYSFS='y',
        USER_OPENSSL_APPS='y',
        USER_SSH_SSHKEYGEN='y',
        )
    )

products['Digi TX54 Manuf Revert'] = dict(
    vendor = 'Digi',
    product = 'TX54-Manuf-Revert',
    arch = 'mips',
    tools = 'mips-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx54_minimal',
        'config_security_dac_y',
        'user_minimal',
        ],
    linux = dict(
        DTB_MT7621_TX54='y',
        ),
    user = dict(
        PROP_LEDCMD_SYSFS='y',
        USER_UBOOT_ENVTOOLS='y',
        )
    )

products['Digi TX64'] = dict(
    vendor = 'Digi',
    product = 'TX64',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx64',
        'hardware_wireless_ath11k',
        'config_tx64',
        'config_all_firmware',
        'busybox_big',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'config_stdcpp',
        ],
    user = dict(
        PROP_SIERRA_SDK='y',
        PROP_CONFIG_WPA3_ENTERPRISE="y",
        PROP_5G_SUPPORT="y",
        )
    )

products['Digi TX64-PR'] = dict(
    vendor = 'Digi',
    product = 'TX64-PR',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx64',
        'hardware_wireless_ath11k',
        'config_tx64',
        'config_all_firmware',
        'busybox_big',
        'config_primary_responder',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'config_stdcpp',
        ],
    user = dict(
        PROP_BOOTLOADER_UPDATE='y',
        PROP_BOOTLOADER_UPDATE_IMAGE="http://eng.digi.com/builds/DAL/candidate/22.2/22.2.9.2/Digi/TX64-PR/TX64-PR-22.2.9.2-202202012155-bloader-signed.bin",
        PROP_BOOTLOADER_UPDATE_IMAGE_HASH="8ae433884b4a70d22100fdf7ab799d8e4093a9dbae5866fe766781cde45ef672",
        PROP_SIERRA_SDK='y',
        PROP_CONFIG_WPA3_ENTERPRISE='y',
        PROP_5G_SUPPORT="y",
        )
    )

products['Digi TX64-Rail-Single-Cellular'] = dict(
    vendor = 'Digi',
    product = 'TX64-Rail-Single-Cellular',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx64',
        'hardware_wireless_ath11k',
        'config_tx64',
        'config_all_firmware',
        'busybox_big',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'config_stdcpp',
    ],
    user = dict(
        PROP_SIERRA_SDK='y',
        PROP_5G_SUPPORT="y",
    )
)

products['Digi TX64-Rail-Single-Cellular-PR'] = dict(
    vendor = 'Digi',
    product = 'TX64-Rail-Single-Cellular-PR',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx64',
        'hardware_wireless_ath11k',
        'config_tx64',
        'config_all_firmware',
        'busybox_big',
        'config_primary_responder',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'config_stdcpp',
    ],
    user = dict(
        PROP_SIERRA_SDK='y',
        PROP_5G_SUPPORT="y",
        PROP_BOOTLOADER_UPDATE='y',
        PROP_BOOTLOADER_UPDATE_IMAGE="http://eng.digi.com/builds/DAL/candidate/22.2/22.2.9.2/Digi/TX64-Rail-Single-Cellular-PR/TX64-Rail-Single-Cellular-PR-22.2.9.2-202202012155-bloader-signed.bin",
        PROP_BOOTLOADER_UPDATE_IMAGE_HASH="86b8c5561b6fd06e722dd38aacdbcdeebfedebb41345e692da426892f32188bf",
    )
)

products['Digi TX64 Migration'] = dict(
    vendor = 'Digi',
    product = 'TX64-Migration',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_tx64_minimal',
        'config_security_dac_y',
        'user_minimal',
        'config_rtc',
        'config_python',
        'config_ledcmd_wrapper',
        ],
    linux = dict(
        BLK_DEV_INITRD='y',
        BLK_DEV='y',
        BLK_DEV_LOOP='y',
        MISC_FILESYSTEMS='y',
        SQUASHFS='y',
        SQUASHFS_XZ='y',
        SQUASHFS_XATTR='y',
        RD_LZMA='y',
        INITRAMFS_SOURCE='../romfs ../vendors/Digi/TX64-Migration/romfs.dev',
        CMDLINE_BOOL='y',
        CMDLINE='console=ttyS0,115200n8',
        CMDLINE_OVERRIDE='y',
        NLS_CODEPAGE_437='y',
        NLS_ISO8859_1='y',
        TMPFS='y',
        CRYPTO_AES='y',
        NET='y',
        UNIX='y',
        ),
    user = dict(
        USER_BUSYBOX_MKFS_VFAT='y',
        USER_OPENSSL_APPS='y',
        USER_SSH_SSHKEYGEN='y',
        USER_DMIDECODE='y',
        USER_KMOD='y',
        USER_KMOD_LIBKMOD='y',
        USER_KMOD_TOOLS='y',
        ),
    )

products['Digi AnywhereUSB2i'] = dict(
    vendor = 'Digi',
    product = 'AnywhereUSB2i',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_layerscape_1012',
        'hardware_layerscape_usb',
        'hardware_nor',
        'hardware_mmc',
        'linux_pci_layerscape',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        'config_anywhereusb2i',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        ],
    linux = dict(
        OVERLAY_FS='n',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='n',
        USB_AWUSB_NUM_PORTS=2,
        ),
    user = dict(
        PROP_DIGI_RESET_NUM_LEDS=2,
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_LSPCI='y',
        PROP_CONFIG_CC_ACL_DEVTYPE='Digi AnywhereUSB 2 Industrial'
        )
    )

products['Digi AnywhereUSB8'] = dict(
    vendor = 'Digi',
    product = 'AnywhereUSB8',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_layerscape_1046',
        'hardware_layerscape_usb',
        'hardware_cellular_cm',
        'hardware_nor',
        'hardware_mmc',
        'linux_pci_layerscape',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        'config_anywhereusb8',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        ],
    linux = dict(
        OVERLAY_FS='n',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='n',
        USB_AWUSB_NUM_PORTS=8,
        PCIEASPM_PERFORMANCE='y',
        ),
    user = dict(
        PROP_DIGI_RESET_NUM_LEDS=8,
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_LSPCI='y',
        )
    )

products['Digi AnywhereUSB8W'] = dict(
    vendor = 'Digi',
    product = 'AnywhereUSB8W',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_layerscape_1046',
        'hardware_layerscape_usb',
        'hardware_cellular_cm',
        'hardware_nor',
        'hardware_mmc',
        'hardware_wireless_ath10k',
        'linux_pci_layerscape',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        'config_anywhereusb8',
        'config_all_firmware',
        'config_stdcpp',
        'config_wireless',
        'config_restrict_5g_channels_ap_only',
        'busybox_big',
        'strongswan_netkey',
        ],
    linux = dict(
        OVERLAY_FS='n',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='n',
        USB_AWUSB_NUM_PORTS=8,
        PCIEASPM_PERFORMANCE='y',
        ),
    user = dict(
        PROP_DIGI_RESET_NUM_LEDS=8,
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_LSPCI='y',
        )
    )

products['Digi AnywhereUSB24'] = dict(
    vendor = 'Digi',
    product = 'AnywhereUSB24',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_layerscape_1046',
        'hardware_layerscape_usb',
        'hardware_cellular_cm',
        'hardware_nor',
        'hardware_mmc',
        'linux_pci_layerscape',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        'config_anywhereusb24',
        'config_all_firmware',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        ],
    linux = dict(
        OVERLAY_FS='n',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='n',
        USB_AWUSB_NUM_PORTS=24,
        PCIEASPM_PERFORMANCE='y',
        ),
    user = dict(
        PROP_DIGI_RESET_NUM_LEDS=24,
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_LSPCI='y',
        )
    )

products['Digi AnywhereUSB24W'] = dict(
    vendor = 'Digi',
    product = 'AnywhereUSB24W',
    arch = 'arm64',
    tools = 'aarch64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_layerscape_1046',
        'hardware_layerscape_usb',
        'hardware_cellular_cm',
        'hardware_nor',
        'hardware_mmc',
        'hardware_wireless_ath10k',
        'linux_pci_layerscape',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        'config_anywhereusb24',
        'config_all_firmware',
        'config_stdcpp',
        'config_wireless',
        'config_restrict_5g_channels_ap_only',
        'busybox_big',
        'strongswan_netkey',
        ],
    linux = dict(
        OVERLAY_FS='n',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='n',
        USB_AWUSB_NUM_PORTS=24,
        PCIEASPM_PERFORMANCE='y',
        ),
    user = dict(
        PROP_DIGI_RESET_NUM_LEDS=24,
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_LSPCI='y',
        )
    )

products['AC 6330-MX'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6330-MX',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_6330_mx',
        'config_6330_mx',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        ]
    )

products['AC 6335-MX'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6335-MX',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_6335_mx',
        'config_6335_mx',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        ]
    )

products['AC 6350-SR'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6350-SR',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_6350_sr',
        'config_6350_sr',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        ]
    )

products['AC 6355-SR'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6355-SR',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_6355_sr',
        'config_6355_sr',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        ]
    )

products['AC 8300'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '8300',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_8300',
        'config_8300',
        'config_all_firmware',
        'snort',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'config_stdcpp',
        'config_security_dac_y',
        ]
    )

products['AC 9400-UA'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '9400-UA',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_9400_ua',
        'config_9400_ua',
        'config_all_firmware',
        'busybox_big',
        'e2fsprogs_all',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'config_security_dac_y',
        'config_stdcpp',
        ],
    user = dict(
        USER_UTIL_LINUX_UUIDD='y',
        )
    )

products['AC Factory8300'] = dict(
    vendor = 'AcceleratedConcepts',
    product = 'Factory8300',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_factory8300',
        'config_factory',
        'config_all_firmware',
        'config_security_dac_y',
        'config_stdcpp',
        ]
    )

products['AC FactoryU115'] = dict(
    vendor = 'AcceleratedConcepts',
    product = 'FactoryU115',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_factoryU115',
        'config_factory',
        'config_security_dac_y',
        'config_stdcpp',
        ]
    )

products['AC Sprite'] = dict(
    vendor = 'AcceleratedConcepts',
    product = 'sprite',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_sprite',
        'config_sprite',
        'linux_crypto',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'config_security_dac_y',
        ]
    )

products['AC U115'] = dict(
    vendor = 'AcceleratedConcepts',
    product = 'U115',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_u115',
        'config_u115',
        'config_base_features',
        'linux_crypto',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'busybox_big',
        'strongswan_netkey',
        'config_stdcpp',
        'config_custdefcfg',
        'gnss_modem',
        ]
    )

products['AC Porter'] = dict(
    vendor = 'AcceleratedConcepts',
    product = 'porter',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20220202',
    libc = 'musl',
    include = [
        'hardware_porter',
        'config_sprite',
        'e2fsprogs_ext2',
        'linux_crypto',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'config_security_dac_y',
        ]
    )

products['ATT U115'] = dict(
    vendor = 'ATT',
    product = 'U115',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_att_u115',
        'config_u115',
        'linux_crypto',
        'linux_ocf_cryptosoft',
        'linux_ocf_mv_cesa',
        'linux_crypto_arm',
        'linux_ocf_mv_cesa',
        'linux_crypto_smp',
        'openswan_klips_ocf',
        'busybox_big',
        'config_stdcpp',
        'config_security_dac_y',
        ],
    user = dict(
        PROP_LOGD_LOGD='y',
        PROP_LOGD_LOGINIT='y',
        USER_NETFLASH_CRYPTO_OPTIONAL='y',
        USER_INIT_DEFAULT_ENTRIES='y',
        USER_INIT_CONSOLE_SH='y',
        USER_INIT_RUN_CONFIGRC='y',
        LIB_LIBUNBOUND='y',
        LIB_LIBLDNS='y',
        )
    )

import product_opengear as _opengear
products.update(_opengear.products)
groups.update(_opengear.groups)

for name, group in groups.items():
    group['name'] = name

for name, product in products.items():
    product['name'] = name

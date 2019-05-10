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
        NLS='y',
        NLS_DEFAULT='iso8859-1',
        NLS_CODEPAGE_437='y',
        NLS_ISO8859_1='y',
        POSIX_TIMERS='y',
        COREDUMP_PRINTK='y',
        CONSOLE_LOGLEVEL_DEFAULT=7,
        MESSAGE_LOGLEVEL_DEFAULT=4,
        PRINTK_SAFE_LOG_BUF_SHIFT='13',
        FRAME_WARN=1024,
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
        CRYPTO_ARC4='y',
        CRYPTO_DEFLATE='y',
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
        CRYPTO_SHA1_ARM64_CE='y',
        CRYPTO_SHA2_ARM64_CE='y',
        CRYPTO_SHA512_ARM64_CE='y',
        CRYPTO_SHA3_ARM64='y',
        CRYPTO_SM3_ARM64_CE='y',
        CRYPTO_SM4_ARM64_CE='y',
        CRYPTO_GHASH_ARM64_CE='y',
        CRYPTO_CRCT10DIF_ARM64_CE='y',
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
        QCA_CLD_WLAN='m',
        )
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
        NF_NAT_IPV4='m',
        NF_NAT_IPV6='m',
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
        NF_NAT_IPV4='m',
        NF_NAT_IPV6='n',
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
        NF_NAT_MASQUERADE_IPV6='y',
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
        NET_KEY='y',
        INET_AH='y',
        INET_ESP='y',
        INET_IPCOMP='y',
        INET_XFRM_MODE_TRANSPORT='y',
        INET_XFRM_MODE_TUNNEL='y',
        NETFILTER_XT_MATCH_POLICY='m',
        )
    )

groups['linux_netkey_ipv6'] = dict(
    linux = dict(
        INET6_AH='y',
        INET6_ESP='y',
        INET6_IPCOMP='y',
        INET6_XFRM_MODE_TRANSPORT='y',
        INET6_XFRM_MODE_TUNNEL='y',
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
        PCI_MT7621='y',
        PCI_MSI='y',
        PCI_QUIRKS='y',
        PCIEPORTBUS='y',
        PCIEAER='y',
        PCIEASPM='y',
        PCIEASPM_DEFAULT='y',
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
        USB_SERIAL_FTDI_SIO='y',
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
        REFCOUNT_FULL='y',
        STACKPROTECTOR='y',
        ),
    user = dict(
        LIB_INSTALL_LIBGCC_S='y'
        ),
    )

groups['linux_arm64'] = dict(
    linux = dict(
        MMU='y',
        VMAP_STACK='y',
        SPARSEMEM_VMEMMAP='y',
        UNMAP_KERNEL_AT_EL0='y',
        PINCTRL='y',
        OF='y',
        DEBUG_BUGVERBOSE='y',
        RSEQ='y',
        REFCOUNT_FULL='y',
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
        )
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
        LBDAF='y',
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
        MTD_NAND='y',
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
        SNAPDOG='y',
        SERIAL_8250='y',
        SERIAL_8250_DEPRECATED_OPTIONS='y',
        SERIAL_8250_CONSOLE='y',
        SERIAL_8250_DMA='y',
        SERIAL_8250_NR_UARTS='4',
        SERIAL_8250_RUNTIME_UARTS='4',
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
        ENABLE_MUST_CHECK='y',
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
    user = dict(
        BOOT_UBOOT_MARVELL_370='y'
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
        SYSCTL_SYSCALL='y',
        KALLSYMS='y',
        ELF_CORE='y',
        CORE_DUMP_DEFAULT_ELF_HEADERS='y',
        AIO='y',
        VM_EVENT_COUNTERS='y',
        KUSER_HELPERS='y',
        MODULE_FORCE_UNLOAD='y',
        FW_LOADER='y',
        EXTRA_FIRMWARE='',
        LBDAF='y',
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
        MTD_NAND='y',
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
        USB_EHCI_MXC='y',
        USB_CHIPIDEA='y',
        USB_CHIPIDEA_HOST='y',
        USB_MXS_PHY='y',
        RESET_CONTROLLER='y',
        EXTCON='y',
        USB_EHCI_HCD_PLATFORM='y',
        SERIAL_IMX='y',
        SERIAL_IMX_CONSOLE='y',
        DEBUG_ALIGN_RODATA='y',
        NOP_USB_XCEIV='y',
        NEW_LEDS='y',
        LEDS_CLASS='y',
        LEDS_TRIGGERS='y',
        DNOTIFY='y',
        ENABLE_MUST_CHECK='y',
        CRYPTO_SHA512='y',
        CRYPTO_ANSI_CPRNG='m',
        CRYPTO_CCM='y',
        CRYPTO_GCM='y',
        XZ_DEC_ARM='y',
        XZ_DEC_ARMTHUMB='y',
        HW_RANDOM_IMX_RNGC='y',
        MFD_SYSCON='y',
        SRAM='y',
        WATCHDOG='y',
        IMX2_WDT='y',
        HW_RANDOM='y',
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
        ],
    linux = dict(
        ARM64_HW_AFDBM='y',
        ARM64_PAN='y',
        ARM64_LSE_ATOMICS='y',
        ARM64_VHE='y',
        ARM64_UAO='y',
        ARM64_RAS_EXTN='y',
        ARM64_CNP='y',
        ARM64_PTR_AUTH='y',
        ARM64_SVE='y',
        ARM64_SSBD='y',
        ARM64_ERRATUM_826319='y',
        ARM64_ERRATUM_827319='y',
        ARM64_ERRATUM_824069='y',
        ARM64_ERRATUM_819472='y',
        ARM64_ERRATUM_832075='y',
        ARM64_ERRATUM_843419='y',
        ARM64_ERRATUM_858921='y',
        ARM64_ERRATUM_1024718='y',
        ARM64_ERRATUM_1188873='y',
        ARM64_ERRATUM_1165522='y',
        ARM64_ERRATUM_1286807='y',
        HARDEN_BRANCH_PREDICTOR='y',
        HARDEN_EL2_VECTORS='y',
        RODATA_FULL_DEFAULT_ENABLED='y',
        HWSPINLOCK='y',
        SLUB_CPU_PARTIAL='y',
        CMA='y',
        DMA_CMA='y',
        CMA_SIZE_MBYTES='32',
        HZ_PERIODIC='y',
        HIGH_RES_TIMERS='y',
        MODULE_FORCE_UNLOAD='y',
        REGULATOR='y',
        REGULATOR_FIXED_VOLTAGE='y',
        ARM_ARCH_TIMER_EVTSTREAM='y',
        HISILICON_ERRATUM_161010101='y',
        ARM_TIMER_SP804='y',
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
        RTC_CLASS='y',
        RTC_HCTOSYS='y',
        RTC_SYSTOHC='y',
        RTC_HCTOSYS_DEVICE='rtc0',
        RTC_SYSTOHC_DEVICE='rtc0',
        RTC_INTF_SYSFS='y',
        RTC_INTF_PROC='y',
        RTC_INTF_DEV='y',
        SATA_AHCI='y',
        SATA_MOBILE_LPM_POLICY='0',
        POWER_SUPPLY='y',
        HWMON='y',
        NEW_LEDS='y',
        LEDS_CLASS='y',
        LEDS_TRIGGERS='y',
        LEDS_TRIGGER_DISK='y',
        LEDS_TRIGGER_HEARTBEAT='y',
        LEDS_TRIGGER_CPU='y',
        LEDS_TRIGGER_DEFAULT_ON='y',
        LEDS_TRIGGER_PANIC='y',
        LEDS_SYSCON='y',
        IOMMU_SUPPORT='y',
        PPS='y',
        DNOTIFY='y',
        ENABLE_MUST_CHECK='y',
        DEBUG_MEMORY_INIT='y',
        MSDOS_PARTITION='y',
        EFI_PARTITION='y',
        TEE='y',
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

groups['hardware_layerscape_1046'] = dict(
    include = [
        'hardware_arm64',
        ],
    linux = dict(
        ARCH_LAYERSCAPE='y',
        FSL_ERRATUM_A008585='y',
        CLK_QORIQ='y',
        PINCTRL_SINGLE='y',
        SERIAL_8250='y',
        SERIAL_8250_DEPRECATED_OPTIONS='y',
        SERIAL_8250_CONSOLE='y',
        SERIAL_OF_PLATFORM='y',
        AHCI_QORIQ='y',
        SPI='y',
        SPI_FSL_SPI='y',
        SPI_FSL_DSPI='y',
        SPI_FSL_QUADSPI='y',
        I2C='y',
        I2C_IMX='y',
        I2C_COMPAT='y',
        I2C_CHARDEV='y',
        I2C_MUX='y',
        I2C_HELPER_AUTO='y',
        I2C_DESIGNWARE_PLATFORM='y',
        NET_VENDOR_FREESCALE='y',
        FSL_FMAN='y',
        FSL_XGMAC_MDIO='y',
        FSL_DPAA='y',
        FSL_DPAA_ETH='y',
        FSL_MC_BUS='y',
        USB_PCI='y',
        USB_STORAGE='y',
        USB_XHCI_HCD='y',
        USB_EHCI_HCD_PLATFORM='y',
        USB_OHCI_HCD='y',
        USB_OHCI_HCD_PCI='y',
        USB_OHCI_HCD_PLATFORM='y',
        USB_MUSB_HDRC='y',
        USB_DWC3='y',
        USB_DWC3_HAPS='y',
        USB_DWC3_OF_SIMPLE='y',
        USB_DWC2='y',
        USB_CHIPIDEA='y',
        USB_CHIPIDEA_HOST='y',
        USB_ISP1760='y',
        NOP_USB_XCEIV='y',
        USB_ULPI='y',
        PWRSEQ_EMMC='y',
        PWRSEQ_SIMPLE='y',
        MTD_M25P80='y',
        MTD_OF_PARTS='y',
        MTD_SPI_NOR='y',
        MTD_SPI_NOR_USE_4K_SECTORS='y',
        BLK_DEV_RAM_SIZE=32768,
        SECCOMP='y',
        EXT4_FS='y',
        EXT4_USE_FOR_EXT2='y',
        VFAT_FS='y',
        CONFIGFS_FS='y',
        ),
    user = dict(
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
       )
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

groups['linux_x86'] = dict(
    include = [
        'linux_smp',
         ],
    linux = dict(
        IA32_EMULATION='y',
        KERNEL_XZ='y',
        LOG_BUF_SHIFT='17',
        LOG_CPU_MAX_BUF_SHIFT='12',
        ELF_CORE='y',
        CORE_DUMP_DEFAULT_ELF_HEADERS='y',
        DEBUG_BUGVERBOSE='y',
        KALLSYMS='y',
        SYSVIPC='y',
        POSIX_MQUEUE='y',
        SGETMASK_SYSCALL='y',
        SYSFS_SYSCALL='y',
        SYSCTL_SYSCALL='y',
        KALLSYMS_ALL='y',
        BPF_SYSCALL='y',
        FHANDLE='y',
        USELIB='y',
        FUTEX='y',
        AIO='y',
        SFI='y',
        DMIID='y',
        PERF_EVENTS='y',
        VM_EVENT_COUNTERS='y',
        COMPAT_BRK='y',
        RD_BZIP2='y',
        RD_LZMA='y',
        RD_XZ='y',
        RD_LZO='y',
        RD_LZ4='y',
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
        STACK_VALIDATION='y',
        SOFTLOCKUP_DETECTOR='y',
        VMAP_STACK='y',
        JUMP_LABEL='y',
        UID16='y',
        RSEQ='y',
        ),
    user = dict(
        LIB_INSTALL_LIBGCC_S='y'
        ),
    )

groups['hardware_x86'] = dict(
    include = [
        'linux_64bit',
        'linux_pci_x86',
        ],
    linux = dict(
        HZ_PERIODIC='y',
        HIGH_RES_TIMERS='y',
        MODULE_FORCE_UNLOAD='y',
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
        PROCESSOR_SELECT='y',
        CPU_SUP_INTEL='y',
        CPU_SUP_AMD='y',
        CPU_SUP_CENTAUR='y',
        CPU_IDLE='y',
        DMI='y',
        SCHED_MC='y',
        X86_VSYSCALL_EMULATION='y',
        X86_MSR='y',
        X86_CPUID='y',
        SPARSEMEM_MANUAL='y',
        SPARSEMEM_VMEMMAP='y',
        X86_RESERVE_LOW='64',
        MTRR='y',
        MTRR_SANITIZER='y',
        MTRR_SANITIZER_ENABLE_DEFAULT='0',
        MTRR_SANITIZER_SPARE_REG_NR_DEFAULT='1',
        ARCH_RANDOM='y',
        X86_SMAP='y',
        X86_INTEL_MPX='y',
        X86_PLATFORM_DEVICES='y',
        X86_FEATURE_NAMES='y',
        EFI='y',
        HZ_250='y',
        PHYSICAL_START='0x100000',
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
        RTC_CLASS='y',
        RTC_HCTOSYS='y',
        RTC_SYSTOHC='y',
        RTC_HCTOSYS_DEVICE='rtc0',
        RTC_SYSTOHC_DEVICE='rtc0',
        RTC_INTF_SYSFS='y',
        RTC_INTF_PROC='y',
        RTC_INTF_DEV='y',
        RTC_DRV_CMOS='y',
        USB_XHCI_HCD='y',
        USB_EHCI_HCD_PLATFORM='y',
        USB_OHCI_HCD='y',
        USB_OHCI_HCD_PCI='y',
        USB_UHCI_HCD='y',
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
        DOUBLEFAULT='y',
        HARDLOCKUP_DETECTOR='y',
        NEW_LEDS='y',
        LEDS_CLASS='y',
        LEDS_TRIGGERS='y',
        SSB='y',
        SSB_PCIHOST='y',
        SSB_DRIVER_PCICORE='y',
        SWCONFIG='y',
        IOMMU_SUPPORT='y',
        AMD_IOMMU='y',
        AMD_IOMMU_V2='y',
        INTEL_IOMMU='y',
        INTEL_IOMMU_DEFAULT_ON='y',
        IRQ_REMAP='y',
        PPS='y',
        PTP_1588_CLOCK='y',
        USB_STORAGE='y',
        USB_PCI='y',
        DNOTIFY='y',
        ENABLE_MUST_CHECK='y',
        DEBUG_MEMORY_INIT='y',
        CRYPTO_MANAGER_DISABLE_TESTS='y',
        CRYPTO_NULL='y',
        CRYPTO_MD4='y',
        CRYPTO_HW='y',
        CRYPTO_DEV_PADLOCK='y',
        CRYPTO_DEV_PADLOCK_AES='y',
        CRYPTO_DEV_PADLOCK_SHA='y',
        XZ_DEC_X86='y',
        CRYPTO_SHA512='y',
        CRC16='y',
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
        MEMCG_SWAP_ENABLED='y',
        )
    )

groups['linux_kvm'] = dict(
    linux = dict(
        VIRTUALIZATION='y',
        KVM='y',
        KVM_INTEL='y',
        KVM_AMD='y',
        VHOST_NET='y',
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
        USER_MTD_UTILS_UBICRC32='y',
        USER_MTD_UTILS_UBINFO='y',
        USER_MTD_UTILS_UBIATTACH='y',
        USER_MTD_UTILS_UBIDETACH='y',
        USER_MTD_UTILS_UBINIZE='y',
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
        ),
    )

groups['hardware_cellular_cm'] = dict(
    include = [
        'hardware_cellular_sierra',
        'hardware_cellular_telit',
        'custom_telit_firmware_upload',
        ],
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
        WLAN_VENDOR_ATH='y',
        ATH5K='y',
        ATH5K_PCI='y',
        ATH9K_BTCOEX_SUPPORT='y',
        ATH9K='m',
        ATH9K_PCI='y',
        ATH9K_PCOEM='y',
        ATH10K='m',
        ATH10K_PCI='m',
        ATH10K_DFS_CERTIFIED='y',
        ATH_REG_NO_FORCED_RADAR='y',
        ATH_REG_FORCE_DEFAULT='y',
        AR5523='y',
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
        )
    )

groups['wireless_ixx4'] = dict(
    include = [
        'config_wireless',
        ],
    linux = dict(
        WIRELESS='y',
        WIRELESS_EXT='y',
        WEXT_PRIV='y',
        WLAN='y',
        WLAN_VENDOR_INTERSIL='y',
        ),
    user = dict(
        USER_HOSTAPD_HOSTAPD_CLI='y',
        USER_IW='y',
        USER_WIRELESS_TOOLS='y',
        USER_WIRELESS_TOOLS_IWCONFIG='y',
        USER_WIRELESS_TOOLS_IWGETID='y',
        USER_WIRELESS_TOOLS_IWLIST='y',
        USER_WIRELESS_TOOLS_IWPRIV='y',
        USER_WIRELESS_TOOLS_IWSPY='y',
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

groups['hardware_53xx_dc'] = dict(
    include = [
        'linux_arm',
        ],
    linux = dict(
        ARM_PATCH_PHYS_VIRT='y',
        ARM_APPENDED_DTB='y',
        ARM_ATAG_DTB_COMPAT='y',
        ARM_ATAG_DTB_COMPAT_CMDLINE_FROM_BOOTLOADER='y',
        HZ_PERIODIC='y',
        LOG_BUF_SHIFT='14',
        CC_OPTIMIZE_FOR_SIZE='y',
        SYSCTL_SYSCALL='y',
        ARCH_MULTIPLATFORM='y',
        ARCH_MULTI_V5='y',
        ARCH_MXC='y',
        SOC_IMX25='y',
        IMX_IRQSTEER='y',
        HZ_100='y',
        PM='y',
        AEABI='y',
        FUTEX='y',
        CROSS_MEMORY_ATTACH='y',
        CMDLINE='console=null',
        CMDLINE_FROM_BOOTLOADER='y',
        AUTO_ZRELADDR='y',
        SUSPEND='y',
        MTD_CFI='y',
        MTD_CFI_ADV_OPTIONS='y',
        MTD_CFI_NOSWAP='y',
        MTD_CFI_GEOMETRY='y',
        MTD_MAP_BANK_WIDTH_2='y',
        MTD_CFI_I1='y',
        MTD_CFI_INTELEXT='y',
        MTD_COMPLEX_MAPPINGS='y',
        MTD_M25P80='y',
        MTD_OF_PARTS='y',
        MTD_SPI_NOR='y',
        MTD_SPI_NOR_USE_4K_SECTORS='y',
        NET_VENDOR_FREESCALE='y',
        UNCOMPRESS_INCLUDE='debug/uncompress.h',
        FEC='y',
        MICREL_PHY='y',
        PPTP='y',
        SERIAL_IMX='y',
        SERIAL_IMX_CONSOLE='y',
        SI32260='y',
        SPI='y',
        SPI_BITBANG='y',
        SPI_IMX='y',
        PPS='y',
        PTP_1588_CLOCK='y',
        WATCHDOG='y',
        SRAM='y',
        IMX2_WDT='y',
        JFFS2_FS='y',
        JFFS2_FS_DEBUG='0',
        JFFS2_FS_WRITEBUFFER='y',
        ARM_UNWIND='y',
        CRYPTO_AUTHENC='y',
        CRYPTO_MD4='y',
        CRYPTO_DES='y',
        BLK_DEV_RAM_SIZE=16384,
        ),
    user = dict(
        BOOT_UBOOT='y',
        BOOT_UBOOT_TARGET='5300-dc',
        PROP_SLIC_SLIC='y',
        LIB_UDNS='y',
        USER_HWCLOCK_HWCLOCK='y',
        USER_UBOOT_ENVTOOLS_DEFAULT_CONFIG_FILE='y',
        USER_MTD_UTILS='y',
        USER_MTD_UTILS_ERASE='y',
        USER_MTD_UTILS_LOCK='y',
        USER_MTD_UTILS_UNLOCK='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_BUSYBOX_GETTY='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_BUSYBOX_LSPCI='y',
        )
    )

groups['hardware_5300_dc'] = dict(
    include = [
        'hardware_53xx_dc',
        'linux_usb_cellular_internal',
        ],
    linux = dict(
        USB_EHCI_MXC='y',
        USB_CHIPIDEA='y',
        USB_CHIPIDEA_HOST='y',
        NOP_USB_XCEIV='y',
        RESET_CONTROLLER='y',
        EXTCON='y',
        )
    )

groups['hardware_5301_dc'] = dict(
    include = [
        'hardware_53xx_dc',
        ],
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
        SYSCTL_SYSCALL='y',
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
        MTD_NAND='y',
        MTD_NAND_ORION='y',
        MTD_NAND_ECC_BCH='y',
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
        'custom_telit_firmware_upload',
        'linux_usb_net_internal',
        'linux_usb_storage',
        'config_stdcpp',
        ],
    linux = dict(
        SNAPDOG='y',
        SNAPDOG_CONNECTITMINI='y',
        BLK_DEV_RAM_SIZE=16384,
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
        NVMEM_IMX_OCOTP='y',
        ),
    user = dict(
        BOOT_UBOOT_FREESCALE_IMX6='y',
        BOOT_UBOOT_FREESCALE_IMX6_TARGET='connectit-mini',
        USER_I2C_TOOLS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_I2C_BUS='1',
        USER_NETFLASH_ATECC508A_EMBEDDED_KERNEL='n',
        USER_SIGS_SIGS='y',
        PROP_SECUREBOOT='y',
        PROP_SECUREBOOT_PROGRAM_ECC508='y',
        PROP_SECUREBOOT_TOOLS='y',
        PROP_IMX_FUSES='y',
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
        BLK_DEV_RAM_SIZE=16384,
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
        SERIAL_8250_NR_UARTS='64',
        SERIAL_8250_RUNTIME_UARTS='64',
        SERIAL_8250_PCI='y',
        SERIAL_8250_EXAR='y',
        SERIAL_8250_EXTENDED='y',
        SERIAL_8250_MANY_PORTS='y',
        SERIAL_8250_SHARE_IRQ='y',
        INPUT_EVDEV='y',
        ),
    user = dict(
        USER_GRUB='y',
        USER_GRUB_EFI='y',
        USER_GRUB_ENVTOOLS='y',
        USER_GRUB_DISABLE_VGA_CONSOLE='y',
        USER_UBOOT_ENVTOOLS='n',
        USER_UBOOT_ENVTOOLS_ENV_OVERWRITE='n',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_NETFLASH_ATECC508A='y',
        USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT=15,
        USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT=9,
        USER_NETFLASH_ATECC508A_I2C_BUS=1,
        USER_NETFLASH_ATECC508A_ALG_ECDSA='y',
        USER_NETFLASH_ATECC508A_EMBEDDED_KERNEL='y',
        USER_I2C_TOOLS='y',
        PROP_SECUREBOOT='y',
        PROP_SECUREBOOT_PROGRAM_ECC508='y',
        PROP_SECUREBOOT_TOOLS='y',
        USER_BUSYBOX_GETTY='y',
        USER_LEDCMD_LEDCMD='n',
        USER_HWCLOCK_HWCLOCK='y',
        PROP_GPIO_NEXCOM_MCU='y',
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
        BLK_DEV_RAM_SIZE=16384,
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
        BOOT_UBOOT_MARVELL_370_TARGET='ac6300cx',
        )
    )

groups['hardware_6300_lx'] = dict(
    include = [
        'hardware_armada_370',
        'hardware_nand',
        'hardware_cellular_external',
        'linux_usb_net_external',
        ],
    linux = dict(
        MACH_6300LX='y',
        MICREL_PHY='y',
        MARVELL_PHY='y',
        REALTEK_PHY='y',
        BLK_DEV_RAM_SIZE=16384,
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
        BOOT_UBOOT_MARVELL_370_TARGET='ac6300lx',
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
        BLK_DEV_RAM_SIZE=16384,
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
        ),
    user = dict(
        BOOT_UBOOT_FREESCALE_IMX6='y',
        BOOT_UBOOT_FREESCALE_IMX6_TARGET='ac6310dx',
        )
    )

groups['hardware_ix14'] = dict(
    include = [
        'hardware_imx6',
        'hardware_nand',
        'hardware_cellular_telit',
        'custom_telit_firmware_upload',
        'linux_container',
        'hardware_bluetooth',
        'modules_cryptodev',
        #
        # Uncomment the following line to enable wireless on the IX14
        #
        # 'modules_qcacld',
        ],
    linux = dict(
        ALLOW_DEV_COREDUMP='y',
        ARM_ARCH_TIMER_EVTSTREAM='y',
        ARM_PMU='y',
        ARM_THUMB='y',
        BLK_DEV_RAM_SIZE=32768,
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
        IMX_SDMA='y',
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
        IIO_CONSUMERS_PER_TRIGGER='2',
        IIO_ST_ACCEL_3AXIS='y',
        IIO_ST_LIS2HH12='y',
        IIO='y',
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
        MANDATORY_FILE_LOCKING='y',
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
        PERF_EVENTS='y',
        PM_DEBUG='y',
        POSIX_MQUEUE='y',
        POWER_RESET_SYSCON_POWEROFF='y',
        POWER_RESET='y',
        POWER_SUPPLY='y',
        PPP_MULTILINK='y',
        PROC_EVENTS='y',
        PWM_IMX='y',
        PWM='y',
        PM='y',
        RELAY='y',
        NVMEM_IMX_OCOTP='y',
        RTC_CLASS='y',
        RTC_DRV_MXC='y',
        RTC_DRV_SNVS='y',
        RTC_HCTOSYS_DEVICE='rtc0',
        RTC_HCTOSYS='y',
        RTC_INTF_DEV_UIE_EMUL='y',
        RTC_INTF_DEV='y',
        RTC_INTF_PROC='y',
        RTC_INTF_SYSFS='y',
        RTC_NVMEM='y',
        RTC_SYSTOHC_DEVICE='rtc0',
        RTC_SYSTOHC='y',
        RT_GROUP_SCHED='y',
        RTC_DRV_MCA_CC6UL='y',
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
        BOOT_UBOOT_DIGI_VERSION='64a2930682d9bc157b6ea2708dc9ecb45f0d7fa9',
        BOOT_UBOOT_DIGI='y',
        LIB_LIBFTDI='y',
        POOR_ENTROPY='n',
        PROP_MTST='y',
        PROP_PREBUILT='y',
        PROP_PREBUILT_DIGI_MCA_TOOL='y',
        PROP_SIM_SIM='y',
        USER_HWCLOCK_HWCLOCK='y',
        USER_BUSYBOX_FEATURE_FANCY_SLEEP='y',
        USER_BUSYBOX_FEATURE_FLOAT_SLEEP='y',
        USER_BUSYBOX_FEATURE_TR_CLASSES='y',
        USER_BUSYBOX_MICROCOM='y',
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
        USER_SIGS_SIGS='y',
        )
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
        ],
    linux = dict(
        RALINK='y',
        SOC_MT7621='y',
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
        SYSCTL_SYSCALL='y',
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
        LBDAF='y',
        BLK_DEV_BSG='y',
        MSDOS_PARTITION='y',
        EFI_PARTITION='y',
        FUTEX='y',
        CROSS_MEMORY_ATTACH='y',
        RELAY='y',
        BLK_DEV_RAM_SIZE=16384,
        MTD_CMDLINE_PARTS='y',
        MTD_OF_PARTS='y',
        MTD_NAND='y',
        MTD_NAND_MT7621='y',
        MTD_UBI='y',
        MTD_UBI_WL_THRESHOLD='4096',
        MTD_UBI_BEB_LIMIT='20',
        MTD_UBI_GLUEBI='y',
        I2C_MT7621='y',
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
        EARLY_PRINTK='y',
        RTC_CLASS='y',
        RTC_HCTOSYS='y',
        RTC_SYSTOHC='y',
        RTC_HCTOSYS_DEVICE='rtc0',
        RTC_SYSTOHC_DEVICE='rtc0',
        RTC_INTF_SYSFS='y',
        RTC_INTF_PROC='y',
        RTC_INTF_DEV='y',
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
        ENABLE_MUST_CHECK='y',
        DEBUG_MEMORY_INIT='y',
        CRYPTO_SHA512='y',
        CRYPTO_ANSI_CPRNG='m',
        CRYPTO_CCM='y',
        CRYPTO_GCM='y',
        WATCHDOG='y',
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
        ),
    user = dict(
        PROP_SIM_SIM='y',
        BOOT_UBOOT='y',
        BOOT_UBOOT_TARGET='ex15',
        )
    )

groups['hardware_ex15w'] = dict(
    include = [
        'hardware_ex15',
        'hardware_wireless',
        ]
    )

groups['hardware_ex55'] = dict(
    include = [
        'hardware_mt7621',
        'hardware_cellular_cm',
        'hardware_wireless',
        ],
    linux = dict(
        DTB_MT7621_EX55='y',
        USB_XHCI_MTK='y',
        NET_DSA='y',
        NET_DSA_MT7530='y',
        SNAPDOG='y',
        ),
    user = dict(
        PROP_SIM_SIM='y',
        BOOT_UBOOT='y',
        BOOT_UBOOT_TARGET='ex55',
        )
    )

groups['hardware_tx54'] = dict(
    include = [
        'hardware_mt7621',
        'hardware_cellular_cm',
        'hardware_wireless',
        ],
    linux = dict(
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
        INPUT='y',
        INPUT_LEDS='y',
        INPUT_MISC='y',
        INPUT_ADXL34X='y',
        INPUT_ADXL34X_I2C='y',
        ),
    user = dict(
        USER_BUSYBOX_HWCLOCK='y',
        USER_BUSYBOX_FEATURE_HWCLOCK_LONG_OPTIONS='y',
        BOOT_UBOOT='y',
        BOOT_UBOOT_TARGET='tx54',
        )
    )

groups['hardware_tx64'] = dict(
    include = [
        'hardware_x86',
        'hardware_cellular_sierra',
        'hardware_cellular_telit',
        'custom_telit_firmware_upload',
        'hardware_cellular_external',
        'hardware_wireless',
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
        BLK_DEV_RAM_SIZE=65536,
        NVRAM='y',
        I2C_ALGOBIT='y',
        I2C_I801='y',
        I2C_ISCH='y',
        I2C_ISMT='y',
        I2C_PIIX4='y',
        I2C_SCMI='y',
        I2C_DESIGNWARE_PCI='m',
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
        LPC_ICH='y',
        ITCO_WDT='y',
        INPUT_EVDEV='y',
        ),
    user = dict(
        LIB_INSTALL_LIBATOMIC='y',
        LIB_LIBPCIACCESS='y',
        LIB_LIBXML2='y',
        USER_LIBQMI_QMICLI='y',
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
        USER_NETFLASH_ATECC508A_I2C_BUS='1',
        USER_NETFLASH_ATECC508A_EMBEDDED_KERNEL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        USER_DISCARD_INETD_ECHO='y',
        USER_DISCARD_ECHO_NO_INSTALL='y',
        USER_SIGS_SIGS='y',
        USER_PROCPS_TLOAD='y',
        USER_PROCPS_VMSTAT='y',
        USER_PROCPS_W='y',
        USER_PCIUTILS='y',
        USER_ETHTOOL_ETHTOOL='y',
        USER_UBOOT_ENVTOOLS='n',
        USER_UBOOT_ENVTOOLS_ENV_OVERWRITE='n',
        USER_LM_SENSORS='y',
        USER_BUSYBOX_GETTY='y',
        USER_GRUB='y',
        USER_GRUB_EFI='y',
        USER_GRUB_ENVTOOLS='y',
        USER_GRUB_DISABLE_VGA_CONSOLE='y',
        USER_LEDCMD_LEDCMD='n',
        PROP_GPIO_NEXCOM_MCU='y',
        USER_BUSYBOX_ACPID='y',
        USER_BUSYBOX_FEATURE_ACPID_COMPAT='y',
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
        BLK_DEV_RAM_SIZE=16384,
        NET_SWITCHDEV='y',
        NET_DSA='y',
        NET_DSA_MV88E6XXX='y',
        NET_DSA_MV88E6XXX_GLOBAL2='y',
        MARVELL_PHY='y',
        FIXED_PHY='y',
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
        BOOT_UBOOT_MARVELL_370_TARGET='ac6350sr',
        )
    )

groups['hardware_6330_mx'] = dict(
    include = [
        'hardware_6335_mx',
        'hardware_wireless',
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
        BLK_DEV_RAM_SIZE=16384,
        NET_SWITCHDEV='y',
        NET_DSA='y',
        NET_DSA_MV88E6XXX='y',
        NET_DSA_MV88E6XXX_GLOBAL2='y',
        MARVELL_PHY='y',
        FIXED_PHY='y',
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
        BOOT_UBOOT_MARVELL_370_TARGET='ac6350sr',
        )
    )

groups['hardware_6350_sr'] = dict(
    include = [
        'hardware_6355_sr',
        'hardware_wireless',
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
        USER_LIBQMI_QMICLI='y',
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
        USER_UBOOT_ENVTOOLS_OPTIONS='y',
        USER_UBOOT_ENVTOOLS_OPTION_ETHADDR='00:27:04:03:02:00',
        USER_UBOOT_ENVTOOLS_OPTION_ETH1ADDR='00:27:04:03:02:01',
        USER_UBOOT_ENVTOOLS_OPTION_ETH2ADDR='00:27:04:03:02:02',
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
        BOOT_UBOOT_MARVELL_370_TARGET='ac8300',
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
        MTD_M25P80='y',
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
        LIB_LIBXML2='y',
        USER_LIBQMI_QMICLI='y',
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
        BLK_DEV_RAM_SIZE=16384,
        MARVELL_PHY='y',
        GPIO_PCF857X='y',
        I2C_MV64XXX='y',
        MACH_U115='y',
        NET_DSA='y',
        NET_DSA_MV88E6XXX='y',
        NET_DSA_MV88E6XXX_GLOBAL2='y',
        ),
    user = dict(
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        USER_ETHTOOL_ETHTOOL='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_NETFLASH_DUAL_IMAGES='y',
        USER_EMCTEST_EMCTEST='y',
        USER_BUSYBOX_LSPCI='y',
        USER_BUSYBOX_SEQ='y',
        USER_BUSYBOX_GETTY='y',
        BOOT_UBOOT_MARVELL_380_TARGET='ac_u115',
        PROP_SIM_SIM='y',
        PROP_SWTEST_MII='y',
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
        BLK_DEV_RAM_SIZE=16384,
        MARVELL_PHY='y',
        GPIO_PCF857X='y',
        I2C_MV64XXX='y',
        MACH_U115='y',
        NET_SWITCHDEV='y',
        MV88E6350_PHY='y',
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
        'hardware_armada_380',
        'hardware_nand',
        'hardware_88e6390',
        'hardware_cellular_cm',
        'hardware_cellular_external',
        'linux_i2c',
        'linux_usb_net_external',
        'linux_usb_storage',
        'busybox_big',
        ],
    linux = dict(
        BLK_DEV_RAM_SIZE=16384,
        BLK_DEV_LOOP='y',
        BLK_DEV_LOOP_MIN_COUNT='8',
        CRYPTO_CCM='y',
        CRYPTO_GCM='y',
        CONSOLE_TRANSLATIONS='y',
        DEBUG_FS='y',
        GPIO_PCF857X='y',
        I2C_MV64XXX='y',
        MACH_U115='y',
        MARVELL_PHY='y',
        NET_DSA='y',
        NET_DSA_MV88E6XXX='y',
        NET_DSA_MV88E6XXX_GLOBAL2='y',
        NET_IPGRE_DEMUX='y',
        NET_IPGRE='y',
        RELAY='y',
        USB_SERIAL_CP210X='y',
        USB_SERIAL_PL2303='y',
        VT='y',
        VT_CONSOLE='y',
        ),
    user = dict(
        BOOT_UBOOT_MARVELL_380_TARGET='ac_u115',
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
        PROP_SWTEST_MII='y',
        USER_BUSYBOX_LSPCI='y',
        USER_BUSYBOX_SEQ='y',
        USER_BUSYBOX_GETTY='y',
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
        USER_ETHTOOL_ETHTOOL='y',
        USER_FDISK_FDISK='y',
        USER_FDISK_SFDISK='y',
        USER_FLATFSD_USE_FLASH_FS='y',
        USER_INIT_CONSOLE_SH='y',
        USER_IPROUTE2_TC_TC='y',
        USER_IPROUTE2_IP_ROUTEF='y',
        USER_IPROUTE2_IP_ROUTEL='y',
        USER_IPROUTE2_IP_RTACCT='y',
        USER_IPROUTE2_IP_RTMON='y',
        USER_LINUX_FIRMWARE='y',
        USER_LIBQMI='y',
        USER_LIBQMI_QMI_FIRMWARE_UPDATE='y',
        USER_NETFLASH_AUTODECOMPRESS='y',
        USER_NETFLASH_DUAL_IMAGES='n',
        USER_NETFLASH_CRYPTO_V3='y',
        USER_MTD_UTILS_ERASEALL='y',
        USER_MTD_UTILS_LOCK='y',
        USER_MTD_UTILS_UNLOCK='y',
        USER_PROCPS_SYSCTL='y',
        USER_PROCPS_TLOAD='y',
        USER_PROCPS_VMSTAT='y',
        USER_PROCPS_W='y',
        USER_SIGS_SIGS='y',
        USER_UBOOT_ENVTOOLS_OPTIONS='y',
        USER_UBOOT_ENVTOOLS_OPTION_ETHADDR='00:27:04:03:02:00',
        USER_UBOOT_ENVTOOLS_OPTION_ETH1ADDR='00:27:04:03:02:01',
        USER_UBOOT_ENVTOOLS_OPTION_ETH2ADDR='00:27:04:03:02:02',
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
        SPARSEMEM_MANUAL='y',
        SPARSEMEM_VMEMMAP='y',
        X86_RESERVE_LOW='64',
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
        DOUBLEFAULT='y',
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
        ENABLE_MUST_CHECK='y',
        CRYPTO_MANAGER_DISABLE_TESTS='y',
        CRYPTO_NULL='y',
        CRYPTO_MD4='y',
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
        'custom_telit_firmware_upload',
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
        USER_UBOOT_ENVTOOLS_OPTIONS='y',
        USER_UBOOT_ENVTOOLS_OPTION_ETHADDR='00:27:04:03:02:00',
        USER_UBOOT_ENVTOOLS_OPTION_ETH1ADDR='00:27:04:03:02:01',
        USER_UBOOT_ENVTOOLS_OPTION_ETH2ADDR='00:27:04:03:02:02',
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
        BOOT_UBOOT_MARVELL_370_TARGET='ac8300',
        USER_LIBQMI='y',
        USER_LIBQMI_QMI_FIRMWARE_UPDATE='y',
        )
    )

groups['busybox'] = dict(
    user = dict(
        USER_BUSYBOX_BUSYBOX='y',
        USER_BUSYBOX_ARPING='y',
        USER_BUSYBOX_ETHER_WAKE='y',
        USER_BUSYBOX_INCLUDE_SUSv2='y',
        USER_BUSYBOX_PLATFORM_LINUX='y',
        USER_BUSYBOX_FEATURE_BUFFERS_USE_MALLOC='y',
        USER_BUSYBOX_SHOW_USAGE='y',
        USER_BUSYBOX_FEATURE_VERBOSE_USAGE='y',
        USER_BUSYBOX_FEATURE_COMPRESS_USAGE='y',
        USER_BUSYBOX_LFS='y',
        USER_BUSYBOX_LONG_OPTS='y',
        USER_BUSYBOX_FEATURE_DEVPTS='y',
        USER_BUSYBOX_NO_DEBUG_LIB='y',
        USER_BUSYBOX_INSTALL_APPLET_SYMLINKS='y',
        USER_BUSYBOX_FEATURE_USE_TERMIOS='y',
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
        USER_BUSYBOX_BASENAME='y',
        USER_BUSYBOX_CAT='y',
        USER_BUSYBOX_DATE='y',
        USER_BUSYBOX_FEATURE_DATE_ISOFMT='y',
        USER_BUSYBOX_FEATURE_DATE_COMPAT='y',
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
        USER_BUSYBOX_FEATURE_MKDIR_LONG_OPTIONS='y',
        USER_BUSYBOX_MKFIFO='y',
        USER_BUSYBOX_MKNOD='y',
        USER_BUSYBOX_MV='y',
        USER_BUSYBOX_FEATURE_MV_LONG_OPTIONS='y',
        USER_BUSYBOX_NICE='y',
        USER_BUSYBOX_PRINTF='y',
        USER_BUSYBOX_PWD='y',
        USER_BUSYBOX_READLINK='y',
        USER_BUSYBOX_FEATURE_READLINK_FOLLOW='y',
        USER_BUSYBOX_RM='y',
        USER_BUSYBOX_RMDIR='y',
        USER_BUSYBOX_FEATURE_RMDIR_LONG_OPTIONS='y',
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
        USER_BUSYBOX_UNIQ='y',
        USER_BUSYBOX_USLEEP='y',
        USER_BUSYBOX_WC='y',
        USER_BUSYBOX_YES='y',
        USER_BUSYBOX_FEATURE_AUTOWIDTH='y',
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
        USER_BUSYBOX_FEATURE_GREP_EGREP_ALIAS='y',
        USER_BUSYBOX_FEATURE_GREP_FGREP_ALIAS='y',
        USER_BUSYBOX_FEATURE_GREP_CONTEXT='y',
        USER_BUSYBOX_XARGS='y',
        USER_BUSYBOX_HALT='y',
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
        USER_BUSYBOX_TELNET='y',
        USER_BUSYBOX_FEATURE_TELNET_TTYPE='y',
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
        )
    )

groups['busybox_big'] = dict(
    user = dict(
        USER_BUSYBOX_FEATURE_DATE_NANO='y',
        USER_BUSYBOX_HOSTID='y',
        USER_BUSYBOX_FEATURE_TEST_64='y',
        USER_BUSYBOX_FEATURE_TR_CLASSES='y',
        USER_BUSYBOX_FEATURE_TR_EQUIV='y',
        USER_BUSYBOX_BASE64='y',
        USER_BUSYBOX_CAL='y',
        USER_BUSYBOX_CKSUM='y',
        USER_BUSYBOX_COMM='y',
        USER_BUSYBOX_FEATURE_DD_IBS_OBS='y',
        USER_BUSYBOX_FEATURE_ENV_LONG_OPTIONS='y',
        USER_BUSYBOX_EXPAND='y',
        USER_BUSYBOX_FEATURE_EXPAND_LONG_OPTIONS='y',
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
        USER_BUSYBOX_FEATURE_FLOAT_SLEEP='y',
        USER_BUSYBOX_FEATURE_SORT_BIG='y',
        USER_BUSYBOX_SPLIT='y',
        USER_BUSYBOX_FEATURE_SPLIT_FANCY='y',
        USER_BUSYBOX_SUM='y',
        USER_BUSYBOX_TAC='y',
        USER_BUSYBOX_FEATURE_TEE_USE_BLOCK_IO='y',
        USER_BUSYBOX_UNEXPAND='y',
        USER_BUSYBOX_FEATURE_UNEXPAND_LONG_OPTIONS='y',
        USER_BUSYBOX_FEATURE_WC_LARGE='y',
        USER_BUSYBOX_WHOAMI='y',
        USER_BUSYBOX_FEATURE_PRESERVE_HARDLINKS='y',
        USER_BUSYBOX_FEATURE_MD5_SHA1_SUM_CHECK='y',
        USER_BUSYBOX_FEATURE_VI_REGEX_SEARCH='y',
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
        USER_BUSYBOX_FEATURE_XARGS_SUPPORT_QUOTES='y',
        USER_BUSYBOX_FEATURE_XARGS_SUPPORT_TERMOPT='y',
        USER_BUSYBOX_FEATURE_XARGS_SUPPORT_ZERO_TERM='y',
        USER_BUSYBOX_BLKID='y',
        USER_BUSYBOX_FEATURE_BLKID_TYPE='y',
        USER_BUSYBOX_FINDFS='y',
        USER_BUSYBOX_GETOPT='y',
        USER_BUSYBOX_FEATURE_GETOPT_LONG='y',
        USER_BUSYBOX_HWCLOCK='y',
        USER_BUSYBOX_FEATURE_HWCLOCK_LONG_OPTIONS='y',
        USER_BUSYBOX_FEATURE_HWCLOCK_ADJTIME_FHS='y',
        USER_BUSYBOX_LOSETUP='y',
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
        USER_BUSYBOX_SH_MATH_SUPPORT_64='y'
        )
    )

groups['busybox_ash'] = dict(
    user = dict(
        USER_BUSYBOX_ASH='y',
        USER_BUSYBOX_ASH_BASH_COMPAT='y',
        USER_BUSYBOX_ASH_JOB_CONTROL='y',
        USER_BUSYBOX_ASH_ALIAS='y',
        USER_BUSYBOX_ASH_GETOPTS='y',
        USER_BUSYBOX_ASH_BUILTIN_ECHO='y',
        USER_BUSYBOX_ASH_BUILTIN_PRINTF='y',
        USER_BUSYBOX_ASH_BUILTIN_TEST='y',
        USER_BUSYBOX_ASH_OPTIMIZE_FOR_SIZE='y',
        USER_BUSYBOX_ASH_RANDOM_SUPPORT='y',
        USER_BUSYBOX_FEATURE_SH_IS_ASH='y',
        USER_BUSYBOX_FEATURE_BASH_IS_ASH='y',
        USER_BUSYBOX_SH_MATH_SUPPORT='y',
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
        USER_E2FSPROGS_UUIDD='y',
        USER_E2FSPROGS_UUIDGEN='y'
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

groups['quagga'] = dict(
    user = dict(
        USER_QUAGGA_ZEBRA_ZEBRA='y',
        USER_QUAGGA_BGPD_BGPD='y',
        USER_QUAGGA_RIPD_RIPD='y',
        USER_QUAGGA_RIPNGD_RIPNGD='y',
        USER_QUAGGA_OSPFD_OSPFD='y',
        USER_QUAGGA_OSPF6D_OSPF6D='y',
        USER_QUAGGA_ISISD_ISISD='y',
        USER_QUAGGA_BABELD_BABELD='y',
        LIB_NCURSES='y',
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
        ],
    user = dict(
        LIB_FLEX='y',
        LIB_LIBSSL='y',
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
        PROP_CONFIG_WEBUI='y',
        PROP_CONFIG_KEYS='y',
        PROP_CONFIG_ACTIOND='y',
        PROP_ACCNS_CERTIFICATES='y',
        USER_BUSYBOX_FEATURE_SORT_BIG='y',
        USER_INIT_INIT='y',
        USER_GETTYD_GETTYD='y',
        USER_FCRON='y',
        USER_FLASHW_FLASHW='y',
        USER_SETMAC_SETMAC='y',
        USER_UBOOT_ENVTOOLS=['y','n'],
        USER_UBOOT_ENVTOOLS_ENV_OVERWRITE=['y','n'],
        USER_NETFLASH_NETFLASH='y',
        USER_NETFLASH_WITH_FTP='y',
        USER_NETFLASH_WITH_CGI='y',
        USER_NETFLASH_HARDWARE='y',
        USER_NETFLASH_DECOMPRESS='y',
        USER_FLATFSD_FLATFSD='y',
        USER_FLATFSD_EXTERNAL_INIT='y',
        USER_BRCTL_BRCTL='y',
        USER_DISCARD_DISCARD='y',
        USER_DISCARD_NO_INSTALL='y',
        USER_DNSMASQ2_DNSMASQ2='y',
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
        USER_UTIL_LINUX_LIBUUID='y'
        )
    )

groups['user_factory'] = dict(
    include = [
        'busybox',
        'busybox_ash',
        'init_overlay',
        'tcpdump',
        ],
    user = dict(
        LIB_LIBSSL='y',
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
        USER_INIT_INIT='y',
        USER_GETTYD_GETTYD='y',
        USER_FLASHW_FLASHW='y',
        USER_SETMAC_SETMAC='y',
        USER_UBOOT_ENVTOOLS='y',
        USER_UBOOT_ENVTOOLS_ENV_OVERWRITE='y',
        USER_NETFLASH_NETFLASH='y',
        USER_NETFLASH_WITH_FTP='y',
        USER_NETFLASH_WITH_CGI='y',
        USER_NETFLASH_HARDWARE='y',
        USER_NETFLASH_DECOMPRESS='y',
        USER_FLATFSD_FLATFSD='y',
        USER_FLATFSD_EXTERNAL_INIT='y',
        USER_DISCARD_DISCARD='y',
        USER_DISCARD_NO_INSTALL='y',
        USER_DNSMASQ2_DNSMASQ2='y',
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
        )
    )

groups['user_firewall'] = dict(
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
        USER_BUSYBOX_FEATURE_HTTPD_USE_SENDFILE='y',
        USER_BUSYBOX_FEATURE_HTTPD_SETUID='y',
        USER_BUSYBOX_FEATURE_HTTPD_BASIC_AUTH='y',
        USER_BUSYBOX_FEATURE_HTTPD_CGI='y',
        USER_BUSYBOX_FEATURE_HTTPD_CONFIG_WITH_SCRIPT_INTERPR='y',
        USER_BUSYBOX_FEATURE_HTTPD_SET_REMOTE_PORT_TO_ENV='y',
        USER_BUSYBOX_FEATURE_HTTPD_ENCODE_URL_STR='y',
        USER_BUSYBOX_FEATURE_HTTPD_ERROR_PAGES='y',
        USER_BUSYBOX_FEATURE_HTTPD_PROXY='y',
        )
    )

groups['flashrom'] = dict(
    linux = dict(
        DEVMEM='y',
        DEVKMEM='y',
        ),
    user = dict(
        USER_FLASHROM='y',
        )
    )

groups['hostapd'] = dict(
    user = dict(
        USER_HOSTAPD_HOSTAPD='y',
        )
    )

groups['init_overlay'] = dict(
    linux = dict(
        OVERLAY_FS='y',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='y',
        ),
    user = dict(
        USER_INIT_INIT='y',
        USER_INIT_OVERLAY='y',
        USER_BUSYBOX_PIVOT_ROOT='y',
        USER_BUSYBOX_CHROOT='y',
        )
    )

groups['iperf'] = dict(
    user = dict(
        C_PLUS_PLUS='y',
        USER_IPERF_IPERF='y',
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
        USER_BUSYBOX_LSUSB='y',
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
        MACVLAN='y',
        MACVTAP='y',
        LIBCRC32C='y',
        BALLOON_COMPACTION='y',
        PCI_STUB='y',
        NET_FOU='y',
        VXLAN='y',
        GENEVE='y'
    ),
    user = dict(
        USER_LIBVIRT='y',
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
        LIB_YAJL='y',
        USER_BUSYBOX_FLOCK='y',
        USER_RESOLVEIP_RESOLVEIP='y',
        USER_JERRYSCRIPT='y',
        )
    )

groups['config_factory'] = dict(
    include = [
        'linux_common',
        'linux_iptables_ipv4',
        'user_factory',
        'user_firewall',
        ],
    linux = dict (
        IP_NF_ARPTABLES='m',
        IP_NF_ARPFILTER='m',
        IP_NF_ARP_MANGLE='m',
        ),
    user = dict (
        PROP_FACTORY_FACTORY='y',
        USER_EMCTEST_EMCECHO='y',
        PROP_CM1002JIG='y',
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
        USER_BUSYBOX_SH_MATH_SUPPORT_64='y',
        USER_BUSYBOX_SYSLOGD='y',
        USER_BUSYBOX_FEATURE_ROTATE_LOGFILE='y',
        USER_BUSYBOX_FEATURE_REMOTE_LOG='y',
        USER_BUSYBOX_FEATURE_SYSLOGD_CFG='y',
        USER_BUSYBOX_FEATURE_SYSLOGD_READ_BUFFER_SIZE='256',
        USER_BUSYBOX_FEATURE_KMSG_SYSLOG='y',
        USER_BUSYBOX_KLOGD='y',
        USER_BUSYBOX_FEATURE_KLOGD_KLOGCTL='y',
        USER_MODEMMANAGER='y',
        USER_MODEMMANAGER_ALLPLUGINS='y',
        USER_I2C_TOOLS='y',
        USER_NTPD_NTPD='y',
        )
    )

groups['config_cellular'] = dict(
    include = [
        'usb',
        ],
    user = dict(
        LIB_DBUS='y',
        LIB_DBUS_GLIB='y',
        USER_LIBQMI='y',
        USER_LIBMBIM='y',
        USER_MODEMMANAGER='y',
        USER_MOBILE_BROADBAND_PROVIDER_INFO='y',
        USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_FULL='y',
        USER_MODEMMANAGER_ALLPLUGINS='y',
        )
    )

groups['config_cellular_small'] = dict(
    include = [
        'usb',
        ],
    user = dict(
        LIB_DBUS='y',
        LIB_DBUS_GLIB='y',
        USER_LIBQMI='y',
        USER_MODEMMANAGER='y',
        USER_MODEMMANAGER_PLUGIN_GOBI='y',
        USER_MODEMMANAGER_PLUGIN_HUAWEI='y',
        USER_MOBILE_BROADBAND_PROVIDER_INFO='y',
        USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_FULL='y',
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

groups['config_serial'] = dict(
    user = dict(
        PROP_CONFIG_REMOTE_MANAGER='y',
        USER_SHELLINABOX='y',
        PROP_CONFIG_XMODEM='y',
        PROP_CONFIG_SERIALD='y',
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
        PROP_CONFIG_WEBUI_CGIBIN_DASHBOARD='y'
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
        PROP_CONFIG_WEBUI_APPLICATIONS='y',
        )
    )

groups['custom_telit_firmware_upload'] = dict(
    user = dict(
        PROP_TELIT_CUSTOM_FIRMWARE='y',
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

groups['config_cli_analyzer'] = dict(
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
    linux = dict (
        DEFAULT_SECURITY_DAC='n',
        )
    )

groups['config_security_dac_y'] = dict(
    linux = dict (
        DEFAULT_SECURITY_DAC='y',
        )
    )

groups['config_apparmor'] = dict(
    linux = dict (
        SECURITY='y',
        SECURITY_APPARMOR='y',
        SECURITY_APPARMOR_BOOTPARAM_VALUE=1,
        SECURITY_APPARMOR_HASH='y',
        SECURITY_APPARMOR_HASH_DEFAULT='y',
        INTEGRITY='y',
        INTEGRITY_AUDIT='y',
        DEFAULT_SECURITY_APPARMOR='y',
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
        )
    )

groups['config_quagga'] = dict(
    include = [
        'quagga',
        ],
    user = dict(
        USER_BUSYBOX_SEQ='y',
        )
    )

groups['config_fips'] = dict(
    user = dict(
        LIB_LIBSSL_FIPS='y',
        )
    )

groups['config_virt'] = dict(
    include = [
        'qemu_x86',
        'virtualisation_extras',
        ],
    )

groups['config_wireless'] = dict(
    include = [
        'hostapd',
        ],
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
        NF_CT_PROTO_GRE='m',
        NET_IPGRE_DEMUX='y',
        NET_IP_TUNNEL='y',
        NET_IPGRE='y',
        ),
    user = dict(
        PROP_CONFIG_IPTUNNEL='y',
        )
    )


groups['config_snmp'] = dict(
    user = dict(
        USER_NETSNMP_SNMPD='y',
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
        NET_CLS_IND='y',
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

groups['config_base_features'] = dict(
    include = [
        'config',
        'config_cli',
        'config_cli_analyzer',
        'config_cloud',
        'config_quagga',
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
        'pppd_pppoe',
        'config_digimdns',
        ],
    )

groups['config_5300_dc'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cellular_small',
        'config_vrrp',
        'config_ddns',
        # OLD PRODUCT: FEATURE FREEZE
        ],
    )

groups['config_5301_dc'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_vrrp',
        'config_ddns',
        # OLD PRODUCT: FEATURE FREEZE
        ],
    user = dict(
        PROP_SECUREIP='y'
        )
    )

groups['config_5400_rm'] = dict(
    include = [
        'config',
        'config_5401_rm',
        'config_cellular',
        ],
    )

groups['config_5401_rm'] = dict(
    include = [
        'config',
        'config_cli',
        'config_cli_legacy',
        'config_cloud',
        'config_serial',
        'config_quagga',
        'config_nagios',
        'config_netflow',
        'config_fips',
        'config_vrrp',
        'config_ddns',
        'config_qos',
        'config_iptunnel',
        # OLD PRODUCT: ESSENTIAL FEATURES ONLY
        ],
    )

groups['config_connectit'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        'config_serial',
        'config_fips',
        ],
    user = dict(
        PROP_CONFIG_SERIAL_EXCLUSIVE_ACCESS='y',
        ),
    )

groups['config_connectit16'] = dict(
    include = [
        'config_connectit48',
        ]
    )

groups['config_connectit48'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        'config_serial',
        'config_apparmor',
        'config_python',
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
        USER_DMIDECODE='y',
        PROP_CONFIG_HIDE_AVIEW='y',
        ),
    )

groups['config_6300_cx'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        ],
    )

groups['config_6300_lx'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        ],
    )

groups['config_ix14'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        'config_serial',
        'config_apparmor',
        'config_cloud',
        'config_python',
        #
        # uncomment the following to enable wireless on the IX14
        #
        #'wireless_ixx4',
        ],
    user = dict(
        USER_BLUEZ_BLUETOOTHD='y',
        PROP_BLUETOOTH='y',
        USER_BLE_PROVISIONING_APP='y',
        PROP_BLUETOOTH_QCA65x4_FW='y',
        PROP_CONFIG_HIDE_AVIEW='y',
        ),
    )

groups['config_6310_dx'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        ],
    )

groups['config_ex15'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        'config_serial',
        ],
    )

groups['config_ex15w'] = dict(
    include = [
        'config_ex15',
        'config_wireless',
        ],
    user = dict(
        USER_CRDA_REGDB_UNRESTRICTED_WORLD='y',
        ),
    )

groups['config_ex55'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        'config_wireless',
        ],
    )

groups['config_tx54'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        'config_wireless',
        'config_serial',
        ],
    )

groups['config_tx64'] = dict(
    include = [
        'config',
        'config_base_features',
        'config_cellular',
        'config_wireless',
        'config_serial',
        'config_python',
        ],
    user = dict(
        PROP_CONFIG_HIDE_AVIEW='y',
        USER_BUSYBOX_MKFS_VFAT='y',
        USER_E2FSPROGS='y',
        USER_E2FSPROGS_MKFS_EXT4='y',
        USER_E2FSPROGS_FSCK_EXT4='y',
        USER_GPTFDISK='y',
        USER_PARTED='y',
        USER_DMIDECODE='y',
        ),
    )

groups['config_anywhereusb8'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        'config_wireless',
        'config_serial',
        'config_apparmor',
        ],
    )

groups['config_6335_mx'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        'config_serial',
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
        'config_cli_legacy',
        'config_cli_analyzer',
        'config_base_features',
        'config_cellular',
        'config_serial',
        ],
    )

groups['config_6350_sr'] = dict(
    include = [
        'config_6355_sr',
        'config_wireless',
        ],
    )

groups['config_8300'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cellular',
        'config_intelliflow',
        'config_netflow',
        'config_wireless',
        'config_vrrp',
        'config_ddns',
        'config_iptunnel',
        ]
    )

groups['config_u115'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cellular',
        'config_intelliflow',
        'config_quagga',
        'config_netflow',
        'config_vrrp',
        'config_ddns',
        'config_serial',
        'config_iptunnel',
        ]
    )

groups['config_9400_ua'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cellular',
        'config_virt',
        'config_quagga',
        'config_nagios',
        'config_netflow',
        'config_vrrp',
        'config_ddns',
        ]
    )

groups['config_sprite'] = dict(
    include = [
        'config',
        'config_cli_legacy',
        'config_cellular',
        ],
    user = dict(
        PROP_JUNIPER='y',
        PROP_JUNIPER_SPRITE='y',
        )
    )

groups['config_factory8300'] = dict(
    include = [
        'config_factory',
        ]
    )

products['AC 5300-DC'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '5300-DC',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_5300_dc',
        'config_5300_dc',
        'strongswan_netkey',
        'linux_crypto_arm',
        'config_security_dac_y',
        ]
    )

products['AC 5301-DC'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '5301-DC',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_5301_dc',
        'config_5301_dc',
        'strongswan_netkey',
        'linux_crypto_arm',
        'config_security_dac_y',
        ]
    )

products['AC 5400-RM'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '5400-RM',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
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
    cc = 'arm-linux-gnueabi-20180525-gcc',
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
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_connectit_mini',
        'config_connectit',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'config_security_dac_y',
        ]
    )

products['Digi Connect IT 4'] = dict(
    vendor = 'Digi',
    product = 'ConnectIT4',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_connectit4',
        'config_connectit',
        'strongswan_netkey',
        'linux_crypto_arm',
        'config_security_dac_y',
        'config_stlport',
        ]
    )

products['Digi Connect IT 16'] = dict(
    vendor = 'Digi',
    product = 'ConnectIT16',
    arch = 'x86',
    cc = 'x86_64-linux-20151029-gcc',
    include = [
        'hardware_connectit16',
        'config_connectit16',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'linux_i2c_nexcom',
        'config_stdcpp',
        'custom_telit_firmware_upload',
        ]
    )

products['Digi Connect IT 48'] = dict(
    vendor = 'Digi',
    product = 'ConnectIT48',
    arch = 'x86',
    cc = 'x86_64-linux-20151029-gcc',
    include = [
        'hardware_connectit48',
        'config_connectit48',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'linux_i2c_nexcom',
        'config_stdcpp',
        'custom_telit_firmware_upload',
        ]
    )

products['AC 6300-CX'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6300-CX',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_6300_cx',
        'config_6300_cx',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'config_security_dac_y',
        ]
    )

products['AC 6300-LX'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6300-LX',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_6300_lx',
        'config_6300_lx',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'config_security_dac_y',
        ]
    )

products['AC 6310-DX'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6310-DX',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_6310_dx',
        'config_6310_dx',
        'strongswan_netkey',
        'linux_crypto_arm',
        'config_security_dac_y',
        'config_stlport',
        ]
    )

products['Digi IX14'] = dict(
    vendor = 'Digi',
    product = 'IX14',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_ix14',
        'config_ix14',
        'strongswan_netkey',
        'linux_crypto_arm',
        'config_stdcpp',
        ]
    )

products['Digi EX15'] = dict(
    vendor = 'Digi',
    product = 'EX15',
    arch = 'mips',
    cc = ' mips-linux-20170624-gcc',
    include = [
        'hardware_ex15',
        'config_ex15',
        'strongswan_netkey',
        'config_stlport',
        ]
    )

products['Digi EX15W'] = dict(
    vendor = 'Digi',
    product = 'EX15W',
    arch = 'mips',
    cc = ' mips-linux-20170624-gcc',
    include = [
        'hardware_ex15w',
        'config_ex15w',
        'strongswan_netkey',
        'config_stlport',
        ]
    )

products['Digi EX55'] = dict(
    vendor = 'Digi',
    product = 'EX55',
    arch = 'mips',
    cc = ' mips-linux-20170624-gcc',
    include = [
        'hardware_ex55',
        'config_ex55',
        'strongswan_netkey',
        'config_stlport',
        ]
    )

products['Digi TX54'] = dict(
    vendor = 'Digi',
    product = 'TX54',
    arch = 'mips',
    cc = ' mips-linux-20170624-gcc',
    include = [
        'hardware_tx54',
        'config_tx54',
        'strongswan_netkey',
        'config_stlport',
        ]
    )

products['Digi TX64'] = dict(
    vendor = 'Digi',
    product = 'TX64',
    arch = 'x86',
    cc = 'x86_64-linux-20151029-gcc',
    include = [
        'hardware_tx64',
        'config_tx64',
        'busybox_big',
        'e2fsprogs_all',
        'iperf',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'config_security_dac_y',
        'config_stlport_shared',
        'config_stdcpp',
        ]
    )

products['Digi AnywhereUSB8'] = dict(
    vendor = 'Digi',
    product = 'AnywhereUSB8',
    arch = 'arm64',
    cc = 'aarch64-linux-20171204-gcc',
    include = [
        'hardware_layerscape_1046',
        'hardware_wireless',
        'hardware_cellular_cm',
        'hardware_nor',
        'hardware_mmc',
        'linux_pci_layerscape',
        'linux_crypto_arm64',
        'linux_crypto_smp',
        'linux_hugepages',
        'linux_sata',
        'config_anywhereusb8',
        'config_security_dac_n',
        'config_stdcpp',
        'busybox_big',
        'strongswan_netkey',
        ],
    device = dict(
        DEFAULTS_LIBC_GLIBC='y',
        ),
    linux = dict(
        LEDS_AWUSB24='y',
        MARVELL_PHY='y',
        MARVELL_10G_PHY='y',
        MV88X3310P_PHY='y',
        RTC_NVMEM='y',
        RTC_DRV_DS1307='y',
        RTC_DRV_DS1307_CENTURY='y',
        ),
    user = dict(
        USER_BUSYBOX_GETTY='y',
        USER_BUSYBOX_LSPCI='y',
        )
    )

products['AC 6330-MX'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6330-MX',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_6330_mx',
        'config_6330_mx',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'config_security_dac_y',
        'config_stlport',
        ]
    )

products['AC 6335-MX'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6335-MX',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_6335_mx',
        'config_6335_mx',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'config_security_dac_y',
        'config_stlport',
        ]
    )

products['AC 6350-SR'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6350-SR',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_6350_sr',
        'config_6350_sr',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'config_security_dac_y',
        'config_stlport',
        ]
    )

products['AC 6355-SR'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '6355-SR',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_6355_sr',
        'config_6355_sr',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'config_security_dac_y',
        'config_stlport',
        ]
    )

products['AC 8300'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '8300',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_8300',
        'config_8300',
        'iperf',
        'snort',
        'strongswan_netkey',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'config_stlport_shared',
        'config_security_dac_y',
        ]
    )

products['AC 9400-UA'] = dict(
    vendor = 'AcceleratedConcepts',
    product = '9400-UA',
    arch = 'x86',
    cc = 'x86_64-linux-20151029-gcc',
    include = [
        'hardware_9400_ua',
        'config_9400_ua',
        'busybox_big',
        'e2fsprogs_all',
        'iperf',
        'strongswan_netkey',
        'linux_crypto_x86',
        'linux_crypto_smp',
        'config_security_dac_y',
        'config_stlport_shared',
        ]
    )

products['AC Factory8300'] = dict(
    vendor = 'AcceleratedConcepts',
    product = 'Factory8300',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_factory8300',
        'config_factory8300',
        'config_security_dac_y',
        'config_stlport',
        ]
    )

products['AC FactoryU115'] = dict(
    vendor = 'AcceleratedConcepts',
    product = 'FactoryU115',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_factoryU115',
        'config_factory',
        'config_security_dac_y',
        'config_stlport',
        ]
    )

products['AC Sprite'] = dict(
    vendor = 'AcceleratedConcepts',
    product = 'sprite',
    arch = 'arm',
    cc = 'arm-linux-gnueabi-20180525-gcc',
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
    cc = 'arm-linux-gnueabi-20180525-gcc',
    include = [
        'hardware_u115',
        'config_u115',
        'linux_crypto',
        'linux_crypto_mv_cesa',
        'linux_crypto_arm',
        'busybox_big',
        'strongswan_netkey',
        'config_security_dac_y',
        'config_stlport',
        ]
    )

products['AC Porter'] = dict(
    vendor = 'AcceleratedConcepts',
    product = 'porter',
    arch = 'x86',
    cc = 'x86_64-linux-20151029-gcc',
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
    cc = 'arm-linux-gnueabi-20180525-gcc',
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
        'config_stlport',
        'config_security_dac_y',
        ],
    user = dict(
        PROP_LOGD_LOGD='y',
        PROP_LOGD_LOGINIT='y',
        USER_NETFLASH_CRYPTO_OPTIONAL='y',
        USER_INIT_DEFAULT_ENTRIES='y',
        USER_INIT_CONSOLE_SH='y',
        LIB_LIBUNBOUND='y',
        LIB_LIBLDNS='y',
        )
    )

for name, group in groups.items():
    group['name'] = name

for name, product in products.items():
    product['name'] = name

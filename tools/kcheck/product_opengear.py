
products = {}
groups = {}

# A note on indentation: This file uses indentation to
# give structure to the config symbols when they don't have
# an obvious relationship. For example, please use single
# indentation for grouping non-obviously related symbols
# like so:
#
#    FOO='y',
#     BAR='y',            # (depends on FOO)
#      BAZ='y',           # (depends on BAR)
#
# You can leave off indentation when the dependent symbols
# have the same prefix, but only if it is clear that there
# is a dependency. e.g.:
#
#    FOO='y',
#    FOO_BAR='y',
#    FOO_BAZ='y',
#
# The principle here is that it should be quick
# to visually deduce the relationship between
# dependent symbols.

groups['opengear/linux_netfilter'] = dict(
    linux = dict(
        NETFILTER='y',
        NETFILTER_ADVANCED='y',
        NETFILTER_NETLINK_LOG='m',
        # NETFILTER_INGRESS='y',

        NF_CONNTRACK='m',
        NF_CONNTRACK_MARK='y',
        NF_CONNTRACK_PROCFS='y',          # obsolete
        # NF_CONNTRACK_EVENTS='m',
        # NF_CONNTRACK_PROC_COMPAT='m',
        NF_CONNTRACK_FTP='m',
        # NF_CONNTRACK_H323='m',
        NF_CONNTRACK_IRC='m',
        # NF_CONNTRACK_SNMP='m',
        # NF_CONNTRACK_PPTP='m',
        NF_CONNTRACK_SIP='m',
        NF_CONNTRACK_TFTP='m',

        # NF_CT_NETLINK='m',
        # NF_CT_PROTO_GRE='m',

        NF_NAT='m',
        # NF_NAT_NEEDED='m',
        NF_NAT_FTP='m',
        NF_NAT_IRC='m',
        NF_NAT_SIP='m',
        NF_NAT_TFTP='m',
        # NF_NAT_PPTP='m',
        # NF_NAT_SNMP_BASIC='m',
        # NF_NAT_PROTO_GRE='m',
    ),
)

groups['opengear/linux_netfilter_xt'] = dict(
    include = ['opengear/linux_netfilter'],
    linux = dict(
        NETFILTER_XTABLES='m',

        NETFILTER_XT_CONNMARK='m',
        NETFILTER_XT_MARK='m',
        NETFILTER_XT_NAT='m',

        # NETFILTER_XT_TARGET_CT='m',
        NETFILTER_XT_TARGET_CONNMARK='m',
        NETFILTER_XT_TARGET_DSCP='m',
        NETFILTER_XT_TARGET_LOG='m',
        # NETFILTER_XT_TARGET_NETMAP='m',
        NETFILTER_XT_TARGET_NFLOG='m',
        # NETFILTER_XT_TARGET_NOTRACK='m',
        # NETFILTER_XT_TARGET_REDIRECT='m',
        # NETFILTER_XT_TARGET_TPROXY='m',
        NETFILTER_XT_TARGET_TCPMSS='m',
        # NETFILTER_XT_TARGET_TRACE='m',
        NETFILTER_XT_TARGET_MARK='m',

        # NETFILTER_XT_MATCH_ADDRTYPE='m',
        # NETFILTER_XT_MATCH_COMMENT='m',
        # NETFILTER_XT_MATCH_CONNLIMIT='m',
        NETFILTER_XT_MATCH_CONNMARK='m',
        NETFILTER_XT_MATCH_CONNTRACK='m',
        NETFILTER_XT_MATCH_DSCP='m',
        # NETFILTER_XT_MATCH_ECN='m',
        # NETFILTER_XT_MATCH_ESP='m',
        # NETFILTER_XT_MATCH_HELPER='m',
        # NETFILTER_XT_MATCH_HL='m',
        # NETFILTER_XT_MATCH_IPRANGE='m',
        # NETFILTER_XT_MATCH_LENGTH='m',
        NETFILTER_XT_MATCH_LIMIT='m',
        NETFILTER_XT_MATCH_MAC='m',
        NETFILTER_XT_MATCH_MARK='m',
        # NETFILTER_XT_MATCH_MULTIPORT='m',
        # NETFILTER_XT_MATCH_PKTTYPE='m',
        # NETFILTER_XT_MATCH_REALM='m',
        NETFILTER_XT_MATCH_RECENT='m',
        # NETFILTER_XT_MATCH_SOCKET='m',
        # NETFILTER_XT_MATCH_STATISTIC='m',
        NETFILTER_XT_MATCH_STATE='m',
        # NETFILTER_XT_MATCH_STRING='m',
        NETFILTER_XT_MATCH_TCPMSS='m',
        # NETFILTER_XT_MATCH_TIME='m',
    ),
)

groups['opengear/linux_netfilter_ipv4'] = dict(
    include = ['opengear/linux_netfilter_xt'],
    linux = dict(
        NF_DEFRAG_IPV4='m',
        NF_SOCKET_IPV4='m',
    ),
)

groups['opengear/linux_netfilter_ipv6'] = dict(
    include = ['opengear/linux_netfilter_xt'],
    linux = dict(
        NF_DEFRAG_IPV6='m',
        NF_SOCKET_IPV6='m',
    ),
)

groups['opengear/linux_iptables'] = dict(
    include = ['opengear/linux_netfilter_ipv4'],
    linux = dict(
        NF_REJECT_IPV4='m',
        NF_LOG_IPV4='m',

        IP_NF_IPTABLES='m',

        # IP_NF_RAW='m',
        IP_NF_FILTER='m',
        IP_NF_MANGLE='m',
        IP_NF_NAT='m',

        # IP_NF_MATCH_RPFILTER='m',

        IP_NF_TARGET_REJECT='m',
        IP_NF_TARGET_MASQUERADE='m',
        # IP_NF_TARGET_ECN='m',
    ),
)

groups['opengear/linux_ip6tables'] = dict(
    include = ['opengear/linux_netfilter_ipv6'],
    linux = dict(
        NF_REJECT_IPV6='m',
        NF_LOG_IPV6='m',

        IP6_NF_IPTABLES='m',

        # IP6_NF_RAW='m',
        IP6_NF_FILTER='m',
        IP6_NF_MANGLE='m',
        IP6_NF_NAT='m',

        # IP6_NF_MATCH_EUI64='m',
        # IP6_NF_MATCH_RPFILTER='m',
        IP6_NF_MATCH_IPV6HEADER='m',

        IP6_NF_TARGET_REJECT='m',
        IP6_NF_TARGET_MASQUERADE='m',
        IP6_NF_TARGET_NPT='m',
    ),
)

# Minimal user and Linux for hardware bringup and basic diag
groups['opengear/minimum'] = dict(
    linux = dict(
        EMBEDDED='y',           # this enables EXPERT and other options
        MODULES='y',            # allow kmod loading
        FW_LOADER='y',          # allow device firmware loading from userspc

        # executables
        BINFMT_ELF='y',         # standard ELF executables
        BINFMT_SCRIPT='y',      # standard #! scripts

        # process & syscalls
        MULTIUSER='y',
         UID16='y',              # small UIDs
        COMPAT_32BIT_TIME='y',  # needed for musl pselect6

        # filesystems
        BLOCK='y',              # enable disk support
         BLK_DEV='y',
          BLK_DEV_RAM='y',        # initrd
           BLK_DEV_RAM_SIZE=32768,
        DEVTMPFS='y',           # /dev filesystem
         DEVTMPFS_MOUNT='y',     # auto-mount /dev for pid 1
        SYSFS='y',              # /sys filesystem
        PROC_FS='y',            # /proc filesystem

        # field recovery/diag
        PANIC_TIMEOUT=[-1, 0],  # 0:panic->halt -1=panic->reboot
        DEBUG_FS='y',           # /sys/kernel/debug
         USB_MON=['y', 'n'],     # /sys/kernel/debug/usb/mon
        BLK_DEBUG_FS='y',
        MAGIC_SYSRQ=['y', 'n'],     # prefer BREAK on console diag
        BUG='y',
         DEBUG_BUGVERBOSE='y',
        VM_EVENT_COUNTERS='y',  # /proc/vmstat
        PRINTK='y',
        PRINTK_TIME='y',        # old style timestamping

        # basic network protocol support
        NET='y',
         UNIX='y',
         INET='y',
         IPV6='y',

        # tty support
        TTY='y',
         VT=['n', 'y'],           # prefer no VTs when we have serial console
         LEGACY_PTYS=['n', 'y'],  # prefer no BSD ptys
         UNIX98_PTYS='y',         # ptys are needed by sshd


        # hw monitoring framework
        HWMON='y',
         THERMAL='y',
         THERMAL_HWMON='y',

        # GPIO framework
        GPIOLIB='y',
         GPIO_SYSFS='y',
         # GPIO_CDEV='y',
    ),
    user = dict(
        USER_INIT_INIT='y',         # enough to boot,
         USER_INIT_EDINITTAB='y',    # edinittab
        USER_AGETTY_AGETTY='y',     # login,
        USER_BASH_BASH=['y', 'n'],  # get a shell,
        USER_BUSYBOX='y',           # have some posix commands like 'ls',
         USER_BUSYBOX_MOUNT='y',
         USER_BUSYBOX_WATCH='y',
         USER_BUSYBOX_LFS='y',       # workaround a libbb.h bug in busybox :(
         USER_BUSYBOX_NO_DEBUG_LIB='y',  # more build workarounds :(
        USER_NETFLASH_NETFLASH='y',     # change firmware,
         USER_NETFLASH_WITH_FTP='y',
         USER_NETFLASH_HARDWARE='y',

        USER_SETFSET_SETFSET='y',   # query factory options,
        LIB_LIBGPIOD_TOOLS='y',     # and fiddle with GPIOs

    ),
)

#------------------------------------------------------------
# Software-only config groups

# POSIX.1-2017 base utilities [status=aspirational]
#  This group enables (almost) all the utilities defined by POSIX.1-2017
#  TODO: the 'XXX' indicates utilities required by POSIX,
#        but not yet supplied; should probably implement them.
#        See busybox's docs/posix_conformance.txt
#
groups['opengear/busybox-posix'] = dict(
    user = dict(
        # alias (shell)
        USER_BUSYBOX_AR='y',
        # at XXX
        USER_BUSYBOX_AWK='y',
        USER_BUSYBOX_BASENAME='y',
        # batch XXX
        USER_BUSYBOX_BC='y',
        USER_BUSYBOX_CAT='y',
        USER_BUSYBOX_CHGRP='y',
        USER_BUSYBOX_CHMOD='y',
        USER_BUSYBOX_CHOWN='y',
        USER_BUSYBOX_CKSUM='y',
        USER_BUSYBOX_CMP='y',
        USER_BUSYBOX_COMM='y',
        USER_BUSYBOX_CP='y',
        USER_BUSYBOX_CRONTAB='y',
        # csplit XXX
        USER_BUSYBOX_CUT='y',
        USER_BUSYBOX_DATE='y',
        USER_BUSYBOX_DD='y',
        USER_BUSYBOX_DIFF='y',
        USER_BUSYBOX_DIRNAME='y',
        USER_BUSYBOX_DU='y',
        USER_BUSYBOX_ECHO='y',
        USER_BUSYBOX_ED='y',
        USER_BUSYBOX_ENV='y',
        USER_BUSYBOX_EXPAND='y',
        USER_BUSYBOX_EXPR='y',
        USER_BUSYBOX_FALSE='y',
        # file XXX
        USER_BUSYBOX_FIND='y',
         USER_BUSYBOX_FEATURE_FIND_PATH='y',
         USER_BUSYBOX_FEATURE_FIND_XDEV='y',
         USER_BUSYBOX_FEATURE_FIND_PRUNE='y',
         USER_BUSYBOX_FEATURE_FIND_PERM='y',
         USER_BUSYBOX_FEATURE_FIND_TYPE='y',
         USER_BUSYBOX_FEATURE_FIND_LINKS='y',
         USER_BUSYBOX_FEATURE_FIND_USER='y',
         USER_BUSYBOX_FEATURE_FIND_GROUP='y',
         USER_BUSYBOX_FEATURE_FIND_SIZE='y',
         USER_BUSYBOX_FEATURE_FIND_MTIME='y',
         USER_BUSYBOX_FEATURE_FIND_EXEC='y',
         USER_BUSYBOX_FEATURE_FIND_NEWER='y',
         USER_BUSYBOX_FEATURE_FIND_DEPTH='y',
        USER_BUSYBOX_FOLD='y',
        # gencat XXX
        # getconf XXX
        # getopts (shell)
        USER_BUSYBOX_GREP='y',
        USER_BUSYBOX_FEATURE_GREP_CONTEXT='y',
        # hash (shell)
        USER_BUSYBOX_HEAD='y',
        USER_BUSYBOX_FEATURE_FANCY_HEAD='y',
        # iconv XXX
        USER_BUSYBOX_ID='y',
        # join XXX
        USER_BUSYBOX_KILL='y',
        USER_BUSYBOX_LN='y',
        # locale XXX
        # localedef XXX
        USER_BUSYBOX_LOGGER='y',
        USER_BUSYBOX_LOGNAME='y',
        # lp XXX
        USER_BUSYBOX_LS='y',
         USER_BUSYBOX_FEATURE_LS_FILETYPES='y',
         USER_BUSYBOX_FEATURE_LS_FOLLOWLINKS='y',
         USER_BUSYBOX_FEATURE_LS_RECURSIVE='y',
         USER_BUSYBOX_FEATURE_LS_SORTFILES='y',
         USER_BUSYBOX_FEATURE_LS_TIMESTAMPS='y',
         USER_BUSYBOX_FEATURE_LS_USERNAME='y',
        # m4 XXX
        # mailx XXX
        USER_BUSYBOX_MAN='y',
        USER_BUSYBOX_MESG='y',
        USER_BUSYBOX_MKDIR='y',
        USER_BUSYBOX_MKFIFO='y',
        USER_BUSYBOX_MV='y',
        USER_BUSYBOX_NICE='y',
        # nohup (shell)
        USER_BUSYBOX_OD='y',
        USER_BUSYBOX_PASTE='y',
        # pathchk XXX
        # pax XXX
        # pr XXX
        USER_BUSYBOX_PRINTF='y',
        USER_BUSYBOX_PS='y',
        USER_BUSYBOX_PWD='y',
        # read (shell)
        USER_BUSYBOX_RENICE='y',
        USER_BUSYBOX_RM='y',
        USER_BUSYBOX_RMDIR='y',
        USER_BUSYBOX_SED='y',
        # sh (shell)
        USER_BUSYBOX_SLEEP='y',
        USER_BUSYBOX_SORT='y',
        USER_BUSYBOX_SPLIT='y',
        USER_BUSYBOX_STRINGS='y',
        USER_BUSYBOX_STTY='y',
        # tabs XXX
        USER_BUSYBOX_TAIL='y',
        USER_BUSYBOX_FEATURE_FANCY_TAIL='y',
        USER_BUSYBOX_TEE='y',
        USER_BUSYBOX_TEST='y',
        USER_BUSYBOX_TIME='y',
        USER_BUSYBOX_TOUCH='y',
        # tput XXX
        USER_BUSYBOX_TR='y',
        USER_BUSYBOX_TRUE='y',
        # tsort XXX
        USER_BUSYBOX_TTY='y',
        # umask (shell)
        # unalias (shell)
        USER_BUSYBOX_UNAME='y',
        USER_BUSYBOX_UNEXPAND='y',
        USER_BUSYBOX_UNIQ='y',
        USER_BUSYBOX_UUDECODE='y',
        USER_BUSYBOX_UUENCODE='y',
        # wait (shell)
        USER_BUSYBOX_WC='y',
        # write XXX
        USER_BUSYBOX_XARGS='y',
        # misc utilities
        USER_BUSYBOX_LESS='y',
    ),
)

# Squashfs rootfd/initrd support
groups['opengear/squashfs'] = dict(
    linux = dict(
        MISC_FILESYSTEMS='y',
         SQUASHFS='y',
          SQUASHFS_XZ='y',              # xz compression is the default
          SQUASHFS_CRAMFS_MAGIC='y',
        BLK_DEV_INITRD='y',
         RD_XZ='y',
         XZ_DEC='y',
        KERNEL_XZ='y',
    )
)

# Developer on-target CLI tools, not required by any application
groups['opengear/ogcs-dev'] = dict(
    user = dict(
        USER_STRACE='y',
        USER_BUSYBOX_HEXDUMP='y',
    )
)

# OGCS application features (common/defaults across all products)
groups['opengear/ogcs-common'] = dict(
    include = [
        'opengear/minimum',
        'opengear/busybox-posix',
        'opengear/all-usb-serial',  # (assumes all products have USB)
        'opengear/ogcs-dev',        # for on-target development
        'opengear/linux_iptables',
        'opengear/linux_ip6tables',
    ],
    linux = dict(
        # enable LED control framework
        NEW_LEDS='y',
        LEDS_CLASS='y',
         LEDS_TRIGGERS='y',
          LEDS_TRIGGER_DEFAULT_ON='y',
          LEDS_TRIGGER_HEARTBEAT='y',
          LEDS_TRIGGER_TIMER='y',

        # supported filesystems we would find on USB storage
        EXT3_FS='y',
        VFAT_FS='y',
        EXFAT_FS='y',

        # linux features
        # (dependencies of user-space tools?)
        EVENTFD='y',            # XXX do we use this?
        FILE_LOCKING='y',       # flock()
        INOTIFY_USER='y',
        POSIX_TIMERS='y',
        PROC_SYSCTL='y',
        FUTEX='y',              # pthreads
        SIGNALFD='y',           # signalfd()
        EPOLL='y',              # used by udevd

        # support for some advanced networking features
        IP_ADVANCED_ROUTER='y',
         IP_MULTIPLE_TABLES='y',   # policy routing
         IPV6_MULTIPLE_TABLES='y',
        IP_MULTICAST='y',
        PACKET='y',             # for tcpdump
        NET_CORE='y',
         TUN='y',
        BRIDGE='y',
         VLAN_8021Q='y',
         BRIDGE_VLAN_FILTERING='y',

#       USB_ACM='y',

        IP_DEF_RT_METRIC='y',
    ),

    user = dict(
        # basic login
        PROP_OGCS_LOGIN_LOGIN='y',
         PROP_OGCS_LOGIN_PAM='y',
         PROP_OGCS_LOGIN_PORTMANAGER='y',
         PROP_OGCS_LOGIN_TTY='y',
        USER_PAM_RADIUS='y',
        USER_PAM_PORTAUTH='y',

        # authentication
        LIB_LIBPAM='y',
         LIB_LIBPAM_PAM_PERMIT='y',
        LIB_LIBKRB5='y',
         USER_PAM_KRB5='y',
        LIB_LIBLDAP='y',
         USER_PAM_LDAP='y',
        LIB_LIBDNSRES='y',
         USER_PAM_TACACS='y',

        # standard libraries
        LIB_INSTALL_LIBGCC_S='y',
        LIB_INSTALL_LIBSTDCPLUS='y',

        LIB_DBUS='y',
        LIB_DBUS_GLIB='y',
        LIB_GLIB='y',
         LIB_LIBFFI='y',                            # needed by glib

        # bash
        USER_BASH_BASH='y',
        USER_BASH_SH='y',                           # /bin/sh -> bash
        USER_BUSYBOX_SH_IS_NONE='y',
        USER_BUSYBOX_BASH_IS_NONE='y',

        USER_BUSYBOX='y',
#         USER_BUSYBOX_PAM='y',                     # PAM support in login
          USER_BUSYBOX_LONG_OPTS='y',               # GNU-style longopts
           USER_BUSYBOX_FEATURE_DIFF_LONG_OPTIONS='y',
          USER_BUSYBOX_FEATURE_DATE_NANO='y',       # compat with hwclock
          USER_BUSYBOX_FEATURE_MOUNT_NFS='y',       # for nfs mounts
          USER_BUSYBOX_FEATURE_DIFF_DIR='y',
          USER_BUSYBOX_MONOTONIC_SYSCALL='y',       # tolerate clock adjusts
          USER_BUSYBOX_ADDGROUP='y',                # used by migrate script
           USER_BUSYBOX_FEATURE_ADDUSER_TO_GROUP='y',
          USER_BUSYBOX_FEATURE_SHADOWPASSWDS='y',   # defines HAVE_SHADOW

        # cron is used for periodic tasks
        USER_BUSYBOX_CRONTAB='y',
        USER_BUSYBOX_CROND='y',

        # OpenGear applications
        PROP_OGCS='y',
         PROP_OGCS_CONFIG='y',
         PROP_OGCS_INFOD='y',
         PROP_OGCS_PERIFROUTED='y',
         PROP_OGCS_CONMAN='y',
         PROP_OGCS_PSMON='y',
         PROP_OGCS_ARD='y',
         PROP_OGCS_CGI='y',
          USER_CHEROKEE='y',
         PROP_OGCS_LH5_NODE='y',
        # PROP_OGCS_PORTMANAGER='y',

        # command utilities that aren't covered by POSIX
        USER_BUSYBOX_FEATURE_IPV6='y',
        USER_BUSYBOX_CHROOT='y',
        USER_BUSYBOX_CLEAR='y',
        USER_BUSYBOX_DF='y',            # XSI
#        USER_BUSYBOX_FEATURE_SKIP_ROOTFS='y', # XXX buggy!?
        USER_BUSYBOX_DMESG='y',
#       USER_BUSYBOX_EGREP='y',       # obsolete
#       USER_BUSYBOX_FGREP='y',       # obsolete
        USER_BUSYBOX_FLOCK='y',
        USER_BUSYBOX_FREE='y',
        USER_BUSYBOX_FSYNC='y',
        USER_BUSYBOX_GETOPT='y',
        USER_BUSYBOX_FEATURE_GETOPT_LONG='y',   # getopt -l option
        USER_BUSYBOX_GROUPS='y',
        USER_BUSYBOX_GUNZIP='y',
        USER_BUSYBOX_GZIP='y',
        USER_BUSYBOX_HALT='y',
        USER_BUSYBOX_HOSTNAME='y',
        USER_BUSYBOX_IOSTAT='y',
        USER_BUSYBOX_KILLALL='y',
        USER_BUSYBOX_LSOF='y',
        USER_BUSYBOX_MD5SUM='y',
        USER_BUSYBOX_MKNOD='y',
        USER_BUSYBOX_MKTEMP='y',
        USER_BUSYBOX_MORE='y',          # posix portable
        USER_BUSYBOX_NSLOOKUP='y',
        USER_BUSYBOX_PGREP='y',
        USER_BUSYBOX_PIDOF='y',
        USER_BUSYBOX_PIVOT_ROOT='y',
        USER_BUSYBOX_PKILL='y',
        USER_BUSYBOX_POWEROFF='y',
        USER_BUSYBOX_READLINK='y',
        USER_BUSYBOX_REALPATH='y',
        USER_BUSYBOX_REBOOT='y',
        USER_BUSYBOX_RESET='y',
        USER_BUSYBOX_SEQ='y',
        USER_BUSYBOX_RESIZE='y',
         USER_BUSYBOX_FEATURE_RESIZE_PRINT='y',
        USER_BUSYBOX_SETSID='y',
        USER_BUSYBOX_SHA1SUM='y',
        USER_BUSYBOX_STAT='y',
        USER_BUSYBOX_SYNC='y',
        USER_BUSYBOX_TAR='y',
        USER_BUSYBOX_TOP='y',
        USER_BUSYBOX_ARPING='y',    # needed by network-connection-wan-dupe-detection conn
        USER_BUSYBOX_UDHCPC='y',
        USER_BUSYBOX_UMOUNT='y',
        USER_BUSYBOX_UPTIME='y',
        USER_BUSYBOX_USLEEP='y',
        USER_BUSYBOX_VI='y',
         USER_BUSYBOX_FEATURE_VI_8BIT='y',
         USER_BUSYBOX_FEATURE_VI_UNDO='y',
         USER_BUSYBOX_FEATURE_VI_SEARCH='y',
         USER_BUSYBOX_FEATURE_VI_READONLY='y',
        USER_BUSYBOX_WATCHDOG='y',
        #USER_BUSYBOX_WGET='y',     # we'll use curl instaed
        USER_BUSYBOX_WHICH='y',
        USER_BUSYBOX_WHOAMI='y',
        USER_BUSYBOX_WHOIS='y',
        USER_BUSYBOX_YES='y',

        # system tools
        USER_UDEV='y',
        USER_LIBGUDEV='y',          # libgudev required by ModemManager

        USER_KMOD='y',
        USER_KMOD_LIBKMOD='y',      # required by USER_UDEV
        USER_KMOD_TOOLS='y',        # modprobe etc tools

        USER_UTIL_LINUX='y',
        USER_UTIL_LINUX_LIBBLKID='y', # required by USER_UDEV
        USER_UTIL_LINUX_LIBUUID='y',  # required by USER_UDEV

        # basic networking
        USER_IPROUTE2='y',
        USER_IPROUTE2_IP_IP='y',
        USER_IPROUTE2_TC_TC='y',

        USER_SSH_SSH='y',
        USER_SSH_SSHD='y',
        USER_SSH_SCP='y',
        USER_SSH_SSHKEYGEN='y',

        USER_TCPDUMP_TCPDUMP='y',
        USER_STUNNEL_STUNNEL='y',   # XXX no longer used?
        USER_OPENRESOLV='y',

        USER_SIGS_SIGS='y',

        # net-tools
        USER_NET_TOOLS='y',
        USER_NET_TOOLS_ARP='y',
        USER_NET_TOOLS_HOSTNAME='y',
        USER_NET_TOOLS_IFCONFIG='y',
        USER_NET_TOOLS_NETSTAT='y',
        USER_NET_TOOLS_ROUTE='y',
        USER_NET_TOOLS_MII_TOOL='y',

        # iputils
        USER_IPUTILS_IPUTILS='y',
        USER_IPUTILS_PING='y',
        USER_IPUTILS_PING6='y',
        USER_IPUTILS_TRACEROUTE6='y',

        # iptables
        USER_IPTABLES_IPTABLES='y',     # avoid --disable-ipv4
        USER_IPTABLES_IP6TABLES='y',    # avoid --disable-ipv6

        # logging
        USER_RSYSLOG_RSYSLOGD='y',

        # XXX we don't actually use flatfsd; the following
        # is simply the easiest way to get --confdir=/etc/config
        # everywhere :(
        USER_FLATFSD_FLATFSD='y',
         USER_FLATFSD_ETC_CONFIG='y',
         USER_FLATFSD_EXTERNAL_INIT='y',

        # PROP_CONFIG_SERIAL_EXCLUSIVE_ACCESS='y',

        USER_IFMETRIC='y',
        USER_FTP_FTP_FTP='y',

        LIB_LIBUSB='y',
        USER_USBUTILS='y',              # lsusb

        USER_OPENSSL_APPS='y',          # openssl

        USER_BRCTL_BRCTL='y',           # brctl
    ),
)

# ------------------------------------------------------------
# Common hardware 'component' configurations

# Support for a large number of serial uarts
groups['opengear/8250-serial'] = dict(
    linux = dict(
        SERIAL_8250='y',
         SERIAL_8250_NR_UARTS='128',
         SERIAL_8250_RUNTIME_UARTS='128',
    ),
)

# Exar's PCI UARTs are used in most Opengear products
groups['opengear/xr-pci-serial'] = dict(
    include = [
        'opengear/8250-serial',
    ],
    linux = dict(
        PCI='y',
         SERIAL_8250_PCI='y',
          SERIAL_8250_EXAR='y',
    ),
)

# How ARM-based Opengear bootloaders usually talk to the kernel
groups['opengear/arm-common'] = dict(
    include = [
        'opengear/squashfs',
    ],
    linux = dict(
        MMU='y',                # dolce vita
        MTD='y',

        KUSER_HELPERS='y',      # (should default y already but doesn't!?)

        # The U-Boot loaders all use atags to pass parameters
        ATAGS='y',
        ARM_APPENDED_DTB='y',
         ARM_ATAG_DTB_COMPAT='y',
        SERIAL_OF_PLATFORM='y',

        XZ_DEC_ARM='y',         # arm-optimised XZ decompress
        DEBUG_UNCOMPRESS='y',   # show messages during decompress

        MTD_BLOCK_RO='y',       # BLs pass root=mtdblock10 etc
        MTD_OF_PARTS='y',       # devicetree defines partition table
    ),
)

# Opengear products with UBI on NAND flash
# (ACM700x, ACM7004-5, CM7196)
groups['opengear/ubi'] = dict(
    linux = dict(
        MTD='y',
         MTD_UBI='y',
         MTD_UBI_GLUEBI='y',    # makes mtd partitions
        MISC_FILESYSTEMS='y',
         UBIFS_FS='y',
    ),
    user = dict(
        USER_MTD_UTILS='y',
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
         USER_MTD_UTILS_MKFSUBIFS='y',
    ),
)

# SFP cage support
groups['opengear/sfp'] = dict(
    linux = dict(
        NETDEVICES='y',
         PHYLIB='y',
          SFP='y',
    ),
    user = dict(
         #PROP_SFPINFO='y'          # TODO: enable
    ),
)

# internal cellular modems
groups['opengear/usb-cellmodem'] = dict(
    linux = dict(
        USB='y',
        USB_PCI='y',
        USB_ANNOUNCE_NEW_DEVICES='y',

        NETDEVICES='y',
        NET_CORE='y',

        USB_EHCI_HCD='y',
        USB_EHCI_ROOT_HUB_TT='y',
        USB_EHCI_TT_NEWSCHED='y',
        USB_EHCI_PCI='y',
        USB_EHCI_HCD_ORION='y',
        USB_EHCI_HCD_PLATFORM='y',

        USB_USBNET='y',
        USB_NET_DRIVERS='y',
        USB_NET_AX8817X='m',
        USB_NET_AX88179_178A='m',
        USB_NET_CDCETHER='m',
        USB_NET_CDC_SUBSET='m',
        USB_NET_CDC_MBIM='m',
        USB_NET_CDC_NCM='m',
        USB_NET_HUAWEI_CDC_NCM='m',
        USB_NET_QMI_WWAN='m',

        # USB Device Class drivers
        USB_ACM='m',
        USB_WDM='y',

        # USB port drivers
        USB_SERIAL_QUALCOMM='m',    # qcserial.ko
    ),
    user = dict(
        PROP_OGCS_CELLCTL='y',

        USER_LIBQMI='y',
        USER_LIBQMI_QMICLI='y',
        USER_LIBQMI_QMI_FIRMWARE_UPDATE='y',

        USER_MODEMMANAGER='y',
        USER_MODEMMANAGER_PLUGIN_SIERRA='y',
        USER_MODEMMANAGER_PLUGIN_HUAWEI='y',

        USER_PPPD_PPPD_PPPD='y',
        USER_CHAT_CHAT='y',         # modem-cmd uses /bin/chat
    ),
)

# All the USB serial devices that Linux can stomach.
# We do this mainly because the IM72xx-24U product
# is advertised to work with all the USB UARTs that
# linux knows about.
groups['opengear/all-usb-serial'] = dict(
    linux = dict(
        USB='y',
        USB_SERIAL='m',
        USB_SERIAL_GENERIC='y',
        USB_SERIAL_AIRCABLE='m',
        USB_SERIAL_ARK3116='m',
        USB_SERIAL_BELKIN='m',
        USB_SERIAL_CH341='m',
        USB_SERIAL_CP210X='m',
        USB_SERIAL_CYBERJACK='m',
        USB_SERIAL_CYPRESS_M8='m',
        USB_SERIAL_DIGI_ACCELEPORT='m',
        USB_SERIAL_EDGEPORT='m',
        USB_SERIAL_EDGEPORT_TI='m',
        USB_SERIAL_EMPEG='m',
        USB_SERIAL_F81232='m',
        USB_SERIAL_F8153X='m',
        USB_SERIAL_FTDI_SIO='m',
        USB_SERIAL_GARMIN='m',
        USB_SERIAL_IPAQ='m',
        USB_SERIAL_IPW='m',
        USB_SERIAL_IR='m',
        USB_SERIAL_IUU='m',
        USB_SERIAL_KEYSPAN='m',
        USB_SERIAL_KEYSPAN_PDA='m',
        USB_SERIAL_KLSI='m',
        USB_SERIAL_KOBIL_SCT='m',
        USB_SERIAL_MCT_U232='m',
        USB_SERIAL_METRO='m',
        USB_SERIAL_MOS7720='m',
        USB_SERIAL_MOS7840='m',
        USB_SERIAL_MXUPORT='m',
        USB_SERIAL_NAVMAN='m',
        USB_SERIAL_OMNINET='m',
        USB_SERIAL_OPTICON='m',
        USB_SERIAL_OPTION='m',
        USB_SERIAL_OTI6858='m',
        USB_SERIAL_PL2303='m',
        USB_SERIAL_QCAUX='m',
        USB_SERIAL_QT2='m',
        USB_SERIAL_SAFE='m',
        USB_SERIAL_SIERRAWIRELESS='m',
        USB_SERIAL_SIMPLE='m',
        USB_SERIAL_SPCP8X5='m',
        USB_SERIAL_SSU100='m',
        USB_SERIAL_SYMBOL='m',
        USB_SERIAL_TI='m',
        USB_SERIAL_UPD78F0730='m',
        USB_SERIAL_VISOR='m',
        USB_SERIAL_WHITEHEAT='m',
        USB_SERIAL_WISHBONE='m',
        USB_SERIAL_WWAN='m',
        USB_SERIAL_XSENS_MT='m',
    ),
)

# ------------------------------------------------------------
# Processors / SoCs

# generic AMD64/X86_64 minimum config
groups['opengear/x86_64'] = dict(
    include = [
        'opengear/minimum',
    ],
    linux = dict(
        GENERIC_CPU='y',
        ACPI='y',
        # 64_BIT='y',           # (defined below)
        X86_64='y',
        # Compatibility with i386
        IA32_EMULATION='y',     # permits UID32 and i386 binaries
        X86_X32_ABI='n',
    ),
)
groups['opengear/x86_64']['linux']['64BIT'] = 'y'   # where is your god now

# Common features available on all Armada 370 SoC boards
groups['opengear/soc/armada_370'] = dict(
    include = [
        'opengear/arm-common',
    ],
    linux = dict(
        ARCH_MULTIPLATFORM='y',
         ARCH_MULTI_V7='y',
         ARCH_MVEBU='y',
          MACH_ARMADA_370='y',
        CACHE_FEROCEON_L2='y',
        VFP='y',                                        # VFP3-D32
         NEON='y',
          KERNEL_MODE_NEON='y',
        #IWMMXT='y',
        ARM_PATCH_IDIV='y',                             # performance
        ARM_ERRATA_754322='y',                          # XXX why?
        PJ4B_ERRATA_4742='y',                           # XXX why?
        ARMADA_THERMAL='y',
        CPU_IDLE='y',
         ARM_MVEBU_V7_CPUIDLE='y',
        I2C='y',
         I2C_MV64XXX='y',                               # 0xd0011000
        WATCHDOG='y',
         ORION_WATCHDOG='y',
        PCI='y',
         PCI_MVEBU='y',
        RTC_CLASS='y',
         RTC_DRV_MV='y',
        USB_SUPPORT='y',
         USB='y',
          USB_EHCI_HCD='y',                             # USB 2.0
           USB_EHCI_HCD_ORION='y',
          USB_MARVELL_ERRATA_FE_9049667='y',
        ETHERNET='y',
         NET_VENDOR_MARVELL='y',
         MVNETA='y',
         DEBUG_INFO_NONE='y',

        # For early debug only:
        DEBUG_LL='y',
         DEBUG_MVEBU_UART0='y',
        DEBUG_MEMORY_INIT='y',

        SPI='y',
         SPI_ORION='y',
         SPI_SPIDEV='y',                                # /dev/spi0
    ),
    user = dict(
        USER_BUSYBOX_LSPCI='y',
    ),
)

groups['opengear/soc/armada_310'] = dict(
    include = [
        'opengear/arm-common',
    ],
    linux = dict(
        ARCH_MULTIPLATFORM='y',
         ARCH_MULTI_V7='n',
         ARCH_MVEBU='y',
          MACH_KIRKWOOD='y',
#       SPI='y',
#        SPI_ORION='y',
#        SPI_SPIDEV='y',                                # /dev/spi0
        I2C='y',
         I2C_MV64XXX='y',                               # 0xd0011000
        WATCHDOG='y',
         ORION_WATCHDOG='y',
        PCI='y',
         PCI_MVEBU='y',
        RTC_CLASS='y',
         RTC_DRV_MV='y',
        USB_SUPPORT='y',
         USB='y',
          USB_EHCI_HCD='y',                             # USB 2.0
           USB_EHCI_HCD_ORION='y',
        ETHERNET='y',
         NET_VENDOR_MARVELL='y',
         MVNETA='y',

        # For early debug only:
        DEBUG_LL='y',
#         DEBUG_UART_NONE='y',
    ),
)

#------------------------------------------------------------
# Boards

# Common config for ACM700x and ACM7004-5
groups['opengear/acm7000'] = dict(
    include = [
        'opengear/soc/armada_370',
        'opengear/ubi',
        'opengear/sfp',
        'opengear/usb-cellmodem',
        'opengear/xr-pci-serial',
    ],
    linux = dict(
        SENSORS_LM75='y',
        GPIO_WATCHDOG='y',
        GPIO_PCA953X='y',
        NETDEVICES='y',
         PHYLIB='y',
          MARVELL_PHY='y',
        LEDS_GPIO='y',

        # console debug
        SERIAL_8250_CONSOLE='y',

        MTD_SPI_NOR='y',
         MTD_SPI_NOR_USE_4K_SECTORS='y',

        # MLC NAND flash
        MTD_RAW_NAND='y',
         MTD_NAND_MARVELL='y',
         MTD_NAND_ECC_SW_BCH='y',
        MTD_CFI='y',
        # MTD_CFI_INTELEXT='y',
        # MTD_CFI_AMDSTD='y',
        # MTD_CFI_STAA='y',

        # enable snapdog because gpio-wdt is too slow to start
        SNAPDOG='y',
        MACH_ACM700x='y',
    ),
)

# ACM7004, ACM7008
groups['opengear/acm700x (309047)'] = dict(
    include = [
        'opengear/acm7000',
    ],
    linux = dict(
        # SERIAL_XR20M117X='y',             # XXX todo, extend xr20m12 driver!
    ),
)

# ACM7004-5
groups['opengear/acm7004-5 (309054)'] = dict(
    include = [
        'opengear/acm7000',
    ],
    linux = dict(
        FIXED_PHY='y',
        NET_DSA='y',
        NET_DSA_MV88E6XXX='y',
    ),
    user=dict(
        PROP_SWTEST_SWTEST='y',
        PROP_SWTEST_MVL88E6350='y',
    ),
)

groups['opengear/cm7100 (309044)'] = dict(
    include = [
        'opengear/soc/armada_370',
        'opengear/xr-pci-serial',
    ],
    linux = dict(
        GPIO_WATCHDOG='y',
        LEDS_GPIO='y',
        NETDEVICES='y',
        PHYLIB='y',
        MARVELL_PHY='y',
    ),
)

groups['opengear/cm7196 (309056)'] = dict(
    include = [
        'opengear/soc/armada_370',
        'opengear/ubi',
        'opengear/sfp',
        'opengear/xr-pci-serial',
    ],
    linux = dict(
        SENSORS_LM75='y',
        GPIO_WATCHDOG='y',
        GPIO_PCA953X='y',
        # NETDEVICES='y',
        # PHYLIB='y',
        # MARVELL_PHY='y',
        LEDS_GPIO='y',
        CRYPTO='y',
        CRYPTO_HW='y',
        CRYPTO_DEV_ATMEL_SHA204A='y',
    ),
)

groups['opengear/im7200 (309037)'] = dict(
    include = [
        'opengear/soc/armada_310',
        'opengear/usb-cellmodem',
        'opengear/xr-pci-serial',
    ],
    linux = dict(
        SENSORS_LTC4151='y',
        GPIO_WATCHDOG='y',
        GPIO_PCA953X='y',
        NETDEVICES='y',
        PHYLIB='y',
        MARVELL_PHY='y',
        LEDS_GPIO='y',
        USB_SUPPORT='y',
        USB='y',
        USB_XHCI_HCD='y',
        USB_XHCI_PCI_RENESAS='y',
    ),
    user=dict(
        PROP_SWTEST_SWTEST='y',
        PROP_SWTEST_MVL88E6390='y',
    ),
)

# Enable drivers for QEMU's default piix machine
groups['opengear/x86/qemu-pc-i440fx'] = dict(
    linux = dict(
        VIRTIO_MENU='y',
        VIRTIO_CONSOLE='y',     # virtual console support
        VIRTIO_BALLOON='y',     # virtio-balloon-device
        PCI='y',                # pci-host-bridge
         VIRTIO_PCI='y',
        VIRTIO_BLK='y',
        ATA='y',                # qemu -hda ...
         ATA_SFF='y',
          ATA_BMDMA='y',
           ATA_PIIX='y',
        BLK_DEV_SD='y',         # /dev/sda ...

        SERIAL_8250='y',        # qemu -serial
         SERIAL_8250_CONSOLE='y',
         SERIAL_8250_PCI='y',   # -device pci-serial-4x

        USB_SUPPORT='y',        # virtual USB
        USB='y',
         USB_XHCI_HCD='y',
          USB_XHCI_PLATFORM='y',
         USB_EHCI_HCD='y',
          USB_EHCI_HCD_PLATFORM='y',

        RTC_CLASS='y',
         RTC_DRV_CMOS='y',

        NETDEVICES='y',
         VIRTIO_NET='y',         # virtio-net-device
    ),
)

# VACM is a guest for x86_64 virtual machines mainly used
# to test the networking/logic part of the OGCS product.
# We don't expect much "high fidelity" in the hardware emulation,
# so it is okay to use virtio-style devices.

groups['opengear/vacm'] = dict(
    include = [
        'opengear/x86_64',      # XXX should be 32-bit to match ACMs :(
        'opengear/squashfs',
        'opengear/minimum',
        'opengear/x86/qemu-pc-i440fx',
        'opengear/ogcs-common',
        'opengear/8250-serial',
    ],
    linux = dict(
        RELOCATABLE='n',        # fsk86 loads bzImage correctly
        XZ_DEC_X86='y',         # use x86-optimised xz decompressor

        # XXX debugging
        DEBUG_KERNEL='y',
        DEBUG_INFO_NONE='n',         # build with debug syms
        DEBUG_INFO_DWARF5='y',
        PANIC_TIMEOUT=0,        # hang on crash
    ),
    user = dict(
        USER_FLATFSD_DISKLIKE='y',
        USER_NETFLASH_VM_NETFLASH='y',
    ),
)

#------------------------------------------------------------
# Products

products['OpenGear ACM700x'] = dict(
    vendor = 'OpenGear',
    product = 'ACM700x',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20191126',
    libc = 'musl',
    include = [
        'opengear/acm700x (309047)',
        'opengear/ogcs-common',
    ],
)

products['OpenGear ACM7004-5'] = dict(
    vendor = 'OpenGear',
    product = 'ACM7004-5',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20191126',
    libc = 'musl',
    include = [
        'opengear/acm7004-5 (309054)',
        'opengear/ogcs-common',
    ],
)

products['OpenGear CM71xx'] = dict(
    vendor = 'OpenGear',
    product = 'CM71xx',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20191126',
    libc = 'musl',
    include = [
        'opengear/cm7100 (309044)',
        'opengear/ogcs-common',
    ],
)

products['OpenGear CM7196'] = dict(
    vendor = 'OpenGear',
    product = 'CM7196',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20191126',
    libc = 'musl',
    include = [
        'opengear/cm7196 (309056)',
        'opengear/ogcs-common',
    ],
)

products['OpenGear IM72xx'] = dict(
    vendor = 'OpenGear',
    product = 'IM72xx',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20191126',
    libc = 'musl',
    include = [
        'opengear/im7200 (309037)',
        'opengear/ogcs-common',
    ],
)

products['OpenGear VACM'] = dict(
    vendor = 'OpenGear',
    product = 'VACM',
    arch = 'x86',
    tools = 'x86_64-linux-musl-20191112',
    libc = 'musl',
    include = [
        'opengear/vacm',
        'opengear/ogcs-common',
    ],
)

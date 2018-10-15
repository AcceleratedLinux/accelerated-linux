
URL = https://mirrors.edge.kernel.org/pub/linux/utils/util-linux/v2.28/util-linux-2.28.2.tar.gz
MAKEVARS = 

CONFOPTS += $(if $(CONFIG_USER_UTIL_LINUX_LIBUUID),--enable-libuuid,--disable-libuuid)
CONFOPTS += $(if $(CONFIG_USER_UTIL_LINUX_LIBBLKID),--enable-libblkid,--disable-libblkid)
CONFOPTS += $(if $(CONFIG_USER_UTIL_LINUX_LIBMOUNT),--enable-libmount,--disable-libmount)
CONFOPTS += $(if $(CONFIG_LIB_NCURSES),--with-ncurses,--without-ncurses)

# bring these options up above as needed,  suggest using busybox version if
# possible

# CONFOPTS += --enable-deprecated-mount
CONFOPTS += --disable-nls
CONFOPTS += --disable-mount
CONFOPTS += --disable-losetup
CONFOPTS += --disable-fsck
CONFOPTS += --disable-partx
CONFOPTS += --disable-uuidd
CONFOPTS += --disable-mountpoint
CONFOPTS += --disable-fallocate
CONFOPTS += --disable-unshare
CONFOPTS += --disable-arch
CONFOPTS += --disable-ddate
CONFOPTS += --disable-eject
CONFOPTS += --disable-agetty
CONFOPTS += --disable-cramfs
CONFOPTS += --disable-wdctl
CONFOPTS += --disable-switch_root
CONFOPTS += --disable-pivot_root
CONFOPTS += --disable-elvtune
CONFOPTS += --disable-kill
CONFOPTS += --disable-last
CONFOPTS += --disable-utmpdump
CONFOPTS += --disable-line
CONFOPTS += --disable-mesg
CONFOPTS += --disable-raw
CONFOPTS += --disable-rename
CONFOPTS += --disable-reset
CONFOPTS += --disable-vipw
CONFOPTS += --disable-newgrp
CONFOPTS += --disable-chfn-chsh
# CONFOPTS += --disable-chsh-only-listed
CONFOPTS += --disable-login
# CONFOPTS += --enable-login-chown-vcs
# CONFOPTS += --enable-login-stat-mail
CONFOPTS += --disable-sulogin
CONFOPTS += --disable-su
CONFOPTS += --disable-schedutils
CONFOPTS += --disable-wall
CONFOPTS += --disable-write
CONFOPTS += --disable-chkdupexe
# CONFOPTS += --enable-socket-activation
# CONFOPTS += --disable-pg-bell
# CONFOPTS += --disable-require-password
# CONFOPTS += --enable-fs-paths-default=paths
# CONFOPTS += --enable-fs-paths-extra=paths
# CONFOPTS += --disable-use-tty-group
# CONFOPTS += --disable-makeinstall-chown
# CONFOPTS += --disable-makeinstall-setuid

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	[ ! -d build/util-linux-2.28.2-install/lib ] || \
		$(ROMFSINST) -v -f build/util-linux-2.28.2-install/lib /lib


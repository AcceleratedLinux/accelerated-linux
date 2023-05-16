VERSION = 2.37.2
URL = https://mirrors.edge.kernel.org/pub/linux/utils/util-linux/v$(basename $(VERSION))/util-linux-$(VERSION).tar.xz
MAKEVARS = 

AUTORECONF = autoreconf -i

CONFOPTS += $(if $(CONFIG_USER_UTIL_LINUX_LIBUUID),--enable-libuuid,--disable-libuuid)
CONFOPTS += $(if $(CONFIG_USER_UTIL_LINUX_LIBBLKID),--enable-libblkid,--disable-libblkid)
CONFOPTS += $(if $(CONFIG_USER_UTIL_LINUX_LIBMOUNT),--enable-libmount,--disable-libmount)
CONFOPTS += $(if $(CONFIG_USER_UTIL_LINUX_UUIDD),--enable-uuidd,--disable-uuidd)
CONFOPTS += $(if $(CONFIG_LIB_NCURSES),--with-ncurses --without-ncursesw --disable-widechar,--without-ncurses)
CONFOPTS += $(if $(CONFIG_USER_UTIL_LINUX_FALLOCATE),--enable-fallocate,--disable-fallocate)
CONFOPTS += $(if $(CONFIG_USER_UTIL_LINUX_LOSETUP),--enable-losetup,--disable-losetup)
CONFOPTS += $(if $(CONFIG_USER_UTIL_LINUX_SWITCH_ROOT),--enable-switch_root,--disable-switch_root)

# bring these options up above as needed,  suggest using busybox version if
# possible

# CONFOPTS += --enable-deprecated-mount
CONFOPTS += --disable-nls
CONFOPTS += --disable-mount
CONFOPTS += --disable-fsck
CONFOPTS += --disable-partx
CONFOPTS += --disable-mountpoint
CONFOPTS += --disable-unshare
CONFOPTS += --disable-arch
CONFOPTS += --disable-ddate
CONFOPTS += --disable-eject
CONFOPTS += --disable-agetty
CONFOPTS += --disable-cramfs
CONFOPTS += --disable-wdctl
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
	[ ! -d build/util-linux-$(VERSION)-install/lib ] || \
		$(ROMFSINST) -v -f build/util-linux-$(VERSION)-install/lib /lib

	$(ROMFSINST) -e CONFIG_USER_UTIL_LINUX_FALLOCATE build/util-linux-$(VERSION)-install/bin/fallocate /bin/fallocate
	$(ROMFSINST) -e CONFIG_USER_UTIL_LINUX_LOSETUP build/util-linux-$(VERSION)-install/sbin/losetup /sbin/losetup
	$(ROMFSINST) -e CONFIG_USER_UTIL_LINUX_FSTRIM build/util-linux-$(VERSION)-install/sbin/fstrim /sbin/fstrim
	$(ROMFSINST) -e CONFIG_USER_UTIL_LINUX_SWITCH_ROOT build/util-linux-$(VERSION)-install/sbin/switch_root /sbin/switch_root

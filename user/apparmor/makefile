AA_VERSION = 2.13
AA_DIR     = apparmor-$(AA_VERSION)

AALIB      = -L../libraries/libapparmor/src/.libs
AALIB     += -lapparmor
AALIB     += $(LIBSTDCPP) -lsupc++
AALIB     += -lpthread
ifdef CONFIG_DEFAULTS_LIBC_UCLIBC
AALIB     += -lintl
endif
AALIB     += -lm
AALIB     += -lgcc_s

CFLAGS += -D_GLIBCXX_HAVE_MBSTATE_T
CFLAGS += -DNEED_REALLOCARRAY=1
CFLAGS += -DNEED_SCANDIRAT=1
CFLAGS += -D__STDC_LIMIT_MACROS=1
CFLAGS += -I$(CURDIR)/build/$(AA_DIR)/libraries/libapparmor/include

MAKEVARS= USE_SYSTEM=1

ifndef CONFIG_USER_APPARMOR_FULL_INSTALL
AUTOMAKE_ROMFS      = my_romfs
INSTALLDIR          = build/$(AA_DIR)-install
MAKEVARS            += DESTDIR=$(CURDIR)/$(INSTALLDIR)
endif

$(AA_DIR)_URL       = https://launchpad.net/apparmor/$(AA_VERSION)/$(AA_VERSION).0/+download/apparmor-$(AA_VERSION).tar.gz
$(AA_DIR)_CONFOPTS += --disable-man-pages
$(AA_DIR)_CONFOPTS += --without-perl
$(AA_DIR)_CONFOPTS += --without-python
$(AA_DIR)_BUILDDIR = $(AA_DIR)/libraries/libapparmor
$(AA_DIR)_SRCDIR   = $(AA_DIR)

binutils_METHOD    = none
binutils_MAKEVARS  = AALIB="$(AALIB)"
binutils_BUILDDIR  = $(AA_DIR)/binutils
binutils_SRCDIR    = $(AA_DIR)
binutils_DEP       = $(AA_DIR)

utils_METHOD       = none
utils_MAKEVARS     = AALIB="$(AALIB)"
utils_BUILDDIR     = $(AA_DIR)/utils
utils_SRCDIR       = $(AA_DIR)
utils_DEP          = $(AA_DIR)

parser_METHOD      = none
parser_MAKEVARS    = AALIB="$(AALIB)"
parser_BUILDDIR    = $(AA_DIR)/parser
parser_SRCDIR      = $(AA_DIR)
parser_DEP         = $(AA_DIR)

profiles_METHOD    = none
profiles_MAKEVARS  = AALIB="$(AALIB)"
profiles_BUILDDIR  = $(AA_DIR)/profiles
profiles_SRCDIR    = $(AA_DIR)
profiles_DEP       = $(AA_DIR)

PKG_y += apparmor-$(AA_VERSION) binutils utils parser profiles

include $(ROOTDIR)/tools/automake.inc

ifndef CONFIG_USER_APPARMOR_FULL_INSTALL

# install a minimal apparmor setup

AA_INST            = build/$(AA_DIR)-install
BINUTILS_INST      = build/binutils-install
UTILS_INST         = build/utils-install
PARSER_INST        = build/parser-install
PROFILES_INST      = build/profiles-install

my_romfs:
	$(ROMFSINST) -d $(INSTALLDIR)/lib/libapparmor.so.1.5.0 /lib/libapparmor.so.1.5.0
	$(ROMFSINST) -s libapparmor.so.1.5.0 /lib/libapparmor.so.1
	$(ROMFSINST) -s libapparmor.so.1.5.0 /lib/libapparmor.so
	$(ROMFSINST) -d $(INSTALLDIR)/etc/apparmor /etc/apparmor
	$(ROMFSINST) -d $(INSTALLDIR)/etc/apparmor /etc/apparmor
	$(ROMFSINST) -d $(INSTALLDIR)/sbin/apparmor_parser /sbin/apparmor_parser
	$(ROMFSINST) -d $(INSTALLDIR)/usr/bin/aa-exec /usr/bin/aa-exec
	$(ROMFSINST) -d $(INSTALLDIR)/usr/sbin/aa-status /usr/sbin/aa-status
	$(ROMFSINST) -s aa-status /usr/sbin/apparmor_status
	$(ROMFSINST) -d $(INSTALLDIR)/usr/share/apparmor /usr/share/apparmor
	$(ROMFSINST) -d $(INSTALLDIR)/usr/share/locale/en_AU /usr/share/locale/en_AU
	$(ROMFSINST) -d $(INSTALLDIR)/usr/share/locale/en_CA /usr/share/locale/en_CA
	$(ROMFSINST) -d $(INSTALLDIR)/usr/share/locale/en_GB /usr/share/locale/en_GB
	$(ROMFSINST) -d $(INSTALLDIR)/lib/apparmor/rc.apparmor.functions /lib/apparmor/apparmor.functions

endif


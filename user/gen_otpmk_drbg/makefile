METHOD = git
URL = https://github.com/nxp-qoriq/cst
VERSION = e9abf79077fc8faf976e9a3c46a38111aa5b2e69

PATCHES=gen_otpmk_drbg-*
MAKEVARS = CC="$(CC) $(CFLAGS)" LD="$(CC) $(CFLAGS)" INSTALL_BINARIES=gen_otpmk_drbg
gen_otpmk_drbg_INSTALLVARS = INSTALL_BINARIES=gen_otpmk_drbg

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) -d build/cst-install/usr/bin/cst/gen_otpmk_drbg /usr/bin/cst/gen_otpmk_drbg

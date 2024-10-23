BASH_VERSION = 5.2.21
URL = https://ftp.gnu.org/pub/gnu/bash/bash-$(BASH_VERSION).tar.gz
DOWNLOADHASH = c8e31bdc59b69aaffc5b36509905ba3e5cbb12747091d27b4b977f078560d5b8

BASH_INSTALL_DIR = build/bash-$(BASH_VERSION)-install
CONFOPTS += --without-bash-malloc

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) $(BASH_INSTALL_DIR)/bin/bash /bin/bash
	$(ROMFSINST) -s bash /bin/sh


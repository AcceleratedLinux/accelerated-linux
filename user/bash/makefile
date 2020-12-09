BASH_VERSION = 5.0
URL = http://ftp.gnu.org/pub/gnu/bash/bash-$(BASH_VERSION).tar.gz
DOWNLOADHASH = b4a80f2ac66170b2913efbfb9f2594f1f76c7b1afd11f799e22035d63077fb4d

BASH_INSTALL_DIR = build/bash-$(BASH_VERSION)-install
CONFOPTS += --without-bash-malloc

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) $(BASH_INSTALL_DIR)/bin/bash /bin/bash
	$(ROMFSINST) -s bash /bin/sh

PATCH_LEVEL = -p0


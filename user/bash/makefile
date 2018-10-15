BASH_VERSION = 4.3
URL = http://ftp.gnu.org/pub/gnu/bash/bash-$(BASH_VERSION).tar.gz

BASH_INSTALL_DIR = build/bash-$(BASH_VERSION)-install

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) $(BASH_INSTALL_DIR)/bin/bash /bin/bash
	$(ROMFSINST) -s bash /bin/sh

PATCH_LEVEL = -p0


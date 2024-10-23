PKG = elf2flt
METHOD = git
URL = https://github.com/uclinux-dev/elf2flt.git

BINUTILS_VERSION = 2.42
BINUTILS_DIR = $(ROOTDIR)/tools/binutils
BINUTILS_BUILD_DIR = $(BINUTILS_DIR)/build/binutils-$(BINUTILS_VERSION)
BINUTILS_INSTALL_DIR = $(BINUTILS_DIR)/build/binutils-$(BINUTILS_VERSION)-install

#
# The elf2flt build relies on some components generated as part of the
# binutils build. These configuration settings point to those.
#
CONFOPTS += --target=$(TARGET)
CONFOPTS += --with-libbfd=$(BINUTILS_BUILD_DIR)/$(TARGET)/bfd/.libs/libbfd.a
CONFOPTS += --with-binutils-include-dir=$(BINUTILS_BUILD_DIR)/include
CONFOPTS += --with-bfd-include-dir=$(BINUTILS_BUILD_DIR)/$(TARGET)/bfd
CONFOPTS += --with-libiberty=$(BINUTILS_BUILD_DIR)/$(TARGET)/libiberty/libiberty.a

#
# The linker mangling install phase of elf2flt needs the binutils linker
# binary to be in the usually installed place. Copy the generated binutils
# ld into place in the el2flt specific install directory.
# 
elf2flt_INSTALLVARS = \
	echo TOOLSPREFIX=$(TOOLSPREFIX) ; \
	mkdir -p $(ROOTDIR)/tools/elf2flt/build/elf2flt-install/$(TOOLSPREFIX)/bin ; \
	cp $(BINUTILS_INSTALL_DIR)$(TOOLSPREFIX)/bin/$(TARGET)-ld $(ROOTDIR)/tools/elf2flt/build/elf2flt-install/$(TOOLSPREFIX)/bin/ ; \
	mkdir -p $(ROOTDIR)/tools/elf2flt/build/elf2flt-install/$(TOOLSPREFIX)/$(TARGET)/bin/ ; \
	cp $(BINUTILS_INSTALL_DIR)$(TOOLSPREFIX)/$(TARGET)/bin/ld $(ROOTDIR)/tools/elf2flt/build/elf2flt-install/$(TOOLSPREFIX)/$(TARGET)/bin/ ;

include $(ROOTDIR)/tools/hostauto.inc


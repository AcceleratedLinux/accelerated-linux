# Makefile to build CMake projects

.EXPORT_ALL_VARIABLES:

CONFIGTARGET ?= cmake
include $(ROOTDIR)/tools/download.inc

CMAKE_OPTS += -DCMAKE_INSTALL_PREFIX= -DCMAKE_SYSTEM_PREFIX_PATH=$(STAGEDIR)

PRECIOUS: $(addprefix build/,$(addsuffix -cmake,$(PKG_y)))

build/%-cmake: build/%-patched
	@echo "CMake $* ..."
	$(AT)mkdir -p build/$($*_BUILDDIR)
	$(AT)cd build/$($*_BUILDDIR); cmake ../$($*_SRCDIR) $(CMAKE_OPTS) $($*_CMAKE_OPTS)
	$(AT)touch $@

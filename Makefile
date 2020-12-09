############################################################################

#
# Makefile -- Top level dist makefile.
#
# Copyright (c) 2001-2007, SnapGear (www.snapgear.com)
# Copyright (c) 2001, Lineo
#

############################################################################
############################################################################
# first ensure we are running with a suitable umask for firmware builds
ifneq ($(shell umask),0022)
.PHONY: all $(MAKECMDGOALS)
all $(MAKECMDGOALS): fixup_umask_for_build
.PHONY: fixup_umask_for_build
fixup_umask_for_build:
	@umask 0022; $(MAKE) $(MAKECMDGOALS)
else
############################################################################
############################################################################
#
# Lets work out what the user wants, and if they have configured us yet
#

.NOTPARALLEL:
ifeq (.config,$(wildcard .config))
all: tools automake subdirs romfs image
else
all: config_error
endif

ROOTDIR = $(shell pwd)

include vendors/config/common/config.arch

############################################################################

DIRS    = $(VENDOR_TOPDIRS) include lib include user

############################################################################
ifeq ($(ANALYZE),1)

# currently only the clang static analyzer is supported so that is all we
# handle
export UCFRONT_ANALYZER=clang

# a list of : or white space seperated directories to scan
export UCFRONT_ANALYZE_PATH=prop

# helper functions for process the scan info
# do not consider reported bugs and eerror just yet
ANALYZE_CLEAN   = rm -rf $(STAGEDIR)/clang/$(1)
ANALYZE_SUMMARY = echo "Producing static analysis results"; \
                   tools/clang/scanreport \
			-e $(ROOTDIR)/tools/clang/scan-exceptions.txt \
			-o $(STAGEDIR)/clang; \
                   rc=$$?; \
                   tar Cczf $(STAGEDIR)/clang \
                        $(IMAGEDIR)/static-analysis.tar.gz .; \
                   exit $$rc
else
ANALYZE_CLEAN = :
ANALYZE_SUMMARY = :
endif
############################################################################

.PHONY: tools
tools: ucfront cksum
	chmod +x tools/romfs-inst.sh tools/modules-alias.sh tools/build-udev-perms.sh

.PHONY: ucfront
ucfront: tools/ucfront/*.c
	$(MAKE) -C tools/ucfront
	ln -sf $(ROOTDIR)/tools/ucfront/ucfront tools/ucfront-gcc
	ln -sf $(ROOTDIR)/tools/ucfront/ucfront tools/ucfront-g++
	ln -sf $(ROOTDIR)/tools/ucfront/ucfront-ld tools/ucfront-ld
	ln -sf $(ROOTDIR)/tools/ucfront/ucfront-env tools/ucfront-env
	ln -sf $(ROOTDIR)/tools/ucfront/jlibtool tools/jlibtool

.PHONY: cksum
cksum: tools/cksum
tools/cksum: tools/sg-cksum/*.c
	$(MAKE) -C tools/sg-cksum
	ln -sf $(ROOTDIR)/tools/sg-cksum/cksum tools/cksum

TOOLSARCHDIR = $(TOOLSPREFIX)/$(CONFIGURE_TOOL)

.PHONY: binutils_only
binutils_only:
	$(MAKE) TARGET=$(CONFIGURE_TOOL) ENDIAN=$(ENDIAN) FLOAT=$(FLOAT) MABI=$(MABI) -C tools/binutils/

binutils_clean:
	$(MAKE) -C tools/binutils/ clean

.PHONY: gcc-pass1_only
gcc-pass1_only:
	$(MAKE) PASS1=1 TARGET=$(CONFIGURE_TOOL) ENDIAN=$(ENDIAN) FLOAT=$(FLOAT) MABI=$(MABI) -C tools/gcc/

.PHONY: gcc-pass2_only
gcc-pass2_only: gcc_clean
	$(MAKE) PASS2=1 TARGET=$(CONFIGURE_TOOL) ENDIAN=$(ENDIAN) FLOAT=$(FLOAT) MABI=$(MABI) -C tools/gcc/

gcc_clean:
	$(MAKE) -C tools/gcc/ clean

.PHONY: toolchain_headers
toolchain_headers:
	$(MAKEARCH_KERNEL) CROSS_COMPILE=$(CONFIGURE_TOOL)- -j$(HOST_NCPU) -C $(LINUXDIR) headers_install
	cp -a $(ROOTDIR)/$(LINUXDIR)/usr/include $(TOOLSARCHDIR)/

.PHONY: toolchain_uClibc
toolchain_uClibc:
	$(MAKE) CROSS_COMPILE=$(CONFIGURE_TOOL)- STAGEDIR=$(TOOLSARCHDIR) -C $(LIBCDIR) install PREFIX=$(TOOLSARCHDIR)/ DEVEL_PREFIX= RUNTIME_PREFIX=
	if [ -f $(LIBCDIR)/lib/libc.so ] ; then \
		sed -e 's/lib\///g' < $(LIBCDIR)/lib/libc.so > $(TOOLSARCHDIR)/lib/libc.so ; \
		sed -e 's/lib\///g' < $(LIBCDIR)/lib/libpthread.so > $(TOOLSARCHDIR)/lib/libpthread.so ; \
	fi

.PHONY: toolchain_musl
toolchain_musl:
	$(MAKE) UCFRONT_ENV= MACHINE=$(MACHINE) CROSS_COMPILE=$(CONFIGURE_TOOL)- musl_only
	cp -a $(ROOTDIR)/$(LIBCDIR)/install/lib/* $(TOOLSARCHDIR)/lib/
	cp -a $(ROOTDIR)/$(LIBCDIR)/install/include/* $(TOOLSARCHDIR)/include/

.PHONY: toolchain_glibc
toolchain_glibc:
	echo Makefile CONFIGURE_OPTS=$(CONFIGURE_OPTS)
	$(MAKE) UCFRONT_ENV= CC=$(CONFIGURE_TOOL)-gcc CXX=$(CONFIGURE_TOOL)-g++ CONFIGURE_OPTS="$(CONFIGURE_OPTS)" TARGETARCH=$(TARGETARCH) STAGEDIR=$(TOOLSARCHDIR) glibc_only
	cp -rp --remove-destination $(ROOTDIR)/$(LIBCDIR)/install/* $(TOOLSARCHDIR)/

.PHONY: gdb_only
gdb_only:
	$(MAKE) TARGET=$(CONFIGURE_TOOL) ENDIAN=$(ENDIAN) FLOAT=$(FLOAT) -C tools/gdb/

gdb_clean:
	$(MAKE) -C tools/gdb/ clean

ifneq ($(ELF2FLT),)
# ELF2FLT only defined for non-MMU targets
TOOLCHAIN_ELF2FLT=toolchain_elf2flt
endif

.PHONY: toolchain_elf2flt
toolchain_elf2flt:
	$(MAKE) TARGET=$(CONFIGURE_TOOL) -C tools/elf2flt/

elf2flt_clean:
	$(MAKE) -C tools/elf2flt/ clean

.PHONY: toolchain_only
toolchain_only: binutils_only gcc-pass1_only toolchain_headers toolchain_$(LIBCDIR) gcc-pass2_only gdb_only $(TOOLCHAIN_ELF2FLT)
	echo "CROSS_COMPILE=$(CONFIGURE_TOOL)-" > .sgbuilt_toolchain
	echo "Toolchain built successfully"

DATE=$(shell date +%Y%m%d)

.PHONY: toolchain_package
toolchain_package:
	mkdir -p $(IMAGEDIR)
	-strip $(TOOLSPREFIX)/bin/*
	-strip $(TOOLSPREFIX)/libexec/gcc/*/*/c*
	-strip $(TOOLSPREFIX)/libexec/gcc/*/*/lt*
	-strip $(TOOLSARCHDIR)/bin/*
	cd $(TOOLSDIR) ; \
	tar --owner=root --group=root -cvzf $(IMAGEDIR)/$(CONFIGURE_TOOL)-$(DATE).tar.gz *
	cd $(IMAGEDIR) ; \
	../bin/mk-disttools-install $(CONFIGURE_TOOL)-$(DATE).tar.gz $(DATE) $(CONFIGURE_TOOL)

toolchain_clean: binutils_clean gcc_clean gdb_clean lib/uClibc_clean lib/musl_clean lib/glibc_clean elf2flt_clean
	rm -f .sgbuilt_toolchain
	rm -rf $(TOOLSDIR)

.PHONY: info
info:
	@echo DATE = $(DATE)
	@echo PRODUCT = $(CONFIG_PRODUCT)
	@echo VENDOR = $(CONFIG_VENDOR)
	@echo ARCH = $(ARCH)
	@echo MACHINE = $(MACHINE)
	@echo ENDIAN = $(ENDIAN)
	@echo FLOAT = $(FLOAT)
	@echo MABI = $(MABI)
	@echo CONFIGURE_HOST = $(CONFIGURE_HOST)
	@echo CONFIGURE_TOOL = $(CONFIGURE_TOOL)
	@echo CROSS_COMPILE = $(CROSS_COMPILE)

.PHONY: automake
automake:
	$(MAKE) -C config automake

############################################################################

#
# Config stuff, we recall ourselves to load the new config.arch before
# running the kernel and other config scripts
#

.PHONY: Kconfig
conf: Kconfig
Kconfig:
	@chmod u+x config/mkconfig
	config/mkconfig > Kconfig

include config/Makefile.conf

SCRIPTS_BINARY_config     = conf
SCRIPTS_BINARY_menuconfig = mconf
SCRIPTS_BINARY_nconfig    = nconf
SCRIPTS_BINARY_qconfig    = qconf
SCRIPTS_BINARY_gconfig    = gconf
SCRIPTS_BINARY_xconfig    = gconf
.PHONY: config menuconfig nconfig qconfig gconfig xconfig
menuconfig: mconf
nconfig: nconf
qconfig: qconf
gconfig: gconf
xconfig: $(SCRIPTS_BINARY_xconfig)
config menuconfig nconfig qconfig gconfig xconfig: Kconfig conf
	KCONFIG_NOTIMESTAMP=1 $(SCRIPTSDIR)/$(SCRIPTS_BINARY_$@) Kconfig
	@if [ ! -f .config ]; then \
		echo; \
		echo "You have not saved your config, please re-run 'make $@'"; \
		echo; \
		exit 1; \
	 fi
	@chmod u+x config/setconfig
	@config/setconfig defaults
	@if egrep "^CONFIG_DEFAULTS_KERNEL=y" .config > /dev/null; then \
		$(MAKE) linux_$@; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_MODULES=y" .config > /dev/null; then \
		$(MAKE) modules_$@; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_VENDOR=y" .config > /dev/null; then \
		$(MAKE) myconfig_$@; \
	 fi
	@config/setconfig final

.PHONY: oldconfig
oldconfig: Kconfig conf
	KCONFIG_NOTIMESTAMP=1 $(SCRIPTSDIR)/conf --oldconfig Kconfig
	@chmod u+x config/setconfig
	@config/setconfig defaults
	@$(MAKE) oldconfig_linux
	@$(MAKE) oldconfig_modules
	@$(MAKE) oldconfig_config
	@$(MAKE) oldconfig_uClibc
	@config/setconfig final

.PHONY: generated_headers
generated_headers:
	if [ ! -f $(LINUXDIR)/include/linux/autoconf.h ] ; then \
		ln -sf $(ROOTDIR)/$(LINUXDIR)/include/generated/autoconf.h $(LINUXDIR)/include/linux/autoconf.h ; \
	fi

.PHONY: modules
modules: generated_headers
	. $(LINUXDIR)/.config; if [ "$$CONFIG_MODULES" = "y" ]; then \
		[ -d $(LINUXDIR)/modules ] || mkdir $(LINUXDIR)/modules; \
		$(MAKEARCH_KERNEL) -j$(HOST_NCPU) -C $(LINUXDIR) modules || exit 1; \
	fi

.PHONY: modules_install
modules_install:
	. $(LINUX_CONFIG); \
	. $(CONFIG_CONFIG); \
	if [ "$$CONFIG_MODULES" = "y" ]; then \
		[ -d $(ROMFSDIR)/lib/modules ] || mkdir -p $(ROMFSDIR)/lib/modules; \
		$(MAKEARCH_KERNEL) -C $(LINUXDIR) INSTALL_MOD_CMD="$(ROMFSINST) -S -r \"\"" INSTALL_MOD_PATH=$(ROMFSDIR) DEPMOD=$(ROOTDIR)/tools/depmod.sh modules_install || exit 1; \
		rm -f $(ROMFSDIR)/lib/modules/*/build; \
		rm -f $(ROMFSDIR)/lib/modules/*/source; \
		find $(ROMFSDIR)/lib/modules -type f -name "*.ko" | xargs -r $(STRIP) -R .comment -R .note -g --strip-unneeded; \
	fi

SYNCCONFIG = KCONFIG_AUTOCONFIG=auto.conf \
	KCONFIG_TRISTATE=tristate.conf \
	KCONFIG_AUTOHEADER=autoconf.h \
	$(SCRIPTSDIR)/conf --syncconfig Kconfig

linux_%:
	KCONFIG_NOTIMESTAMP=1 $(MAKEARCH_KERNEL) -C $(LINUXDIR) $(patsubst linux_%,%,$@)
modules_%:
	[ ! -d modules ] || KCONFIG_NOTIMESTAMP=1 $(MAKEARCH) -C modules $(patsubst modules_%,%,$@)
	[ ! -d modules -o $* = clean ] || (cd modules && $(SYNCCONFIG))
myconfig_%:
	KCONFIG_NOTIMESTAMP=1 $(MAKEARCH) -C config $(patsubst myconfig_%,%,$@)
	cd config && $(SYNCCONFIG)
oldconfig_config: myconfig_oldconfig
oldconfig_modules: modules_oldconfig
oldconfig_linux: linux_oldconfig
oldconfig_uClibc:
	[ -z "$(findstring uClibc,$(LIBCDIR))" ] || KCONFIG_NOTIMESTAMP=1 $(MAKEARCH) -C $(LIBCDIR) oldconfig

############################################################################
#
# normal make targets
#

.PHONY: romfs
romfs: romfs.newlog romfs.newfakeroot romfs.subdirs modules_install romfs.post

.PHONY: romfs.newlog
romfs.newlog:
	rm -f $(IMAGEDIR)/romfs-inst.log
	rm -f ${IMAGEDIR}/license.log

.PHONY: romfs.newfakeroot
romfs.newfakeroot:
	rm -f $(ROOTDIR)/tools/fakeroot-build.sh

.PHONY: romfs.subdirs
romfs.subdirs:
	for dir in vendors $(DIRS) ; do [ ! -d $$dir ] || $(MAKEARCH) -C $$dir romfs || exit 1 ; done

.PHONY: romfs.post
romfs.post:
	$(MAKEARCH) -C vendors romfs.post
	-find $(ROMFSDIR)/. -name CVS | xargs -r rm -rf
	. $(LINUXDIR)/.config; if [ "$$CONFIG_INITRAMFS_SOURCE" != "" ]; then \
		$(MAKEARCH_KERNEL) -j$(HOST_NCPU) -C $(LINUXDIR) $(LINUXTARGET) || exit 1; \
	fi
ifdef CONFIG_USER_PYTHON_REMOVE_SOURCE
	for i in $(ROMFSDIR)/usr/lib/python*; do \
		python=$(ROOTDIR)/user/python/build/Python-Hostinstall/bin/`basename $$i`; \
		$$python -m compileall -b $$i || exit 1; \
		find $$i -type d -name __pycache__ -exec rm -r {} + ; \
		find $$i -type f -name "*.py" -delete ; \
	done
endif

.PHONY: image
image:
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	$(MAKEARCH) -C vendors image
	@echo "Package:License:URL:Target Filesystem location" > $(IMAGEDIR)/acl-licenses.txt
	@sort -u $(IMAGEDIR)/license.log >> $(IMAGEDIR)/acl-licenses.txt
	@if grep :UNKNOWN: $(IMAGEDIR)/acl-licenses.txt; then \
	    echo "ERROR: packages have an Unknown license in this build" >&2;\
	    echo "-----------------------------------------------------" >&2;\
	    grep :UNKNOWN: $(IMAGEDIR)/acl-licenses.txt >&2; \
	    echo "-----------------------------------------------------" >&2;\
	    grep -qe "CONFIG_PROP.*=y" $(ROOTDIR)/config/.config && \
		exit 1; \
	fi
	-$(call ANALYZE_SUMMARY)

.PHONY: release
release:
	[ -d $(RELDIR) ] || mkdir -p $(RELDIR)
	@prefix=$(CONFIG_PRODUCT)-$(VERSIONPKG)-`date -u "-d$(BUILD_START_STRING)" +%Y%m%d%H%M`; \
	for f in $(RELFILES) $(IMAGEDIR)/acl-licenses.txt; do \
		s=`echo "$$f" | sed 's/^\([^,]*\)\(,.*\)\{0,1\}$$/\1/'`; \
		d=`echo "$$s" | sed 's/\([^,]*\)\([.][^.]*,\)\{0,1\}/\1/'`; \
		[ -f "$$s" ] || continue; \
		dest="$$prefix-`basename $$d`"; \
		echo "$$dest"; \
		cp "$$s" "$(RELDIR)/$$dest"; \
	done

.PHONY: single
single:
	$(MAKE) NON_SMP_BUILD=1

#
# fancy target that allows a vendor to have other top level
# make targets,  for example "make vendor_flash" will run the
# vendor_flash target in the vendors directory
#

vendor_%:
	$(MAKEARCH) -C vendors $@

.PHONY: linux
linux:
	. $(LINUXDIR)/.config; if [ "$$CONFIG_INITRAMFS_SOURCE" != "" ]; then \
	    ( \
		cd $(LINUXDIR) ; \
		: For each target: check if file or directory, and create, if; \
		: not exist; \
		for f in $$CONFIG_INITRAMFS_SOURCE; do \
			bn=`basename $$f`; \
			if [ "$${bn##*.}" = "$$bn" ]; then \
				mkdir -p $$f || exit 1; \
			else \
				mkdir -p `dirname $$f`; \
				touch $$f || exit 1; \
			fi; \
		done; \
	    ) \
	fi
	@if expr "$(LINUXDIR)" : 'linux-2\.[0-4].*' > /dev/null && \
			 [ ! -f $(LINUXDIR)/.depend ] ; then \
		echo "ERROR: you need to do a 'make dep' first" ; \
		exit 1 ; \
	fi
	$(MAKEARCH_KERNEL) -j$(HOST_NCPU) -C $(LINUXDIR) $(LINUXTARGET) || exit 1
	@if ! expr "$(LINUXDIR)" : 'linux-2.[01234].*' > /dev/null ; then \
		: ignore failure in headers_install; \
		$(MAKEARCH_KERNEL) -j$(HOST_NCPU) -C $(LINUXDIR) headers_install || true; \
	fi
	if [ -f $(LINUXDIR)/vmlinux ]; then \
		ln -f $(LINUXDIR)/vmlinux $(LINUXDIR)/linux ; \
	fi

linux%_only: linux

.PHONY: sparse
sparse:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) C=1 $(LINUXTARGET) || exit 1

.PHONY: sparseall
sparseall:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) C=2 $(LINUXTARGET) || exit 1

.PHONY: subdirs
subdirs: linux modules
	for dir in $(DIRS) ; do [ ! -d $$dir ] || $(MAKEARCH) -C $$dir || exit 1 ; done

dep:
	@if [ ! -f $(LINUXDIR)/.config ] ; then \
		echo "ERROR: you need to do a 'make config' first" ; \
		exit 1 ; \
	fi
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) dep

# This one removes all executables from the tree and forces their relinking
.PHONY: relink
relink:
	find user prop vendors -type f -name '*.gdb' | sed 's/^\(.*\)\.gdb/\1 \1.gdb/' | xargs rm -f

clean: modules_clean
	for dir in $(LINUXDIR) $(DIRS) vendors config/kconfig; do [ ! -d $$dir ] || $(MAKEARCH) -C $$dir clean ; done
	rm -rf $(ROMFSDIR)/*
	rm -rf $(STAGEDIR)/*
	rm -rf $(IMAGEDIR)/*
	rm -f $(LINUXDIR)/linux
	rm -f $(LINUXDIR)/include/asm
	rm -f $(LINUXDIR)/include/linux/autoconf.h
	rm -f $(LINUXDIR)/usr/include/linux/autoconf.h
	rm -rf $(LINUXDIR)/net/ipsec/alg/libaes $(LINUXDIR)/net/ipsec/alg/perlasm
	rm -f $(LINUXDIR)/net/ipsec/.linked
	$(call ANALYZE_CLEAN,"")

real_clean mrproper: clean
	[ -d "$(LINUXDIR)" ] && $(MAKEARCH_KERNEL) -C $(LINUXDIR) mrproper || :
	[ -d uClibc ] && $(MAKEARCH) -C uClibc distclean || :
	[ -d modules ] && $(MAKEARCH) -C modules distclean || :
	[ -d "$(RELDIR)" ] && $(MAKEARCH) -C $(RELDIR) clean || :
	-$(MAKEARCH) -C config clean
	rm -rf romfs Kconfig config.arch images
	rm -rf .config .config.old .oldconfig autoconf.h auto.conf

linux_distclean:
	-$(MAKEARCH_KERNEL) -C $(LINUXDIR) distclean

distclean: mrproper linux_distclean toolchain_clean
	-rm -f user/tinylogin/applet_source_list user/tinylogin/config.h
	-rm -f lib/uClibc lib/uClibc-ng lib/glibc lib/musl
	-rm -f glibc/install musl/install
	-rm -f uClibc-ng/.config
	-$(MAKE) -C tools/ucfront clean
	-rm -f tools/ucfront-gcc tools/ucfront-g++ tools/ucfront-ld tools/ucfront-env tools/jlibtool
	-$(MAKE) -C tools/sg-cksum clean
	-rm -f tools/cksum

.PHONY: bugreport
bugreport:
	rm -rf ./bugreport.tar.gz ./bugreport
	mkdir bugreport
	$(HOSTCC) -v 2> ./bugreport/host_vers
	$(CROSS_COMPILE)gcc -v 2> ./bugreport/toolchain_vers
	cp .config bugreport/
	mkdir bugreport/$(LINUXDIR)
	cp $(LINUXDIR)/.config bugreport/$(LINUXDIR)/
	if [ -f $(LIBCDIR)/.config ] ; then \
		set -e ; \
		mkdir bugreport/$(LIBCDIR) ; \
		cp $(LIBCDIR)/.config bugreport/$(LIBCDIR)/ ; \
	fi
	mkdir bugreport/config
	cp config/.config bugreport/config/
	tar czf bugreport.tar.gz bugreport
	rm -rf ./bugreport

%_only: tools
	@case "$(@)" in \
	single*) $(MAKE) NON_SMP_BUILD=1 `expr $(@) : 'single[_]*\(.*\)'` ;; \
	*/*) d=`expr $(@) : '\([^/]*\)/.*'`; \
	     t=`expr $(@) : '[^/]*/\(.*\)'`; \
	     $(MAKEARCH) -C $$d $$t;; \
	*)   $(MAKEARCH) -C $(*);; \
	esac
	-$(call ANALYZE_SUMMARY)

%_clean:
	@case "$(@)" in \
	single*) $(MAKE) NON_SMP_BUILD=1 `expr $(@) : 'single[_]*\(.*\)'` ;; \
	*/*) d=`expr $(@) : '\([^/]*\)/.*'`; \
	     t=`expr $(@) : '[^/]*/\(.*\)'`; \
	     $(call ANALYZE_CLEAN,$$d); \
	     $(MAKEARCH) -C $$d $$t;; \
	*)   $(call ANALYZE_CLEAN,$(*)); \
	     $(MAKEARCH) -C $(*) clean;; \
	esac

%_romfs:
	@case "$(@)" in \
	single*) $(MAKE) NON_SMP_BUILD=1 `expr $(@) : 'single[_]*\(.*\)'` ;; \
	*/*) d=`expr $(@) : '\([^/]*\)/.*'`; \
	     t=`expr $(@) : '[^/]*/\(.*\)'`; \
	     $(MAKEARCH) -C $$d $$t;; \
	*)   $(MAKEARCH) -C $(*) romfs;; \
	esac

vendors/%_defconfig:
	$(MAKE) $(*)_defconfig

%_defconfig: conf
	@if [ ! -f "vendors/$(*)/config.device$(ALTDEF)" ]; then \
		echo "vendors/$(*)/config.device$(ALTDEF) must exist first"; \
		exit 1; \
	 fi
	-$(MAKE) clean > /dev/null 2>&1
	cp vendors/$(*)/config.device$(ALTDEF) .config
	yes "" | make oldconfig
	chmod u+x config/setconfig
	yes "" | config/setconfig defconfig
	config/setconfig final
	-$(MAKE) dep 2> /dev/null

%_default: conf
	$(MAKE) $(*)_defconfig ALTDEF="$(ALTDEF)"
	unset VERSIONPKG ; \
	$(MAKE)

config_error:
	@echo "*************************************************"
	@echo "You have not run make config."
	@echo "The build sequence for this source tree is:"
	@echo "1. 'make config' or 'make xconfig'"
	@echo "2. 'make dep'"
	@echo "3. 'make'"
	@echo "*************************************************"
	@exit 1

prune: ucfront
	@for i in `ls -d linux-* | grep -v $(LINUXDIR)`; do \
		rm -fr $$i; \
	done
	$(MAKE) -C lib prune
	$(MAKE) -C user prune
	$(MAKE) -C vendors prune

dist-prep:
	-find $(ROOTDIR) -name 'Makefile*.bin' | while read t; do \
		$(MAKEARCH) -C `dirname $$t` -f `basename $$t` $@; \
	 done

help:
	@echo "Quick reference for various supported make commands."
	@echo "----------------------------------------------------"
	@echo ""
	@echo "make xconfig               Configure the target etc"
	@echo "make config                \""
	@echo "make menuconfig            \""
	@echo "make nconfig               \""
	@echo "make qconfig               \""
	@echo "make gconfig               \""
	@echo "make dep                   2.4 and earlier kernels need this step"
	@echo "make                       build the entire tree and final images"
	@echo "make clean                 clean out compiled files, but not config"
	@echo "make distclean             clean out all non-distributed files"
	@echo "make oldconfig             re-run the config without interaction"
	@echo "make linux                 compile the selected kernel only"
	@echo "make romfs                 install all files to romfs directory"
	@echo "make image                 combine romfs and kernel into final image"
	@echo "make info                  print out configured information"
	@echo "make modules               build all modules"
	@echo "make modules_install       install modules into romfs"
	@echo "make toolchain_only        build gcc based toolchain for target"
	@echo "make toolchain_package     package up host built toolchain"
	@echo "make DIR_only              build just the directory DIR"
	@echo "make DIR_romfs             install files from directory DIR to romfs"
	@echo "make DIR_clean             clean just the directory DIR"
	@echo "make single                non-parallelised build"
	@echo "make single[make-target]   non-parallelised build of \"make-target\""
	@echo "make V/P_default           full default build for V=Vendor/P=Product"
	@echo "make prune                 clean out uncompiled source (be careful)"
	@echo ""
	@echo "Typically you want to start with this sequence before experimenting."
	@echo ""
	@echo "make config                select platform, kernel, etc, customise nothing."
	@echo "make dep                   optional but safe even on newer kernels."
	@echo "make                       build it as the creators intended."
	@exit 0
	

############################################################################
############################################################################
endif # umask check
############################################################################

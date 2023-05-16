##############################################################################
#
#	Makefile -- Master makefile for all libraries.
#

.EXPORT_ALL_VARIABLES:

all::

##############################################################################
#
# Include architecture specific build rules.
#

_reldir = .
include lib.mak

##############################################################################

dir_base = $(LIBCDIR)
dir_smp  = $(LIBCDIR) glib dbus dbus-glib libgmp gettext

ifeq ($(LIBCDIR),libc)
.PHONY:
libm: $(LIBCDIR)
dir_base += libm
lib_libm = *.a
inc_libm = *.h,.
endif

dir_$(CONFIG_LIB_ADNS) += adns
lib_adns = src/libadns.a

dir_$(CONFIG_LIB_FLEX) += flex
lib_flex = build/*.a
inc_flex = build/FlexLexer.h,.

dir_$(CONFIG_LIB_LIBGMP) += libgmp
lib_libgmp = build/install/lib/*.a
inc_libgmp = build/install/include/*.h,.

dir_$(CONFIG_LIB_LIBG) += libg
lib_libg = *.a

dir_$(CONFIG_LIB_UEV) += libuev
lib_uev = *.a

dir_$(CONFIG_LIB_ITE) += libite
lib_ite = *.a

dir_$(CONFIG_LIB_LIBPAM) += libpam
lib_libpam = build/libpam*/.libs/*.a build/libpam*/.libs/*.so
inc_libpam = */include/security/*.h,security

dir_$(CONFIG_LIB_LIBPCAP) += libpcap
lib_libpcap = *.a
inc_libpcap = \
	pcap*.h,. \
	pcap/*.h,pcap \
	net/bpf.h,net

dir_$(CONFIG_LIB_LIBPCAP) += libpcapng
lib_libpcapng = *.a
dep_libpcapng = libpcap
inc_libpcapng = \
	pcap*.h,. \
	pcap/*.h,pcap \
	net/bpf.h,net

dir_$(CONFIG_LIB_ZLIB) += zlib
lib_zlib = libz.a* libz.so*
inc_zlib = *.h,. \

dir_$(CONFIG_LIB_LIBBZ2) += libbzip2
lib_libbzip2 = libbz2.a* libbz2.so*
inc_libbzip2 = bzlib.h,.

dir_$(CONFIG_LIB_LIBATM) += libatm
lib_libatm = \
	src/lib/.libs/*.a \
	src/lib/*.so* \
	src/lib/*.a
inc_libatm = src/include/atm*.h,.

dir_$(CONFIG_LIB_LIBNET) += Libnet

dir_$(CONFIG_LIB_LIBNETOLD) += libnet
lib_libnet = *.a
inc_libnet = *.h,.

dir_$(CONFIG_LIB_LIBJPEG) += libjpeg
lib_libjpeg = *.a
inc_libjpeg = *.h,.

dir_$(CONFIG_LIB_LIBUPNP) += libupnp
lib_libupnp = upnp/bin/*.so*
inc_libupnp = upnp/inc/upnp,.

dir_$(CONFIG_LIB_LIBKRB5) += libkrb5
dep_libkrb5 = termcap ncurses
lib_libkrb5 = build/lib/*.so
inc_libkrb5 = \
	build/include/com_err.h,. \
	build/include/krb5/*.h,. \
	build/include/gssapi/*.h,.

dir_$(CONFIG_LIB_TINYTCL) += tinytcl
path_tinytcl = ../user/tinytcl
lib_tinytcl = *.a* *.so*
inc_tinytcl = *.h,.

dir_$(CONFIG_LIB_LIBCCMALLOC) += libccmalloc

dir_$(CONFIG_LIB_ARES) += libares
lib_libares = build/*.a
inc_libares = ares.h,.

dir_$(CONFIG_LIB_LIBLZO) += lzo
lib_lzo = build/src/.libs/*.a
inc_lzo = include/lzo,.

dir_$(CONFIG_LIB_LIBLZMA) += liblzma
lib_libzlma = *.a

dir_$(CONFIG_LIB_OSIP2) += osip2
lib_osip2 = build/install/lib/*.a
inc_osip2 = build/install/include/osip*2,.

dir_$(CONFIG_LIB_LIBEFENCE) += libefence
lib_libefence = *.a

dir_$(CONFIG_LIB_TERMCAP) += termcap
lib_termcap = *.a *.so
inc_termcap = termcap.h,.

dir_$(CONFIG_USER_TCL_LIBTCL) += tcl
path_tcl = ../user/tcl
lib_tcl = build/*.a

dir_$(CONFIG_LIB_IPTABLES) += iptables
path_iptables = ../user/iptables
lib_iptables = install/lib/*.a install/lib/*.so*
inc_iptables = install/include/*,.

dir_$(CONFIG_LIB_LUA) += lua
dep_lua = readline ncurses
path_lua = ../user/lua
lib_lua = build/lua-5.1.4/src/*.a
inc_lua = build/lua-5.1.4/src/lua.h,. \
	build/lua-5.1.4/src/luaconf.h,. \
	build/lua-5.1.4/src/lualib.h,. \
	build/lua-5.1.4/src/lauxlib.h,.

dir_$(CONFIG_LIB_READLINE) += readline
path_readline = ../user/readline
lib_readline = build/readline-7.0-install/lib/*.so*
inc_readline = build/readline-7.0-install/include/readline/*.h,readline
dep_readline = termcap

dir_$(CONFIG_LIB_LIBIBERTY) += libiberty
lib_libiberty = build/*.a

dir_$(CONFIG_LIB_BFD) += bfd
lib_bfd = build/*.a
inc_bfd = build/bfd.h,.

dir_$(CONFIG_LIB_LIBJUDY) += libjudy
lib_libjudy = \
	src/obj/.libs/libJudy.a \
	src/obj/.libs/libJudy.so

dir_$(CONFIG_LIB_CLASSPATH) += classpath

dir_$(CONFIG_LIB_BITVECTOR) += bitvector
lib_bitvector = *.so
inc_bitvector = *.h,.

dir_$(CONFIG_LIB_LIBNVRAM) += libnvram
lib_libnvram = *.so *.a
inc_libnvram = *.h,.

dir_$(CONFIG_LIB_LIBPNG) += libpng
dep_libpng = zlib
lib_libpng = *.a
inc_libpng = *.h,.

dir_$(CONFIG_LIB_LIBXML2) += libxml2
lib_libxml2 = build/.libs/libxml2.so
inc_libxml2 = \
	include/libxml/*.h,libxml \
	build/include/libxml/*.h,libxml

dir_$(CONFIG_LIB_PWLIB) += pwlib
dep_pwlib = STLport

dir_$(CONFIG_LIB_LIBOPENH323) += libopenh323
dep_libopenh323 = pwlib

dir_$(CONFIG_LIB_MINI_LIBINTL) += mini-libintl
inc_mini-libintl = libintl.h,.

dir_$(CONFIG_LIB_LIBOPENGEAR) += libopengear
dep_libopengear = jansson scew openssl
lib_libopengear = build/*.a \
	build/*.so*
inc_libopengear = include/opengear/*.h,opengear

dep_libftdi = libusb STLport
dep_libunbound = libldns expat
dep_libgcrypt = libgpg-error
dep_libssh = openssl
dep_libldns = openssl
dep_libdnsres = libevent
dep_libldap = openssl
dep_libnettle = libgmp
dep_iptables = libpcap
dep_glib = pcre zlib libffi libiconv

#
# dir 6, 7 and 8 reserved for Makefile.local
#
-include Makefile.local

# And build libraries in the prop directory last
dir_extra += $(ROOTDIR)/prop

-include Makefile.auto

##############################################################################

DIRS_y   = $(sort $(dir_base) $(dir_y) $(dir_extra))

DIRS_all = $(dir_$(n)_n) $(dir_$(n)_) $(sort $(dir_base) $(dir_y) $(dir_n) $(dir_) $(dir_extra))
DIRPATH = $(if $(path_$(1)),$(path_$(1)),$(1))

##############################################################################

# The dynamic linker library needs to be linked into lib via a different name
LD_LIB = $(ROOTDIR)/lib/uClibc/lib/ld-uClibc-0.9.29.so
LD_LIB_DEST = $(STAGEDIR)/lib/ld-linux.so.2

##############################################################################

all::
	$(MAKE) -j$(HOST_NCPU) base $(DIRS_y) || exit $$?

$(DIRS_y): base

define dep
.PHONY: $(1)
$(1): base $(dep_$(1))
endef

$(foreach d,$(sort $(dir_y) $(dir_n) $(dir_)),$(eval $(call dep,$(d))))

.PHONY: base
base: $(dir_base) basepost
	@ln -sf $(LD_LIB) $(LD_LIB_DEST)

$(dir_base): baseprep

basepost: $(dir_base)

.PHONY: baseprep
baseprep: REALCC = $(subst ucfront-gcc,,$(CC))
baseprep:
	mkdir -p $(STAGEDIR)/lib
	$(MAKE) -j1 -C ../include || exit $$?
	LIBGCC_S_FILES="$$(dirname $$(realpath "$$($(REALCC) -print-file-name=libgcc_s.so)"))/libgcc_s.so*"; \
	LIBSTDCXX_FILES="$$(dirname $$(realpath "$$($(REALCC) -print-file-name=libstdc++.so)"))/libstdc++.so*"; \
	for LIB in $${LIBGCC_S_FILES} $${LIBSTDCXX_FILES}; do \
		[ -f "$${LIB}" ] || continue; \
		echo "Symlink: $${LIB} under $(STAGEDIR)/lib"; \
		ln -sf "$${LIB}" $(STAGEDIR)/lib; \
	done

.PHONY: basepost
basepost:
ifneq ($(findstring uClibc,$(LIBCDIR)),)
	$(MAKE) -C $(LIBCDIR) install PREFIX=$(STAGEDIR)/ DEVEL_PREFIX= RUNTIME_PREFIX=
	for LIB in libc.so libpthread.so ; do \
		if head -n 1 $(LIBCDIR)/lib/$$LIB | grep -q "GNU ld script" ; then \
			sed -e 's/lib\///g' < $(LIBCDIR)/lib/$$LIB > $(STAGEDIR)/lib/$$LIB ; \
		fi ; \
	done
endif

.PHONY: $(dir_extra)
$(dir_extra): base $(dir_y)

# the actual dir target
.PHONY: $(DIRS_y)
$(DIRS_y):
	i=$(call DIRPATH,$@); [ ! -d $$i ] || ( touch $$i/.sgbuilt_lib && $(MAKE) $(if $(filter $@,$(dir_smp)),-j$(HOST_NCPU),$(SUBDIR_SMP_BUILD)) -C $$i ) || exit $$?
	@set -f; \
	for p in $(lib_$@); do \
		set +f; \
		for i in $(ROOTDIR)/lib/$(call DIRPATH,$@)/$$p; do \
			[ ! -e "$$i" ] && continue; \
			[ "`basename $$i`" = CVS ] && continue; \
			[ -L "$(STAGEDIR)/lib/`basename $$i`" ] && continue; \
			ln -s $$i $(STAGEDIR)/lib/.; \
			echo "ln -s $$i $(STAGEDIR)/lib/."; \
		done ; \
	done
	@set -f; \
	for p in $(inc_$@); do \
		src=$(ROOTDIR)/lib/$(call DIRPATH,$@)/`expr "$$p" : '\(.*\),.*'` ; \
		dst=$(STAGEDIR)/include/`expr "$$p" : '.*,\(.*\)'` ; \
		set +f; \
		for i in $$src; do \
			[ ! -e "$$i" ] && continue; \
			[ "`basename $$i`" = CVS ] && continue; \
			[ -L "$$dst/`basename $$i`" ] && continue; \
			[ -d $$dst ] || mkdir $$dst; \
			ln -s $$i $$dst/.; \
			echo "ln -s $$i $$dst/."; \
		done ; \
	done

%_only:
	touch $(@:_only=)/.sgbuilt_lib && $(MAKE) -j$(HOST_NCPU) $(SUBDIR_SMP_BUILD) -C $(@:_only=)

%_clean:
	$(MAKE) -j1 -C $(@:_clean=) clean; rm -f $(@:_clean=)/.sgbuilt_lib; true

%_romfs:
	$(MAKE) $(SUBDIR_SMP_BUILD) -C $(@:_romfs=) romfs

##############################################################################

# Not the best place to install these symlinks, but it will do for now
romfs:
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_ETC_CONFIG -s /etc/config/passwd /etc/passwd
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_ETC_CONFIG -s /etc/config/group /etc/group
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_ETC_CONFIG -s /etc/config/mtab /etc/mtab
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_ETC_CONFIG -s /etc/config/fstab /etc/fstab
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_ETC_CONFIG -s /etc/config/hosts /etc/hosts
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_ETC_CONFIG -s /etc/config/resolv.conf /etc/resolv.conf
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_ETC_CONFIG -e CONFIG_USER_BASH_BASH -s /etc/config/profile /etc/profile
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_ETC_CONFIG -e CONFIG_USER_TINYLOGIN_SHADOWPASSWDS -s /etc/config/shadow /etc/shadow
	for i in $(foreach d,$(DIRS_y),$(call DIRPATH,$d)) ; do \
		[ ! -d $$i ] || $(MAKE) $(SUBDIR_SMP_BUILD) -C $$i romfs || exit $$? ; \
	done
	FNAME=$$($(CC) $(CFLAGS) -print-file-name=libgcc_s.so.1) ; \
	export ACL_LICENSE="GPLv3 with GCC Runtime Library Exception"; \
	export ACL_PKG=gcc-`$(CC) -dumpversion)`; \
	export ACL_URL=https://gcc.gnu.org/; \
	$(ROMFSINST) -e CONFIG_LIB_INSTALL_LIBGCC_S $$FNAME /lib/`basename $$FNAME`
	FNAME=$$($(CC) $(CFLAGS) -print-file-name=libatomic.so.1) ; \
	export ACL_LICENSE="GPLv3 with GCC Runtime Library Exception"; \
	export ACL_PKG=gcc-`$(CC) -dumpversion)`; \
	export ACL_URL=https://gcc.gnu.org/; \
	$(ROMFSINST) -e CONFIG_LIB_INSTALL_LIBATOMIC $$FNAME /lib/`basename $$FNAME`
	FNAME=$$($(CC) $(CFLAGS) -print-file-name=libstdc++.so.6) ; \
	export ACL_LICENSE="GPLv3 with GCC Runtime Library Exception"; \
	export ACL_PKG=gcc-`$(CC) -dumpversion)`; \
	export ACL_URL=https://gcc.gnu.org/; \
	$(ROMFSINST) -e CONFIG_LIB_INSTALL_LIBSTDCPLUS $$FNAME /lib/`basename $$FNAME`

##############################################################################

clean:
	-for i in $(foreach d,$(DIRS_all),$(call DIRPATH,$d)); do \
		if [ -f $$i/.sgbuilt_lib ]; then \
			$(MAKE) -C $$i clean ; \
			rm -f $$i/.sgbuilt_lib; \
		fi; \
	done

##############################################################################

prune:
	-for i in $(foreach d,$(DIRS_all),$(call DIRPATH,$d)); do \
		found=0; \
		for j in $(sort $(foreach d,$(DIRS_y),$(call DIRPATH,$d))) ; do \
			if [ $$i = $$j ]; then \
				found=1; \
			fi; \
		done; \
		if [ $$found = 0 ]; then \
			rm -fr $$i; \
		fi; \
	done

##############################################################################

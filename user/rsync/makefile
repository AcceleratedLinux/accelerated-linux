
URL = https://download.samba.org/pub/rsync/src/rsync-3.2.7.tar.gz

CONFOPTS += --disable-xxhash
CONFOPTS += --disable-zstd
CONFOPTS += --disable-lz4
ifeq ($(ARCH),x86_64)
 # 3.2.7's x86_64 simd optimizations assume indirect function support
 # in the compiler, which our toolchain doesn't have
 CONFOPTS += --disable-roll-simd
endif

include $(ROOTDIR)/tools/automake.inc

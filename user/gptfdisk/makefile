URL     = https://sourceforge.net/projects/gptfdisk/files/gptfdisk/1.0.9/gptfdisk-1.0.9.tar.gz

BUILD   = gdisk sgdisk

CPPFLAGS += -D_LARGEFILE64_SOURCE
LDFLAGS += -lstdc++ -lm
ifndef CONFIG_DEFAULTS_LIBC_MUSL
LDFLAGS += -lintl
endif

include $(ROOTDIR)/tools/automake.inc

build/%-installed: build/%-$(BUILDTARGET)
	install -d build/$*-install/sbin/
	install build/$*/gdisk build/$*-install/sbin/
	install build/$*/sgdisk build/$*-install/sbin/



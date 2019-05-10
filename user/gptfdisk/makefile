
URL     = https://downloads.sourceforge.net/project/gptfdisk/gptfdisk/1.0.4/gptfdisk-1.0.4.tar.gz
BUILD   = gdisk sgdisk
LDFLAGS += -lstdc++ -lintl

include $(ROOTDIR)/tools/automake.inc

build/%-installed: build/%-$(BUILDTARGET)
	install -d build/$*-install/sbin/
	install build/$*/gdisk build/$*-install/sbin/
	install build/$*/sgdisk build/$*-install/sbin/



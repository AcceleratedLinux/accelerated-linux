URL = https://github.com/edgexfoundry/edgex-go.git
METHOD = git
VERSION = v1.0.0

BUILD = build
FINALTARGET = built
MAKEVARS =

AUTOMAKE_ROMFS = my_romfs
edgex-go_CLEAN = my_clean

export CC = ucfront-gcc.sh
export GOPATH = $(shell pwd)/build/go
export GOARCH = $(ARCH)

include $(ROOTDIR)/tools/automake.inc

EDGEX_BINARIES := \
	core-command \
	core-data \
	core-metadata \
	export-client \
	export-distro \
	support-scheduler

my_romfs:
	for BIN in $(EDGEX_BINARIES); do \
		$(ROMFSINST) -d build/edgex-go/cmd/$$BIN/$$BIN /usr/bin/edgex-$$BIN; \
	done

my_clean:
	go clean -modcache

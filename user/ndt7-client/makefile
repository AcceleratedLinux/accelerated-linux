
URL                = https://github.com/m-lab/ndt7-client-go
METHOD             = git
VERSION            = main
FINALTARGET        = my_build
AUTOMAKE_ROMFS     = my_romfs
GOPATH             = $(shell pwd)/build

ifeq ($(ARCH),x86_64)
GOARCH             = amd64
else
GOARCH             = $(ARCH)
endif

include $(ROOTDIR)/tools/automake.inc

build/%-my_build: build/%-patched makefile
	@echo "Building $* ..."
	cd build/ndt7-client-go; \
		env GOPATH=$(GOPATH) GOOS=linux GOARCH=$(GOARCH) \
		go get ./cmd/ndt7-client
	touch $@

my_romfs:
	$(ROMFSINST) -d $(GOPATH)/bin/linux_arm/ndt7-client /bin/ndt7-client


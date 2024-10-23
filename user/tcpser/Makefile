ACL_LICENSE = GPLv2
VERSION = 008ad5c84d4b704b74ddb4ea6fcb834ff783fd18
URL = https://github.com/go4retro/tcpser.git
METHOD = git

INSTALLTARGET=my_install

include $(ROOTDIR)/tools/automake.inc

build/tcpser-my_install: build/tcpser-$(BUILDTARGET)
	@echo "Installing tcpser ..."
	install -D build/tcpser/tcpser build/tcpser-install/bin/tcpser
	touch $@

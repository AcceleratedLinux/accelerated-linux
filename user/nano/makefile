NANO_VERSION = 8.1
URL = https://www.nano-editor.org/dist/v8/nano-$(NANO_VERSION).tar.xz

AUTOMAKE_ROMFS = my_romfs
LDFLAGS += -lncurses -ltinfo

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) build/nano-$(NANO_VERSION)/src/nano /bin/nano

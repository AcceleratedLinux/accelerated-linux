ACL_LICENSE = Digi
ACL_URL     = http://www.digi.com/
ACL_PKG     = Digi Accelerated Linux
export ACL_PKG ACL_URL ACL_LICENSE

CFLAGS += -Wall
CFLAGS += -fPIC
CFLAGS += -I$(ROOTDIR)/lib/libpam/libpam/include
LDFLAGS += -nostdlib -L$(ROOTDIR)/lib
LDLIBS += -lpam

ifeq ($(CONFIG_LIB_LIBPAM_DEBUG),y)
 CFLAGS += -DDEBUG
endif

all: pam_portauth.so

pam_portauth.so: pam_portauth.o
	$(LINK.c) -shared $^ $(LDLIBS) -o $@

clean:
	rm -f *~ *.so *.o

romfs:
	$(ROMFSINST) pam_portauth.so /lib/security

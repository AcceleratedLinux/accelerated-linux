VERSION=4.2.5
URL=https://github.com/appneta/tcpreplay/releases/download/v4.2.5/tcpreplay-$(VERSION).tar.gz

CONFVARS :=
CONFVARS += ac_cv_linux_vers=2
CONFVARS += td_cv_buggygetaddrinfo=false

CONFOPTS += --with-libpcap=$(ROOTDIR)/lib/libpcap/build/libpcap-1.9.1
ifdef CCONFIG_LIB_LIBNL_GENERIC
LDFLAGS += -lnl
endif

include $(ROOTDIR)/tools/automake.inc

romfs:
	$(ROMFSINST) build/tcpreplay-$(VERSION)/src/tcpreplay /sbin/tcpreplay
	$(ROMFSINST) build/tcpreplay-$(VERSION)/src/tcprewrite /sbin/tcprewrite
	$(ROMFSINST) build/tcpreplay-$(VERSION)/src/tcpprep /sbin/tcpprep

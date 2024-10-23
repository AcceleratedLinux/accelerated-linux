DNSMASQ_VERSION = 2.90
URL = https://www.thekelleys.org.uk/dnsmasq/dnsmasq-$(DNSMASQ_VERSION).tar.xz

MAKEVARS = PREFIX=
MAKEVARS += COPTS="-DNO_TFTP -DNO_AUTH"
MAKEVARS += CFLAGS="$(CFLAGS)"
MAKEVARS += LDFLAGS="$(LDFLAGS)"

include $(ROOTDIR)/tools/automake.inc

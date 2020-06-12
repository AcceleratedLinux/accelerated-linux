URL = http://www.tcpdump.org/release/libpcap-1.9.1.tar.gz

CONFVARS :=
CONFVARS += ac_cv_linux_vers=2

CONFOPTS = --with-pcap=linux
CONFOPTS += --disable-dbus
CONFOPTS += --disable-can
CONFOPTS += --disable-canusb
CONFOPTS += --disable-bluetooth

ifdef CONFIG_IPV6
CONFOPTS += --enable-ipv6
endif
ifndef CONFIG_LIB_LIBNL_GENERIC
CONFOPTS += --without-libnl
endif

include $(ROOTDIR)/tools/automake.inc


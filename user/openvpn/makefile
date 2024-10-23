VERSION = 2.6.9
URL = https://swupdate.openvpn.org/community/releases/openvpn-$(VERSION).tar.gz

CONFOPTS += --disable-dco

MAKEVARS = 
CONFVARS += ac_cv_path_ROUTE=/sbin/route
CONFVARS += ac_cv_path_IFCONFIG=/sbin/ifconfig
CONFVARS += ac_cv_path_IPROUTE=/bin/ip
CONFVARS += ac_cv_path_IPROUTE=/bin/sed
CONFVARS += ac_cv_path_IPROUTE=/bin/netstat
CONFVARS += ac_cv_path_IPROUTE=/bin/fgrep

include $(ROOTDIR)/tools/automake.inc

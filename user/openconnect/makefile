
URL      = ftp://ftp.infradead.org/pub/openconnect/openconnect-8.10.tar.gz
CONFOPTS = --with-vpnc-script=/var/run/openconnect.sh
ifdef CONFIG_USER_NUTTCP
CONFVARS += ac_cv_path_NUTTCP=/usr/bin/nuttcp
endif
MAKEVARS =  

include $(ROOTDIR)/tools/automake.inc


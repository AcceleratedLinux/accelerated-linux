URL = https://github.com/chaos/powerman
METHOD = git
VERSION = 7403da3a8f987736493044cebf1404325f30545b

AUTORECONF := ./autogen.sh
CONFVARS += ac_cv_file__dev_ptmx=yes
CONFVARS += ac_cv_file__dev_ptc=no

CONFOPTS += --program-prefix=
ifdef CONFIG_USER_POWERMAN_HTTPPOWER
CONFOPTS += --with-httppower
endif
ifdef CONFIG_USER_POWERMAN_SNMPPOWER
CONFOPTS += --with-snmppower
endif

include $(ROOTDIR)/tools/automake.inc

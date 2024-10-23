VERSION = 5.9.3
URL = https://downloads.sourceforge.net/project/net-snmp/net-snmp/$(VERSION)/net-snmp-$(VERSION).tar.gz

CORE_MIBS = SNMPv2-TC SNMPv2-SMI SNMPv2-MIB SNMP-TARGET-MIB \
	SNMP-VIEW-BASED-ACM-MIB SNMP-COMMUNITY-MIB SNMP-FRAMEWORK-MIB \
	SNMP-MPD-MIB SNMP-PROXY-MIB SNMP-USER-BASED-SM-MIB RFC1155-SMI \
	RFC1213-MIB RFC-1215 IF-MIB IP-MIB IPV6-ICMP-MIB IPV6-MIB IPV6-TCP-MIB \
	IPV6-UDP-MIB IPV6-TC INET-ADDRESS-MIB IANAifType-MIB HOST-RESOURCES-MIB \
	HOST-RESOURCES-TYPES DISMAN-EVENT-MIB SNMP-NOTIFICATION-MIB SNMPv2-TM \
	UCD-SNMP-MIB UCD-DEMO-MIB UCD-DLMOD-MIB NET-SNMP-MIB NET-SNMP-AGENT-MIB \
	NET-SNMP-EXTEND-MIB

AUTOMAKE_ROMFS = my_romfs
MAKEVARS = 
CFLAGS += -D__deprecated=
CFLAGS += $(CONFIG_USER_NETSNMP_ADDITIONAL_CFLAGS)

CONFOPTS = --with-install-prefix=/
# Hide build paths
CONFVARS += NETSNMP_CONFIGURE_OPTIONS=

ifndef CONFIG_USER_NETSNMP_INCLUDE_DEBUG
CONFOPTS += --disable-debugging
endif

CONFOPTS += --without-perl-modules --disable-embedded-perl
CONFOPTS += --without-python-modules

ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
CONFOPTS += --sysconfdir=/etc/config
endif

ifdef CONFIG_CPU_BIG_ENDIAN
CONFOPTS += --with-endianness=big
else
CONFOPTS += --with-endianness=little
endif

ifdef CONFIG_USER_NETSNMP_BUILDSTATIC
CONFOPTS += --enable-shared=no
endif

ifdef CONFIG_USER_NETSNMP_SNMPD
ifdef CONFIG_USER_NETSNMP_SNMPD_MINI
CONFOPTS += --enable-mini-agent
endif
else
CONFOPTS += --disable-agent
endif

ifndef CONFIG_USER_NETSNMP_APPS
CONFOPTS += --disable-applications
endif

ifndef CONFIG_USER_NETSNMP_MANUALS
CONFOPTS += --disable-manuals
endif

ifndef CONFIG_USER_NETSNMP_SCRIPTS
CONFOPTS += --disable-scripts
endif

ifdef CONFIG_USER_NETSNMP_MIBS
ifndef CONFIG_USER_NETSNMP_MIBLOADING
CONFOPTS += --disable-mib-loading
endif
else
CONFOPTS += --disable-mibs --disable-mib-loading
endif

ifndef CONFIG_USER_NETSNMP_V1
CONFOPTS += --disable-snmpv1
endif

ifndef CONFIG_USER_NETSNMP_V2C
CONFOPTS += --disable-snmpv2c
endif

ifdef CONFIG_USER_NETSNMP_IPV6
ifndef CONFIG_IPV6
$(error CONFIG_IPV6 not defined)
else
CONFOPTS += --enable-ipv6
CONFVARS += ac_cv_have_struct_sockaddr_in6_sin6_addr=yes
endif
endif

ifdef CONFIG_USER_NETSNMP_NOTRANSPORTS
CONFOPTS += --with-out-transports=$(CONFIG_USER_NETSNMP_EXTRANSPORTS)
endif

ifdef CONFIG_USER_NETSNMP_OPENSSL
ifndef CONFIG_LIB_OPENSSL
$(error CONFIG_LIB_OPENSSL not defined)
else
CONFOPTS += --with-openssl=$(STAGEDIR)/lib
endif
endif

ifdef CONFIG_USER_NETSNMP_OVRDEFAULTS
CONFOPTS += --with-default-snmp-version=$(CONFIG_USER_NETSNMP_DEFVERSION)
CONFOPTS += --with-sys-contact=$(CONFIG_USER_NETSNMP_DEFSYSCONTACT)
CONFOPTS += --with-sys-location=$(CONFIG_USER_NETSNMP_DEFSYSLOCATION)
CONFOPTS += --with-logfile=$(CONFIG_USER_NETSNMP_DEFLOGFILE)
CONFOPTS += --with-persistent-directory=$(CONFIG_USER_NETSNMP_DEFPERSISDIR)
CONFOPTS += --with-enterprise-oid=$(CONFIG_USER_NETSNMP_DEFENTERPRISEOID)
CONFOPTS += --with-enterprise-sysoid=$(CONFIG_USER_NETSNMP_DEFENTERPRISESYSOID)
else
CONFOPTS += --with-defaults
endif

ifdef CONFIG_USER_NETSNMP_ADDMIBS
CONFOPTS += --with-mib-modules=$(CONFIG_USER_NETSNMP_ADDITIONALMIBS)
endif

include $(ROOTDIR)/tools/automake.inc

# Override automake's --prefix= which net-snmp translates to the build dir
CONFOPTS += --prefix=/

ifdef CONFIG_USER_NETSNMP_ADDITIONAL_MIB_DIRS
AMIBDIR = $(shell pwd)/build/net-snmp-$(VERSION)/agent/mibgroup

build/net-snmp-$(VERSION)-symlink-mibs: build/net-snmp-$(VERSION)-extracted
	@for d in $(CONFIG_USER_NETSNMP_ADDITIONAL_MIB_DIRS); do \
		echo "Adding MIBs from $$d"; \
		[ ! -d "$$d" ] && echo "$$d does not exist" && exit 1; \
		cd $$d; \
		[ -e ./create.sh ] && ./create.sh; \
		find . -type d | while read t; do mkdir -p $(AMIBDIR)/$$t;done;\
		find . -type f | while read t; do ln -sf $$d/$$t $(AMIBDIR)/$$t;done;\
	done
	touch $@

build/net-snmp-$(VERSION)-configured: build/net-snmp-$(VERSION)-symlink-mibs
endif

ifdef CONFIG_USER_NETSNMP_INSTALL_CORE_MIBS
MIBSDIR = build/net-snmp-$(VERSION)/mibs

build/net-snmp-$(VERSION)-strip-core-mibs: build/net-snmp-$(VERSION)-extracted
	mkdir -p $(MIBSDIR)/stripped
	smistrip -d $(MIBSDIR)/stripped $(patsubst %,$(MIBSDIR)/%.txt,$(CORE_MIBS))
	touch $@

build/net-snmp-$(VERSION)-configured: build/net-snmp-$(VERSION)-strip-core-mibs
endif

INSTALLDIR = build/net-snmp-$(VERSION)-install

my_romfs:
ifndef CONFIG_USER_NETSNMP_BUILDSTATIC
	$(ROMFSINST) -v -f $(INSTALLDIR)/lib /lib
	rm -f $(ROMFSDIR)/lib/*.a
	rm -f $(ROMFSDIR)/lib/*.la
endif
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_SNMPD $(INSTALLDIR)/sbin/snmpd /sbin/snmpd
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_BULKGET $(INSTALLDIR)/bin/snmpdsnmpbulkget /bin/snmpbulkget
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_BULKWALK $(INSTALLDIR)/bin/snmpbulkwalk /bin/snmpbulkwalk
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_DELTA $(INSTALLDIR)/bin/snmpdelta /bin/snmpdelta
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_DF $(INSTALLDIR)/bin/snmpdf /bin/snmpdf
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_GET $(INSTALLDIR)/bin/snmpget /bin/snmpget
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_GETNEXT $(INSTALLDIR)/bin/snmpgetnext /bin/snmpgetnext
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_SET $(INSTALLDIR)/bin/snmpset /bin/snmpset
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_STATUS $(INSTALLDIR)/bin/snmpstatus /bin/snmpstatus
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_TABLE $(INSTALLDIR)/bin/snmptable /bin/snmptable
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_TRANSLATE $(INSTALLDIR)/bin/snmptranslate /bin/snmptranslate
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_TRAP $(INSTALLDIR)/bin/snmptrap /bin/snmptrap
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_TRAPD $(INSTALLDIR)/sbin/snmptrapd /sbin/snmptrapd
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_USM $(INSTALLDIR)/bin/snmpusm /bin/snmpusm
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_VACM $(INSTALLDIR)/bin/snmpvacm /bin/snmpvacm
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_WALK $(INSTALLDIR)/bin/snmpwalk /bin/snmpwalk
	$(ROMFSINST) -e CONFIG_USER_NETSNMP_APPS_NETSTAT $(INSTALLDIR)/bin/snmpnetstat /bin/snmpnetstat
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_ETC_CONFIG -s config /etc/snmp
ifdef CONFIG_USER_NETSNMP_INSTALL_CORE_MIBS
	$(ROMFSINST) -d -v -f build/net-snmp-$(VERSION)/mibs/stripped /etc/snmp/mibs
endif

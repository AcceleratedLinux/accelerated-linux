#
# Makefile for net-snmp
#

CFLAGS = -D__deprecated=

NET_SNMP_CFG = --prefix=$(STAGEDIR) --disable-debugging

NET_SNMP_CFG += --disable-embedded-perl

ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
NET_SNMP_CFG += --sysconfdir=/etc/config
endif

ifdef CONFIG_CPU_BIG_ENDIAN
NET_SNMP_CFG += --with-endianness=big
else
NET_SNMP_CFG += --with-endianness=little
endif

ifdef CONFIG_USER_NETSNMP_BUILDSTATIC
NET_SNMP_CFG += --enable-shared=no
NET_SNMP_LIB_EXT = a
NET_SNMP_EXE_DIR = 
else
NET_SNMP_LIB_EXT = so
NET_SNMP_EXE_DIR = .libs/
endif

ifdef CONFIG_USER_NETSNMP_SNMPD
ifdef CONFIG_USER_NETSNMP_SNMPD_MINI
NET_SNMP_CFG += --enable-mini-agent
endif
else
NET_SNMP_CFG += --disable-agent
endif

ifndef CONFIG_USER_NETSNMP_APPS
NET_SNMP_CFG += --disable-applications
endif

ifndef CONFIG_USER_NETSNMP_MANUALS
NET_SNMP_CFG += --disable-manuals
endif

ifndef CONFIG_USER_NETSNMP_SCRIPTS
NET_SNMP_CFG += --disable-scripts
endif

ifdef CONFIG_USER_NETSNMP_MIBS
ifndef CONFIG_USER_NETSNMP_MIBLOADING
NET_SNMP_CFG += --disable-mib-loading
endif
else
NET_SNMP_CFG += --disable-mibs --disable-mib-loading
endif

ifdef CONFIG_USER_NETSNMP_NOV1
NET_SNMP_CFG += --disable-snmpv1
endif

ifdef CONFIG_USER_NETSNMP_NOV2C
NET_SNMP_CFG += --disable-snmpv2c
endif

ifdef CONFIG_USER_NETSNMP_IPV6
ifndef CONFIG_IPV6
$(error CONFIG_IPV6 not defined)
else
NET_SNMP_CFG += --enable-ipv6
CONFVARS += ac_cv_have_struct_sockaddr_in6_sin6_addr=yes
endif
endif

ifdef CONFIG_USER_NETSNMP_NOTRANSPORTS
NET_SNMP_CFG += --with-out-transports=$(CONFIG_USER_NETSNMP_EXTRANSPORTS)
endif

ifdef CONFIG_USER_NETSNMP_OPENSSL
ifndef CONFIG_LIB_LIBSSL
$(error CONFIG_LIB_LIBSSL not defined)
else
NET_SNMP_CFG += --with-openssl=$(STAGEDIR)/lib
endif
endif

ifdef CONFIG_USER_NETSNMP_OVRDEFAULTS
NET_SNMP_CFG += --with-default-snmp-version=$(CONFIG_USER_NETSNMP_DEFVERSION)
NET_SNMP_CFG += --with-sys-contact=$(CONFIG_USER_NETSNMP_DEFSYSCONTACT)
NET_SNMP_CFG += --with-sys-location=$(CONFIG_USER_NETSNMP_DEFSYSLOCATION)
NET_SNMP_CFG += --with-logfile=$(CONFIG_USER_NETSNMP_DEFLOGFILE)
NET_SNMP_CFG += --with-persistent-directory=$(CONFIG_USER_NETSNMP_DEFPERSISDIR)
NET_SNMP_CFG += --with-enterprise-oid=$(CONFIG_USER_NETSNMP_DEFENTERPRISEOID)
NET_SNMP_CFG += --with-enterprise-sysoid=$(CONFIG_USER_NETSNMP_DEFENTERPRISESYSOID)
else
NET_SNMP_CFG += --with-defaults
endif

ifdef CONFIG_USER_NETSNMP_ADDMIBS
NET_SNMP_CFG_EXTRA = --with-mib-modules=$(CONFIG_USER_NETSNMP_ADDITIONALMIBS)
endif

all: build/Makefile
	$(MAKE) -j1 xEXT=${NET_SNMP_LIB_EXT} xDIR=${NET_SNMP_EXE_DIR} -C build install

build/Makefile: makefile
	rm -fr source build
	mkdir source build
	find . -type d > .dirs
	find . ! -type d | grep -v ./makefile > .files
	while read t; do mkdir -p source/$$t; done < .dirs
	while read t; do ln -sf `pwd`/$$t source/$$t; done < .files
	rm -f .dirs .files
ifdef CONFIG_USER_NETSNMP_ADDITIONAL_MIB_DIRS
	AMIBDIR=`pwd`/source/agent/mibgroup; \
	for d in $(CONFIG_USER_NETSNMP_ADDITIONAL_MIB_DIRS); do \
		echo "Adding MIBs from $$d"; \
		[ ! -d "$$d" ] && echo "$$d does not exist" && exit 1; \
		cd $$d; \
		find . -type d | while read t; do mkdir -p $$AMIBDIR/$$t;done;\
		find . -type f | while read t; do ln -sf $$d/$$t $$AMIBDIR/$$t;done;\
	done
endif
	chmod u+x configure
	( \
		cd build; \
		CC="$(CC) $(CFLAGS)" LDFLAGS="$(LDFLAGS)" LIBS="$(LDLIBS)" \
			$(CONFVARS) sh ../source/configure $(CONFIGURE_OPTS) $(NET_SNMP_CFG) $(NET_SNMP_CFG_EXTRA); \
		if [ $$? -ne 0 ]; then \
			CC="$(CC) $(CFLAGS)" LDFLAGS="$(LDFLAGS)" LIBS="$(LDLIBS)" \
				$(CONFVARS) sh ../source/configure $(CONFIGURE_OPTS) $(NET_SNMP_CFG); \
		fi; \
	)

NET_SNMP_VERSION=20.0.0
NET_SNMP_VERSION_MAJOR=20

romfs:
ifndef CONFIG_USER_NETSNMP_BUILDSTATIC
	$(ROMFSINST) build/snmplib/.libs/libnetsnmp.so.$(NET_SNMP_VERSION) /lib/libnetsnmp.so.$(NET_SNMP_VERSION_MAJOR)
	$(ROMFSINST) build/agent/.libs/libnetsnmpagent.so.$(NET_SNMP_VERSION) /lib/libnetsnmpagent.so.$(NET_SNMP_VERSION_MAJOR)
	$(ROMFSINST) build/agent/.libs/libnetsnmpmibs.so.$(NET_SNMP_VERSION) /lib/libnetsnmpmibs.so.$(NET_SNMP_VERSION_MAJOR)
	$(ROMFSINST) build/agent/helpers/.libs/libnetsnmphelpers.so.$(NET_SNMP_VERSION) /lib/libnetsnmphelpers.so.$(NET_SNMP_VERSION_MAJOR)
ifdef CONFIG_USER_NETSNMP_APPS_TRAPD
	$(ROMFSINST) build/apps/.libs/libnetsnmptrapd.so.$(NET_SNMP_VERSION) /lib/libnetsnmptrapd.so.$(NET_SNMP_VERSION_MAJOR)
endif
endif
ifdef CONFIG_USER_NETSNMP_SNMPD
	$(ROMFSINST) build/agent/${NET_SNMP_EXE_DIR}snmpd /bin/snmpd
endif
ifdef CONFIG_USER_NETSNMP_APPS_BULKGET
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpbulkget /bin/snmpbulkget
endif
ifdef CONFIG_USER_NETSNMP_APPS_BULKWALK
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpbulkwalk /bin/snmpbulkwalk
endif
ifdef CONFIG_USER_NETSNMP_APPS_DELTA
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpdelta /bin/snmpdelta
endif
ifdef CONFIG_USER_NETSNMP_APPS_DF
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpdf /bin/snmpdf
endif
ifdef CONFIG_USER_NETSNMP_APPS_GET
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpget /bin/snmpget
endif
ifdef CONFIG_USER_NETSNMP_APPS_GETNEXT
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpgetnext /bin/snmpgetnext
endif
ifdef CONFIG_USER_NETSNMP_APPS_SET
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpset /bin/snmpset
endif
ifdef CONFIG_USER_NETSNMP_APPS_STATUS
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpstatus /bin/snmpstatus
endif
ifdef CONFIG_USER_NETSNMP_APPS_TABLE
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmptable /bin/snmptable
endif
ifdef CONFIG_USER_NETSNMP_APPS_TRANSLATE
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmptranslate /bin/snmptranslate
endif
ifdef CONFIG_USER_NETSNMP_APPS_TRAP
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmptrap /bin/snmptrap
endif
ifdef CONFIG_USER_NETSNMP_APPS_TRAPD
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmptrapd /bin/snmptrapd
endif
ifdef CONFIG_USER_NETSNMP_APPS_USM
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpusm /bin/snmpusm
endif
ifdef CONFIG_USER_NETSNMP_APPS_VACM
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpvacm /bin/snmpvacm
endif
ifdef CONFIG_USER_NETSNMP_APPS_WALK
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpwalk /bin/snmpwalk
endif
ifdef CONFIG_USER_NETSNMP_APPS_NETSTAT
	$(ROMFSINST) build/apps/snmpnetstat/${NET_SNMP_EXE_DIR}snmpnetstat /bin/snmpnetstat
endif
ifdef CONFIG_USER_NETSNMP_INSTALL_CORE_MIBS
	$(ROMFSINST) -d mibs/stripped/SNMPv2-TC /etc/snmp/mibs/SNMPv2-TC
	$(ROMFSINST) mibs/stripped/SNMPv2-SMI /etc/snmp/mibs/SNMPv2-SMI
	$(ROMFSINST) mibs/stripped/SNMPv2-MIB /etc/snmp/mibs/SNMPv2-MIB
	$(ROMFSINST) mibs/stripped/SNMP-TARGET-MIB /etc/snmp/mibs/SNMP-TARGET-MIB
	$(ROMFSINST) mibs/stripped/SNMP-VIEW-BASED-ACM-MIB /etc/snmp/mibs/SNMP-VIEW-BASED-ACM-MIB
	$(ROMFSINST) mibs/stripped/SNMP-COMMUNITY-MIB /etc/snmp/mibs/SNMP-COMMUNITY-MIB
	$(ROMFSINST) mibs/stripped/SNMP-FRAMEWORK-MIB /etc/snmp/mibs/SNMP-FRAMEWORK-MIB
	$(ROMFSINST) mibs/stripped/SNMP-MPD-MIB /etc/snmp/mibs/SNMP-MPD-MIB
	$(ROMFSINST) mibs/stripped/SNMP-PROXY-MIB /etc/snmp/mibs/SNMP-PROXY-MIB
	$(ROMFSINST) mibs/stripped/SNMP-USER-BASED-SM-MIB /etc/snmp/mibs/SNMP-USER-BASED-SM-MIB
	$(ROMFSINST) mibs/stripped/RFC1155-SMI /etc/snmp/mibs/RFC1155-SMI
	$(ROMFSINST) mibs/stripped/RFC1213-MIB /etc/snmp/mibs/RFC1213-MIB
	$(ROMFSINST) mibs/stripped/RFC-1215 /etc/snmp/mibs/RFC-1215
	$(ROMFSINST) mibs/stripped/IF-MIB /etc/snmp/mibs/IF-MIB
	$(ROMFSINST) mibs/stripped/IP-MIB /etc/snmp/mibs/IP-MIB
	$(ROMFSINST) mibs/stripped/IPV6-ICMP-MIB /etc/snmp/mibs/IPV6-ICMP-MIB
	$(ROMFSINST) mibs/stripped/IPV6-MIB /etc/snmp/mibs/IPV6-MIB
	$(ROMFSINST) mibs/stripped/IPV6-TCP-MIB /etc/snmp/mibs/IPV6-TCP-MIB
	$(ROMFSINST) mibs/stripped/IPV6-UDP-MIB /etc/snmp/mibs/IPV6-UDP-MIB
	$(ROMFSINST) mibs/stripped/IPV6-TC /etc/snmp/mibs/IPV6-TC
	$(ROMFSINST) mibs/stripped/INET-ADDRESS-MIB /etc/snmp/mibs/INET-ADDRESS-MIB
	$(ROMFSINST) mibs/stripped/IANAifType-MIB /etc/snmp/mibs/IANAifType-MIB
	$(ROMFSINST) mibs/stripped/HOST-RESOURCES-MIB /etc/snmp/mibs/HOST-RESOURCES-MIB
	$(ROMFSINST) mibs/stripped/HOST-RESOURCES-TYPES /etc/snmp/mibs/HOST-RESOURCES-TYPES
	$(ROMFSINST) mibs/stripped/DISMAN-EVENT-MIB /etc/snmp/mibs/DISMAN-EVENT-MIB
	$(ROMFSINST) mibs/stripped/SNMP-NOTIFICATION-MIB /etc/snmp/mibs/SNMP-NOTIFICATION-MIB
	$(ROMFSINST) mibs/stripped/SNMPv2-TM /etc/snmp/mibs/SNMPv2-TM
	$(ROMFSINST) mibs/stripped/UCD-SNMP-MIB /etc/snmp/mibs/UCD-SNMP-MIB
	$(ROMFSINST) mibs/stripped/UCD-DEMO-MIB /etc/snmp/mibs/UCD-DEMO-MIB
	$(ROMFSINST) mibs/stripped/UCD-DLMOD-MIB /etc/snmp/mibs/UCD-DLMOD-MIB
	$(ROMFSINST) mibs/stripped/NET-SNMP-MIB /etc/snmp/mibs/NET-SNMP-MIB
	$(ROMFSINST) mibs/stripped/NET-SNMP-AGENT-MIB /etc/snmp/mibs/NET-SNMP-AGENT-MIB
	$(ROMFSINST) mibs/stripped/NET-SNMP-EXTEND-MIB /etc/snmp/mibs/NET-SNMP-EXTEND-MIB
endif
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_ETC_CONFIG -s config /etc/snmp

clean:
	rm -rf source build


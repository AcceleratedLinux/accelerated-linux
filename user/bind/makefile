
VERSION = 9.14.2
URL = https://downloads.isc.org/isc/bind9/$(VERSION)/bind-$(VERSION).tar.gz

CONFOPTS =
CONFOPTS += --without-python
CONFOPTS += --without-python
CONFOPTS += --without-gssapi
CONFOPTS += --without-libxml2
CONFOPTS += --disable-linux-caps

MAKEVARS = 

ifndef USER_BIND_FULL_INSTALL
AUTOMAKE_ROMFS = my_romfs
endif

include $(ROOTDIR)/tools/automake.inc

ifndef USER_BIND_FULL_INSTALL
my_romfs:
	$(ROMFSINST) -e CONFIG_USER_BIND_NSUPDATE \
		build/bind-$(VERSION)-install/bin/nsupdate \
		/bin/nsupdate
	$(ROMFSINST) -e CONFIG_USER_BIND_DELV \
		build/bind-$(VERSION)-install/bin/delv \
		/bin/delv
	$(ROMFSINST) -e CONFIG_USER_BIND_MDIG \
		build/bind-$(VERSION)-install/bin/mdig \
		/bin/mdig
	$(ROMFSINST) -e CONFIG_USER_BIND_ARPANAME \
		build/bind-$(VERSION)-install/bin/arpaname \
		/bin/arpaname
	$(ROMFSINST) -e CONFIG_USER_BIND_HOST \
		build/bind-$(VERSION)-install/bin/host \
		/bin/host
	$(ROMFSINST) -e CONFIG_USER_BIND_DIG \
		build/bind-$(VERSION)-install/bin/dig \
		/bin/dig
	$(ROMFSINST) -e CONFIG_USER_BIND_NAMED \
		build/bind-$(VERSION)-install/bin/named-rrchecker \
		/bin/named-rrchecker
	$(ROMFSINST) -e CONFIG_USER_BIND_NSLOOKUP \
		build/bind-$(VERSION)-install/bin/nslookup \
		/bin/nslookup
	$(ROMFSINST) -d -e CONFIG_USER_BIND_NAMED \
		build/bind-$(VERSION)-install/lib/named/filter-aaaa.so \
		/lib/named/filter-aaaa.so
	$(ROMFSINST) -d -e CONFIG_USER_BIND_NAMED \
		build/bind-$(VERSION)-install/sbin/named \
		/sbin/named
	$(ROMFSINST) -e CONFIG_USER_BIND_NAMED \
		build/bind-$(VERSION)-install/sbin/named-checkzone \
		/sbin/named-checkzone
	$(ROMFSINST) -e CONFIG_USER_BIND_RNDC \
		build/bind-$(VERSION)-install/sbin/rndc \
		/sbin/rndc
	$(ROMFSINST) -e CONFIG_USER_BIND_DNSSEC \
		build/bind-$(VERSION)-install/sbin/dnssec-signzone \
		/sbin/dnssec-signzone
	$(ROMFSINST) -e CONFIG_USER_BIND_DNSSEC \
		build/bind-$(VERSION)-install/sbin/dnssec-dsfromkey \
		/sbin/dnssec-dsfromkey
	$(ROMFSINST) -e CONFIG_USER_BIND_DNSSEC \
		build/bind-$(VERSION)-install/sbin/dnssec-keyfromlabel \
		/sbin/dnssec-keyfromlabel
	$(ROMFSINST) -e CONFIG_USER_BIND_DNSSEC \
		build/bind-$(VERSION)-install/sbin/dnssec-settime \
		/sbin/dnssec-settime
	$(ROMFSINST) -e CONFIG_USER_BIND_DNSSEC \
		build/bind-$(VERSION)-install/sbin/dnssec-cds \
		/sbin/dnssec-cds
	$(ROMFSINST) -e CONFIG_USER_BIND_DNSSEC \
		build/bind-$(VERSION)-install/sbin/dnssec-importkey \
		/sbin/dnssec-importkey
	$(ROMFSINST) -e CONFIG_USER_BIND_DNSSEC \
		build/bind-$(VERSION)-install/sbin/dnssec-revoke \
		/sbin/dnssec-revoke
	$(ROMFSINST) -e CONFIG_USER_BIND_DNSSEC \
		build/bind-$(VERSION)-install/sbin/dnssec-keygen \
		/sbin/dnssec-keygen
	$(ROMFSINST) -e CONFIG_USER_BIND_DNSSEC \
		build/bind-$(VERSION)-install/sbin/dnssec-verify \
		/sbin/dnssec-verify
	$(ROMFSINST) -e CONFIG_USER_BIND_DNSSEC \
		build/bind-$(VERSION)-install/sbin/nsec3hash \
		/sbin/nsec3hash
endif

SAMBA = samba-4.15.2
URL = https://download.samba.org/pub/samba/stable/$(SAMBA).tar.gz

$(SAMBA)_NODEFCONF = 1
$(SAMBA)_CONFOPTS += --cross-compile
$(SAMBA)_CONFOPTS += --cross-answers=$(ROOTDIR)/user/samba/cross.answers
$(SAMBA)_CONFOPTS += --hostcc=gcc
$(SAMBA)_CONFOPTS += --disable-avahi
$(SAMBA)_CONFOPTS += --disable-cups
$(SAMBA)_CONFOPTS += --disable-iprint
$(SAMBA)_CONFOPTS += --disable-glusterfs
$(SAMBA)_CONFOPTS += --disable-python
$(SAMBA)_CONFOPTS += --disable-rpath
$(SAMBA)_CONFOPTS += --disable-rpath-install
$(SAMBA)_CONFOPTS += --without-acl-support
$(SAMBA)_CONFOPTS += --without-ad-dc
$(SAMBA)_CONFOPTS += --without-ads
$(SAMBA)_CONFOPTS += --without-dmapi
$(SAMBA)_CONFOPTS += --without-fam
$(SAMBA)_CONFOPTS += --without-gpgme
$(SAMBA)_CONFOPTS += --without-json
$(SAMBA)_CONFOPTS += --without-ldap
$(SAMBA)_CONFOPTS += --without-ldb-lmdb
$(SAMBA)_CONFOPTS += --without-libarchive
$(SAMBA)_CONFOPTS += --without-pam
$(SAMBA)_CONFOPTS += --without-regedit
$(SAMBA)_CONFOPTS += --without-systemd
$(SAMBA)_CONFOPTS += --bundled-libraries='!asn1_compile,!compile_et'
$(SAMBA)_CONFOPTS += --prefix=''
$(SAMBA)_CONFOPTS += --exec-prefix=''
$(SAMBA)_CONFOPTS += --sbindir='/bin'
$(SAMBA)_CONFOPTS += --sysconfdir='/etc/config'
$(SAMBA)_CONFOPTS += --with-configdir='/etc/config'
$(SAMBA)_CONFOPTS += --with-smbpasswd-file='/etc/smbpasswd'

include $(ROOTDIR)/tools/automake.inc

romfs:
	$(ROMFSINST) -R /share/


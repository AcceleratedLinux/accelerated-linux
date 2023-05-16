VERSION         = 4.2.8p15
DOWNLOADHASH    = e1e6b23d2fc75cced41801dbcd6c2561
URL             = https://ftp.osuosl.org/pub/blfs/conglomeration/ntp/ntp-$(VERSION).tar.gz
CONFOPTS       += --with-yielding-select=yes --sbindir=/bin
CONFOPTS       += --without-ntpsnmpd

AUTOMAKE_ROMFS  = my_romfs

# For ntp-4.2.8p13-inline-ntp-syscalls-fallback.patch
AUTORECONF = autoreconf -i

include $(ROOTDIR)/tools/automake.inc

ifndef CONFIG_USER_NTP_ENABLE_HARDENFILE
CONFOPTS += --with-hardenfile=/dev/null
endif

INSTALL_DIR = build/$(PKG)-install

my_romfs:
	$(ROMFSINST) -e CONFIG_USER_NTPD_NTPD \
		$(INSTALL_DIR)/bin/ntpd /bin/ntpd
	$(ROMFSINST) -e CONFIG_USER_NTPD_TICKADJ \
		$(INSTALL_DIR)/bin/tickadj /bin/tickadj
	$(ROMFSINST) -e CONFIG_USER_NTPD_CALC_TICKADJ \
		$(INSTALL_DIR)/bin/calc_tickadj /bin/calc_tickadj
	$(ROMFSINST) -e CONFIG_USER_NTPD_NTP_KEYGEN \
		$(INSTALL_DIR)/bin/ntp-keygen /bin/ntp-keygen
	$(ROMFSINST) -e CONFIG_USER_NTPD_NTPTRACE \
		$(INSTALL_DIR)/bin/ntptrace /bin/ntptrace
	$(ROMFSINST) -e CONFIG_USER_NTPD_SNTP \
		$(INSTALL_DIR)/bin/sntp /bin/sntp
	$(ROMFSINST) -e CONFIG_USER_NTPD_NTPDATE \
		$(INSTALL_DIR)/bin/ntpdate /bin/ntpdate
	$(ROMFSINST) -e CONFIG_USER_NTPD_NTP_WAIT \
		$(INSTALL_DIR)/bin/ntp-wait /bin/ntp-wait
	$(ROMFSINST) -e CONFIG_USER_NTPD_NTPQ \
		$(INSTALL_DIR)/bin/ntpq /bin/ntpq
	$(ROMFSINST) -e CONFIG_USER_NTPD_NTPDC \
		$(INSTALL_DIR)/bin/ntpdc /bin/ntpdc
	$(ROMFSINST) -e CONFIG_USER_NTPD_UPDATE_LEAP \
		$(INSTALL_DIR)/bin/update-leap /bin/update-leap

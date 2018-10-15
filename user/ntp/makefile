
#URL             = http://archive.ntp.org/ntp4/ntp-4.2/ntp-4.2.8p4.tar.gz
URL             = https://ftp.osuosl.org/pub/blfs/conglomeration/ntp/ntp-4.2.8p4.tar.gz
CONFOPTS       += --with-yielding-select=yes --sbindir=/bin
AUTOMAKE_ROMFS  = my_romfs

include $(ROOTDIR)/tools/automake.inc

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

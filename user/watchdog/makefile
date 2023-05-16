VERSION = 3.5
URL = https://github.com/troglobit/watchdogd
METHOD = git

AUTORECONF = NOCONFIGURE=yes ./autogen.sh

CONFOPTS += --with-generic --with-loadavg --with-filenr --with-meminfo

AUTOMAKE_ROMFS = watchdog_romfs
INSTALLDIR = build/watchdogd-install

include $(ROOTDIR)/tools/automake.inc

watchdog_romfs:
	$(ROMFSINST) -v -f $(INSTALLDIR)/bin /bin
	$(ROMFSINST) -d -p 555 $(INSTALLDIR)/sbin/watchdogd /sbin/watchdog_adv
	$(ROMFSINST) -d $(INSTALLDIR)/lib/libwdog.so /lib/libwdog.so


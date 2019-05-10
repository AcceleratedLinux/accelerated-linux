
VERSION  = 5.52
URL      = http://www.stunnel.org/downloads/archive/5.x/stunnel-$(VERSION).tar.gz
CONFOPTS = --with-ssl=$(STAGEDIR) --with-random=/dev/urandom --prefix=
CONFOPTS += --sysconfdir=/etc
CONFVARS = ax_cv_check_cflags___fstack_protector=no

# CONFOPTS += --disable-shared --enable-static

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) -d build/stunnel-$(VERSION)-install/bin/stunnel /bin/stunnel
	$(ROMFSINST) -d build/stunnel-$(VERSION)-install/lib/stunnel/libstunnel.so /lib/stunnel/libstunnel.so
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
	$(ROMFSINST) -s /etc/config /etc/stunnel
endif

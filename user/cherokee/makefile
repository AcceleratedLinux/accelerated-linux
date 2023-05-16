PKG                 = cherokee-webserver
URL                 = https://github.com/cherokee/webserver/tarball/51f13b9535e652421c128ef541371854637ac32e
DOWNLOADNAME        = cherokee-webserver-51f13b9.tgz
DOWNLOADHASH        = d49547fb2dec1ba103b3fe06bbd87544a57ece42f1ffb24a98ff1a5fca189619
BUILDDIR            = cherokee-webserver-51f13b9
ACL_LICENSE         = GPLv2

LIBTOOL = $(abspath $(CURDIR)/build/$(BUILDDIR)/libtool)

AUTOMAKE_ROMFS      = cherokee_romfs

CONFOPTS +=  --host="$(MACHINE)"
CONFOPTS += --sysconfdir=/etc/config
CONFOPTS += --disable-nls -disable-pam --disable-shared --disable-admin
CONFOPTS += --enable-static-module=all --disable-trace
CONFOPTS += --with-python=/usr/bin/python2
CONFVARS += ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes
LIBS += -lssl -lcrypto
AUTORECONF = $(CONFVARS) ./autogen.sh $(CONFOPTS)

include $(ROOTDIR)/tools/automake.inc

cherokee_romfs:
	$(ROMFSINST) build/$(PKG)-install/sbin/cherokee /sbin/cherokee
	$(ROMFSINST) build/$(PKG)-install/sbin/cherokee-worker /sbin/cherokee-worker
	$(ROMFSINST) -d build/$(PKG)-install/share/cherokee/themes/white/entry.html  /etc/cherokee/themes/white/entry.html
	$(ROMFSINST) -d build/$(PKG)-install/share/cherokee/themes/white/footer.html /etc/cherokee/themes/white/footer.html
	$(ROMFSINST) -d build/$(PKG)-install/share/cherokee/themes/white/header.html /etc/cherokee/themes/white/header.html
	$(ROMFSINST) -d build/$(PKG)-install/share/cherokee/themes/white/theme.css   /etc/cherokee/themes/white/theme.css
	$(ROMFSINST) -a http:unknown:/sbin/cherokee /etc/default/inittab.d/http


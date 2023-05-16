VERSION = 8.45
URL = https://sourceforge.net/projects/pcre/files/pcre/$(VERSION)/pcre-$(VERSION).tar.gz
DOWNLOADHASH = 4e6ce03e0336e8b4a3d6c2b70b1c5e18590a5673a98186da90d4f33c23defc09

MAKEVARS =
CONFOPTS = --disable-cpp --enable-utf --enable-unicode-properties
ROMFS = pcre_cleanup
include $(ROOTDIR)/tools/automake.inc

pcre_cleanup:
	@for i in build/pcre-*-install/bin/*; do \
		[ -f "$$i" ] || continue; \
		$(ROMFSINST) -R "/bin/`basename $$i`"; \
	done

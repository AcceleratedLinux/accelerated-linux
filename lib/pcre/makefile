VERSION = 8.44
URL = https://ftp.pcre.org/pub/pcre/pcre-$(VERSION).tar.gz
DOWNLOADHASH = aecafd4af3bd0f3935721af77b889d9024b2e01d96b58471bd91a3063fb47728

MAKEVARS =
CONFOPTS = --disable-cpp --enable-utf --enable-unicode-properties
ROMFS = pcre_cleanup
include $(ROOTDIR)/tools/automake.inc

pcre_cleanup:
	@for i in build/pcre-*-install/bin/*; do \
		[ -f "$$i" ] || continue; \
		$(ROMFSINST) -R "/bin/`basename $$i`"; \
	done

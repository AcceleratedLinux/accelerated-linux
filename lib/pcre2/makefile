VERSION = 10.42
URL = https://github.com/PCRE2Project/pcre2/releases/download/pcre2-$(VERSION)/pcre2-$(VERSION).tar.gz
MAKEVARS =
CONFOPTS = --disable-cpp --enable-utf --enable-unicode-properties
ROMFS = pcre_cleanup

include $(ROOTDIR)/tools/automake.inc

pcre_cleanup:
	@for i in build/pcre2-*-install/bin/*; do \
		[ -f "$$i" ] || continue; \
		$(ROMFSINST) -R "/bin/`basename $$i`"; \
	done

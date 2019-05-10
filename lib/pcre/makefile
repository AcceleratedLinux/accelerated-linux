URL = ftp://ftp.pcre.org/pub/pcre/pcre-8.42.tar.gz
MAKEVARS =
CONFOPTS = --disable-cpp --enable-utf --enable-unicode-properties
ROMFS = pcre_cleanup
include $(ROOTDIR)/tools/automake.inc

pcre_cleanup:
	@for i in build/pcre-*-install/bin/*; do \
		[ -f "$$i" ] || continue; \
		$(ROMFSINST) -R "/bin/`basename $$i`"; \
	done

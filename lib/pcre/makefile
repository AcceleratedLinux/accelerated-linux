URL = ftp://ftp.pcre.org/pub/pcre/pcre-8.36.tar.gz
MAKEVARS =
pcre-8.36_CONFOPTS = --disable-cpp
ROMFS = pcre_cleanup
include $(ROOTDIR)/tools/automake.inc

pcre_cleanup:
	@for i in build/pcre-*-install/bin/*; do \
		[ -f "$$i" ] || continue; \
		$(ROMFSINST) -R "/bin/`basename $$i`"; \
	done

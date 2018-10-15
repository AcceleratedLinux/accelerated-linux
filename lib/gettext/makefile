
URL = http://ftp.gnu.org/gnu/gettext/gettext-0.16.1.tar.gz

CONFOPTS =  --disable-libasprintf --disable-openmp --disable-rpath 
CONFOPTS += --disable-java --disable-native-java --disable-csharp
CONFOPTS += --disable-relocatable --without-emacs
CONFOPTS += --enable-nls
# CONFOPTS += --with-included-gettext

ROMFS = gettext_romfs

MAKEVARS = 

include $(ROOTDIR)/tools/automake.inc

gettext_romfs:
ifndef CONFIG_LIB_GETTEXT_TOOLS
	@for i in build/gettext-*-install/bin/*; do \
		[ -f "$$i" ] || continue; \
		echo "Removing `basename $$i` from romfs"; \
		rm -f "$(ROMFSDIR)/bin/`basename $$i`"; \
	done
	rm -rf "$(ROMFSDIR)/lib/gettext"
endif

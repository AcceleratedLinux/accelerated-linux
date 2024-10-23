
CONFOPTS := --enable-static
URL := https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz
ROMFS = iconv_romfs

include $(ROOTDIR)/tools/automake.inc

iconv_romfs:
	$(ROMFSINST) -R /lib/preloadable_libiconv.so


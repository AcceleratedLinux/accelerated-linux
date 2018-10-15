
CONFOPTS :=
ifdef CONFIG_LIB_LIBSSL
CONFOPTS += --with-ssl=openssl
else
CONFOPTS += --without-ssl
endif

URL = http://ftp.gnu.org/gnu/wget/wget-1.16.tar.xz

include $(ROOTDIR)/tools/automake.inc

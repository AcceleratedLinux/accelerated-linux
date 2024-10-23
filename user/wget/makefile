
CONFOPTS :=
ifdef CONFIG_LIB_OPENSSL
CONFOPTS += --with-ssl=openssl
CONFVARS  = OPENSSL_CFLAGS=" " OPENSSL_LIBS="-lssl -lcrypto"
else
CONFOPTS += --without-ssl
endif

LDFLAGS += -lpthread

URL = https://ftp.gnu.org/gnu/wget/wget-1.19.5.tar.gz

include $(ROOTDIR)/tools/automake.inc

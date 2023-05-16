VERSION = s20190709
URL = https://github.com/iputils/iputils.git
METHOD = git

ifdef CONFIG_USER_IPUTILS_ARPING
MAKEVARS += BUILD_ARPING=y
endif
ifdef CONFIG_USER_IPUTILS_CLOCKDIFF
MAKEVARS += BUILD_CLOCKDIFF=y
endif
ifdef CONFIG_USER_IPUTILS_PING
MAKEVARS += BUILD_PING=y
endif
ifdef CONFIG_USER_IPUTILS_RARPD
MAKEVARS += BUILD_RARPD=y
endif
ifdef CONFIG_USER_IPUTILS_RDISC
MAKEVARS += BUILD_RDISC=y
endif
ifdef CONFIG_USER_IPUTILS_TFTPD
MAKEVARS += BUILD_TFTPD=y
endif
ifdef CONFIG_USER_IPUTILS_TRACEPATH
MAKEVARS += BUILD_TRACEPATH=y
endif
ifdef CONFIG_USER_IPUTILS_TRACEROUTE6
MAKEVARS += BUILD_TRACEROUTE6=y
endif

ifdef CONFIG_LIB_OPENSSL
MAKEVARS += HAVE_OPENSSL=y
endif
ifdef CONFIG_LIB_LIBCAP
MAKEVARS += HAVE_LIBCAP=y
endif
ifdef CONFIG_LIB_LIBIDN2
ifdef CONFIG_DEFAULTS_LIBC_GLIBC
# iputil's IDN support requires some glibc extensions to getaddrinfo()
MAKEVARS += HAVE_LIBIDN=y
endif
endif

MAKEVARS += INSTALLDIR="$(shell pwd)/build/iputils-install"
CONFIGURE = :
CFLAGS += -DPACKAGE_VERSION="\"$(VERSION)\""
LDLIBS += -lm

include $(ROOTDIR)/tools/automake.inc

iputils_ROMFS = link_romfs

link_romfs:
	$(ROMFSINST) -e CONFIG_USER_IPUTILS_PING -m -C cap_net_raw,cap_net_admin+ep /bin/ping
	$(ROMFSINST) -e CONFIG_USER_IPUTILS_PING6 -s /bin/ping /bin/ping6
	$(ROMFSINST) -e CONFIG_USER_IPUTILS_TRACEPATH6 -s /bin/tracepath /bin/tracepath6

VERSION = 1.6
URL = https://download.aircrack-ng.org/aircrack-ng-$(VERSION).tar.gz

CFLAGS += -D_GLIBCXX_HAVE_MBSTATE_T=1 -fno-stack-protector
CXXFLAGS += -D_GLIBCXX_HAVE_MBSTATE_T=1 -fno-stack-protector -lstdc++

# install without prefixes
CONFOPTS += --program-prefix=

# autogen.sh tries to run configure with the wrong arguments,
# tell it not to and let automake do that for us
AUTORECONF = NOCONFIGURE=1 sh ./autogen.sh

# the fast libtool we have in automake/DAL often gets libraries
# mixed up if the libtool use is more complex, and empty MAKEVARS
# disables that tool
MAKEVARS = 

ifndef USER_AIRCRACK_FULL_INSTALL
AUTOMAKE_ROMFS = my_romfs
endif

include $(ROOTDIR)/tools/automake.inc

ifndef USER_AIRCRACK_FULL_INSTALL
my_romfs:
	$(ROMFSINST) -e CONFIG_USER_AIRCRACK_NG_AIRODUMP build/aircrack-ng-$(VERSION)-install/sbin/airodump-ng /sbin/airodump-ng
	$(ROMFSINST) -e CONFIG_USER_AIRCRACK_NG_OSDEP build/aircrack-ng-$(VERSION)-install/lib/libaircrack-osdep-1.6.0.so /lib/libaircrack-osdep-1.6.0.so
endif


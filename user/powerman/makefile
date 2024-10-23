URL = https://github.com/chaos/powerman
METHOD = git
# Version 2.3.27
VERSION = 770066b4e4fa3d29334a7fad9d1a9280f35f2bf7
AUTOMAKE_ROMFS=my_romfs
INSTALLTARGET=my_install

LDFLAGS += -latomic
MAKEVARS =

AUTORECONF := ./autogen.sh
CONFVARS += ac_cv_file__dev_ptmx=yes
CONFVARS += ac_cv_file__dev_ptc=no

CONFOPTS += --program-prefix=
ifdef CONFIG_USER_POWERMAN_HTTPPOWER
CONFOPTS += --with-httppower
endif
ifdef CONFIG_USER_POWERMAN_SNMPPOWER
CONFOPTS += --with-snmppower
endif

include $(ROOTDIR)/tools/automake.inc

build/powerman-my_install: build/powerman-$(BUILDTARGET)
	ln -sf "`pwd`/build/powerman/libpowerman/.libs/libpowerman.so.0.0.0" "$(STAGEDIR)/lib/libpowerman.so"
	ln -sf "`pwd`/build/powerman/libpowerman/libpowerman.h" "$(STAGEDIR)/include/libpowerman.h"

my_romfs:
	for t in $(wildcard build/powerman/etc/*); do $(ROMFSINST) -d $$t /etc/powerman/`basename $$t`; done
	for t in $(wildcard custom_scripts/*); do $(ROMFSINST) -d $$t /etc/powerman/`basename $$t`; done
	$(ROMFSINST) build/powerman/libpowerman/.libs/libpowerman.so.0.0.0 /lib/libpowerman.so.0.0.0
	$(ROMFSINST) -s /lib/libpowerman.so.0.0.0 /lib/libpowerman.so.0
	$(ROMFSINST) -s /lib/libpowerman.so.0.0.0 /lib/libpowerman.so
	$(ROMFSINST) build/powerman/powerman/powerman /bin/powerman
	$(ROMFSINST) build/powerman/powermand/powermand /bin/powermand
	$(ROMFSINST) -e CONFIG_USER_POWERMAN_HTTPPOWER build/powerman/httppower/httppower /bin/httppower
	$(ROMFSINST) -e CONFIG_USER_POWERMAN_SNMPPOWER build/powerman/snmppower/snmppower /bin/snmppower

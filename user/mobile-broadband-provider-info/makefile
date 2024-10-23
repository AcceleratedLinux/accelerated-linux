## ADD Telstra Mobile
## ADD Wildcards instead of unknown
## country codes to dialing prefix http://www.worldatlas.com/aatlas/ctycodes.htm

URL    = https://gitlab.gnome.org/GNOME/mobile-broadband-provider-info.git
VERSION = 1bd3cdc11a9939405e5a6c11948cb2a2ddcf041c
METHOD = git
CONFIGURE = autogen.sh
mobile-broadband-provider-info_CLEAN = my_clean

ifndef USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_COMPLETE
AUTOMAKE_ROMFS = my_romfs
endif

include $(ROOTDIR)/tools/automake.inc

COUNTRY_FILTER_y = 
COUNTRY_FILTER_$(CONFIG_USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_AU) += au
COUNTRY_FILTER_$(CONFIG_USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_US) += us
COUNTRY_FILTER_$(CONFIG_USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_CA) += ca

DIGI_VENTUS_$(CONFIG_DEFAULTS_DIGI_IX10_V)                   = y
DIGI_VENTUS_$(CONFIG_DEFAULTS_DIGI_IX20_V)                   = y
DIGI_VENTUS_$(CONFIG_DEFAULTS_DIGI_IX20W_V)                  = y
DIGI_VENTUS_$(CONFIG_DEFAULTS_DIGI_IX30_V)                   = y
DIGI_VENTUS_$(CONFIG_DEFAULTS_DIGI_EX12_V)                   = y
DIGI_VENTUS_$(CONFIG_DEFAULTS_DIGI_EX15_V)                   = y
DIGI_VENTUS_$(CONFIG_DEFAULTS_DIGI_EX15W_V)                  = y

ifdef CONFIG_USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_PARTIAL
SPACE =
SPACE +=
FILTER = (egrep '^($(subst $(SPACE),|,$(COUNTRY_FILTER_y))),' ; true)
else
FILTER = cat
endif

my_clean:
	rm -f serviceproviders-local.txt serviceproviders.txt

serviceproviders-ventus.txt: add-iccid-prefixes.awk local_database/serviceproviders-ventus.xml country-dialing-codes.txt
	xsltproc serviceproviders.xsl \
		local_database/serviceproviders-ventus.xml | $(FILTER) \
		| awk -f add-iccid-prefixes.awk \
		> serviceproviders-ventus.txt
ifdef CONFIG_USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_UNKNOWN
	cp serviceproviders-ventus.txt serviceproviders-ventus.txt.orig
	awk -F , '{ print "*,*,"$$3","$$4"," }' < serviceproviders-ventus.txt.orig|\
		sort | uniq >> serviceproviders-ventus.txt
	rm -f serviceproviders-ventus.txt.orig
endif

serviceproviders-local.txt: add-iccid-prefixes.awk local_database/serviceproviders-local.xml country-dialing-codes.txt
	xsltproc serviceproviders.xsl \
		local_database/serviceproviders-local.xml | $(FILTER) \
		| awk -f add-iccid-prefixes.awk \
		> serviceproviders-local.txt
ifdef CONFIG_USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_UNKNOWN
	cp serviceproviders-local.txt serviceproviders-local.txt.orig
	awk -F , '{ print "*,*,"$$3","$$4"," }' < serviceproviders-local.txt.orig|\
		sort | uniq >> serviceproviders-local.txt
	rm -f serviceproviders-local.txt.orig
endif

serviceproviders.txt: add-iccid-prefixes.awk build/mobile-broadband-provider-info/serviceproviders.xml country-dialing-codes.txt
	xsltproc serviceproviders.xsl \
		build/mobile-broadband-provider-info/serviceproviders.xml | $(FILTER) \
		| awk -f add-iccid-prefixes.awk \
		> serviceproviders.txt
ifdef CONFIG_USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_UNKNOWN
	cp serviceproviders.txt serviceproviders.txt.orig
	awk -F , '{ print "*,*,"$$3","$$4"," }' < serviceproviders.txt.orig|\
		sort | uniq >> serviceproviders.txt
	rm -f serviceproviders.txt.orig
endif

ifndef CONFIG_USER_MOBILE_BROADBAND_PROVIDER_INFO_INSTALL_NONE
my_romfs: serviceproviders-ventus.txt serviceproviders-local.txt serviceproviders.txt
	$(ROMFSINST) -e DIGI_VENTUS_y -d serviceproviders-ventus.txt /etc/serviceproviders.txt
	$(ROMFSINST) -E DIGI_VENTUS_y -d serviceproviders-local.txt /etc/serviceproviders-local.txt
	$(ROMFSINST) -E DIGI_VENTUS_y -d serviceproviders.txt /etc/serviceproviders.txt
else
my_romfs:
endif


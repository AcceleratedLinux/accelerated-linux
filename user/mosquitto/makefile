VERSION = 2.0.14
PKG = mosquitto-$(VERSION)
URL = https://mosquitto.org/files/source/mosquitto-$(VERSION).tar.gz

MAKEVARS =  WITH_MEMORY_TRACKING=no
MAKEVARS += WITH_STRIP=yes
MAKEVARS += WITH_DOCS=no
MAKEVARS += WITH_PERSISTENCE=no
MAKEVARS += WITH_CJSON=no
MAKEVARS += prefix=/usr
MAKEVARS += CROSS_COMPILE=
MAKEVARS += libdir=/lib

ROMFS = mosquitto_romfs

include $(ROOTDIR)/tools/automake.inc

mosquitto_romfs:
	# Remove not required files
	$(ROMFSINST) -R /etc/mosquitto
	$(ROMFSINST) -R /usr/include/mosquitto_broker.h
	$(ROMFSINST) -R /usr/include/mosquitto.h
	$(ROMFSINST) -R /usr/include/mosquitto_plugin.h
	$(ROMFSINST) -R /usr/include/mosquittopp.h
	$(ROMFSINST) -R /usr/include/mqtt_protocol.h


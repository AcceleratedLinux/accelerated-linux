VERSION = v0.0.4
URL = ftp://ftp.netfilter.org/pub/arptables/arptables-$(VERSION).tar.gz

AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) build/arptables-$(VERSION)-install/usr/local/sbin/arptables /sbin/
	$(ROMFSINST) -p 755 arptables-restore /sbin/
	$(ROMFSINST) -p 755 arptables-save /sbin/

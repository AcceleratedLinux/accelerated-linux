
URL = https://matt.ucc.asn.au/dropbear/releases/dropbear-2012.55.tar.gz
AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) -e CONFIG_USER_DROPBEAR_DROPBEAR \
		build/dropbear-2012.55-install/sbin/dropbear /sbin/dropbear
	$(ROMFSINST) -e CONFIG_USER_DROPBEAR_DROPBEARCONVERT \
		build/dropbear-2012.55-install/bin/dropbearconvert /bin/dropbearconvert
	$(ROMFSINST) -e CONFIG_USER_DROPBEAR_DROPBEARKEY \
		build/dropbear-2012.55-install/bin/dropbearkey /bin/dropbearkey
	$(ROMFSINST) -e CONFIG_USER_DROPBEAR_NBCLIENT \
		build/dropbear-2012.55-install/bin/nbclient /bin/nbclient


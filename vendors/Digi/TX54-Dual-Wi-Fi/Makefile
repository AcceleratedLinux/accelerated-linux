#
# Makefile -- Build instructions for Digi/TX54-Dual-Wi-Fi
#

include ../TX54-Dual-Cellular/tx54_common.mak

romfs: romfs.common
	echo "Digi TX54 Dual Wi-Fi" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -p 555 ../TX54-Dual-Cellular/sim /bin/sim1
	$(ROMFSINST) -d -p 555 ../TX54-Single-Cellular/init_gpios /etc/init_gpios

clean: clean.common

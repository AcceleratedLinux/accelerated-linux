#
# Makefile -- Build instructions for Digi/TX54-Single-Cellular-PR
#

include ../TX54-Dual-Cellular/tx54_common.mak

romfs: romfs.common
	echo "Digi TX54 Single Cellular PR" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -p 555 ../TX54-Dual-Cellular/sim /bin/sim
	$(ROMFSINST) -d -p 555 ../TX54-Single-Cellular/init_gpios /etc/init_gpios

clean: clean.common

#
# Makefile -- Build instructions for Digi/TX54-Dual-Cellular-PR
#

include ../TX54-Dual-Cellular/tx54_common.mak

romfs: romfs.common
	echo "Digi TX54 Dual Cellular PR" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -p 555 ../TX54-Dual-Cellular/sim /bin/sim1
	$(ROMFSINST) -p 555 -s /bin/sim1 /bin/sim2
	$(ROMFSINST) -d -p 555 ../TX54-Dual-Cellular/init_gpios /etc/init_gpios

clean: clean.common

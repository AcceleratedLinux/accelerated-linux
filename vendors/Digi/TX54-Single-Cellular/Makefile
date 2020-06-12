#
# Makefile -- Build instructions for Digi/TX54-Single-Cellular
#

include ../TX54-Dual-Cellular/tx54_common.mak

romfs: romfs.common
	echo "Digi TX54 Single Cellular" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -p 555 sim /bin/sim
	$(ROMFSINST) -d -p 555 /etc/init_gpios

clean: clean.common

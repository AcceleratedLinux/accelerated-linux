#
# Makefile -- Build instructions for Digi/TX54-Dual-Cellular
#

include tx54_common.mak

romfs: romfs.common
	echo "Digi TX54 Dual Cellular" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -p 555 sim /bin/sim1
	$(ROMFSINST) -p 555 -s /bin/sim1 /bin/sim2
	$(ROMFSINST) -d -p 555 /etc/init_gpios

clean: clean.common

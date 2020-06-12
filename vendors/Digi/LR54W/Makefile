#
# Makefile -- Build instructions for Digi/LR54W
#

#
# Note: this platform is using ATSHA204 chip bank 2 for signature verification
#

include ../LR54/lr54_common.mak

romfs: romfs.common
	echo "Digi LR54W" > $(ROMFSDIR)/etc/issue
	$(ROMFSINST) -d -p 555 /bin/get_wifi_mac.sh

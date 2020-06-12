METHOD = git
URL = https://github.com/openwrt/mt76.git
VERSION = 3245ca8b8aeb74c404ce28e0de41ce15772022f9

mt76_BUILDVARS = :
mt76_INSTALLVARS = :
mt76_ROMFS = mt76_romfs
SDIR = build/mt76/firmware

include $(ROOTDIR)/tools/automake.inc

#
# We only want the firmware images for now.
#
mt76_romfs:
	$(ROMFSINST) -d $(SDIR)/mt7603_e2.bin /lib/firmware/mt7603_e2.bin


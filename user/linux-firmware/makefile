METHOD = git
URL = git://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git
VERSION = 65b1c68c63f974d72610db38dfae49861117cae2

linux-firmware_INSTALLVARS = :
linux-firmware_ROMFS = linux-firmware_romfs
SDIR = build/linux-firmware

include $(ROOTDIR)/tools/automake.inc

#
# The entire set of linux-firmware is really big. So we only
# install the firmwaree that we actually care about.
#
linux-firmware_romfs:
	$(ROMFSINST) -e CONFIG_R8169 -d $(SDIR)/rt2561.bin /lib/firmware/rt2561.bin
	$(ROMFSINST) -e CONFIG_R8169 -d $(SDIR)/rt2661.bin /lib/firmware/rt2661.bin
	$(ROMFSINST) -e CONFIG_R8169 -d $(SDIR)/rt2870.bin /lib/firmware/rt2870.bin
	$(ROMFSINST) -e CONFIG_R8169 -d $(SDIR)/rt3071.bin /lib/firmware/rt3071.bin
	$(ROMFSINST) -e CONFIG_R8169 -d $(SDIR)/rt3290.bin /lib/firmware/rt3290.bin
	$(ROMFSINST) -e CONFIG_R8169 -d $(SDIR)/rt2561.bin /lib/firmware/rt2561s.bin
	$(ROMFSINST) -e CONFIG_R8169 -d $(SDIR)/rt2860.bin /lib/firmware/rt2860.bin
	$(ROMFSINST) -e CONFIG_R8169 -d $(SDIR)/rt3070.bin /lib/firmware/rt3070.bin
	$(ROMFSINST) -e CONFIG_R8169 -d $(SDIR)/rt3090.bin /lib/firmware/rt3090.bin
	$(ROMFSINST) -e CONFIG_R8169 -d $(SDIR)/rt73.bin /lib/firmware/rt73.bin
	$(ROMFSINST) -e CONFIG_R8169 $(SDIR)/rtl_nic /lib/firmware/rtl_nic
	$(ROMFSINST) -e CONFIG_ATH10K $(SDIR)/ath10k/QCA988X /lib/firmware/ath10k/QCA988X


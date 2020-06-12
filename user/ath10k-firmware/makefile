export ACL_LICENSE = Qualcomm License

METHOD = git
URL = https://github.com/kvalo/ath10k-firmware.git
VERSION = 3766adc7e5e5c1f3c76233d4bea204a181cd8621

ath10k-firmware_BUILDVARS = :
ath10k-firmware_INSTALLVARS = :
ath10k-firmware_ROMFS = ath10k-firmware_romfs
SDIR = build/ath10k-firmware

include $(ROOTDIR)/tools/automake.inc

#
# Only install the firmware we are interrested in.
#
ath10k-firmware_romfs:
	$(ROMFSINST) -e CONFIG_ATH10K_SDIO -d $(SDIR)/QCA9377/hw1.0/board.bin /lib/firmware/ath10k/QCA9377/hw1.0/board-sdio.bin
	$(ROMFSINST) -e CONFIG_ATH10K_SDIO -d $(SDIR)/QCA9377/hw1.0/untested/firmware-sdio-5.bin_WLAN.TF.1.1.1-00061-QCATFSWPZ-1 /lib/firmware/ath10k/QCA9377/hw1.0/firmware-sdio-5.bin


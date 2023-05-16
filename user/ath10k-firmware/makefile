export ACL_LICENSE = Qualcomm License

METHOD = git
URL = https://github.com/kvalo/ath10k-firmware.git
VERSION = 54ec6346a9bc8e615cc04e36c9346b09cfd17aaf

ath10k-firmware_BUILDVARS = :
ath10k-firmware_INSTALLVARS = :
ath10k-firmware_ROMFS = ath10k-firmware_romfs
SDIR = build/ath10k-firmware

include $(ROOTDIR)/tools/automake.inc

#
# Only install the firmware we are interrested in.
#
ath10k-firmware_romfs:
	# Override with new file for board-sdio.bin
	$(ROMFSINST) -e CONFIG_ATH10K_SDIO -d files/QCA9377/hw1.0/board.bin /lib/firmware/ath10k/QCA9377/hw1.0/board-sdio.bin
	$(ROMFSINST) -e CONFIG_ATH10K_SDIO -d $(SDIR)/QCA9377/hw1.0/untested/firmware-sdio-5.bin_WLAN.TF.1.1.1-00061-QCATFSWPZ-1 /lib/firmware/ath10k/QCA9377/hw1.0/firmware-sdio-5.bin


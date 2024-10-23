linux-firmware_METHOD = git
linux-firmware_URL = git://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git
linux-firmware_VERSION = 20220509
linux-firmware_FINALTARGET = lndir

AUTOMAKE_ROMFS = linux-firmware_romfs
SDIR = build/linux-firmware-install/lib/firmware

AUTOMAKE_y = linux-firmware

###############################################################################
# Same firmware for QCA988X as XOS is using
###############################################################################
ifdef CONFIG_USER_LINUX_FIRMWARE_QCA988X_XOS
qca988x_xos_VERSION = 710963fe53ee3f227556d36839df3858daf6e232
qca988x_xos_URL = $(linux-firmware_URL)
qca988x_xos_METHOD = git
qca988x_xos_DOWNLOADNAME = qca988x_xos
qca988x_xos_FINALTARGET = lndir

qca988x_xos_SDIR = build/qca988x_xos-install/lib/firmware

AUTOMAKE_y += qca988x_xos
endif

include $(ROOTDIR)/tools/automake.inc

#
# The entire set of Linux-firmware is really big. So we only
# install the firmware that we actually care about.
#
linux-firmware_romfs:
	$(ROMFSINST) -e CONFIG_R8169 -o '$(CONFIG_USER_LINUX_FIRMWARE_RT2561)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/rt2561.bin /lib/firmware/rt2561.bin
	$(ROMFSINST) -e CONFIG_R8169 -o '$(CONFIG_USER_LINUX_FIRMWARE_RT2661)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/rt2661.bin /lib/firmware/rt2661.bin
	$(ROMFSINST) -e CONFIG_R8169 -o '$(CONFIG_USER_LINUX_FIRMWARE_RT2870)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/rt2870.bin /lib/firmware/rt2870.bin
	$(ROMFSINST) -e CONFIG_R8169 -o '$(CONFIG_USER_LINUX_FIRMWARE_RT3071)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/rt3071.bin /lib/firmware/rt3071.bin
	$(ROMFSINST) -e CONFIG_R8169 -o '$(CONFIG_USER_LINUX_FIRMWARE_RT3290)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/rt3290.bin /lib/firmware/rt3290.bin
	$(ROMFSINST) -e CONFIG_R8169 -o '$(CONFIG_USER_LINUX_FIRMWARE_RT2561S)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/rt2561.bin /lib/firmware/rt2561s.bin
	$(ROMFSINST) -e CONFIG_R8169 -o '$(CONFIG_USER_LINUX_FIRMWARE_RT2860)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/rt2860.bin /lib/firmware/rt2860.bin
	$(ROMFSINST) -e CONFIG_R8169 -o '$(CONFIG_USER_LINUX_FIRMWARE_RT3070)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/rt3070.bin /lib/firmware/rt3070.bin
	$(ROMFSINST) -e CONFIG_R8169 -o '$(CONFIG_USER_LINUX_FIRMWARE_RT3090)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/rt3090.bin /lib/firmware/rt3090.bin
	$(ROMFSINST) -e CONFIG_R8169 -o '$(CONFIG_USER_LINUX_FIRMWARE_RT73)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/rt73.bin /lib/firmware/rt73.bin
	$(ROMFSINST) -e CONFIG_R8169 -o '$(CONFIG_USER_LINUX_FIRMWARE_RTL_NIC)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' $(SDIR)/rtl_nic /lib/firmware/rtl_nic

	$(ROMFSINST) -e CONFIG_ATH10K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA988X)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -O '$(CONFIG_USER_LINUX_FIRMWARE_QCA988X_XOS)' $(SDIR)/ath10k/QCA988X /lib/firmware/ath10k/QCA988X
	$(ROMFSINST) -e CONFIG_ATH10K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA988X_XOS)' $(qca988x_xos_SDIR)/ath10k/QCA988X /lib/firmware/ath10k/QCA988X
	$(ROMFSINST) -e CONFIG_ATH10K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA9984)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' $(SDIR)/ath10k/QCA9984 /lib/firmware/ath10k/QCA9984
	$(ROMFSINST) -e CONFIG_ATH10K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA6174)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' files/QCA6174 /lib/firmware/ath10k/QCA6174/hw3.0

	$(ROMFSINST) -e CONFIG_ATH11K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA_BLUETOOTH)' -d $(SDIR)/qca/htbtfw20.tlv /lib/firmware/qca/htbtfw20.tlv
	$(ROMFSINST) -e CONFIG_ATH11K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA_BLUETOOTH)' -d $(SDIR)/qca/htnv20.bin /lib/firmware/qca/htnv20.bin

	$(ROMFSINST) -e CONFIG_IMX_SDMA -o '$(CONFIG_USER_LINUX_FIRMWARE_SDMA_IMX6Q)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/imx/sdma/sdma-imx6q.bin /lib/firmware/imx/sdma/sdma-imx6q.bin
	$(ROMFSINST) -e CONFIG_MT76x2_COMMON -o '$(CONFIG_USER_LINUX_FIRMWARE_MT7662)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' $(SDIR)/mt7662.bin /lib/firmware/mt7662.bin
	$(ROMFSINST) -e CONFIG_MT76x2_COMMON -o '$(CONFIG_USER_LINUX_FIRMWARE_MT7662_ROM_PATCH)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' $(SDIR)/mt7662_rom_patch.bin /lib/firmware/mt7662_rom_patch.bin
	$(ROMFSINST) -e CONFIG_WLAN_VENDOR_MARVELL -o '$(CONFIG_USER_LINUX_FIRMWARE_MARVELL_SD8887)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/mrvl/sd8887_uapsta.bin /lib/firmware/mrvl/sd8887_uapsta.bin
	$(ROMFSINST) -e CONFIG_WLAN_VENDOR_INTEL -o '$(CONFIG_USER_LINUX_FIRMWARE_AX210)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/iwlwifi-ty-a0-gf-a0-71.ucode /lib/firmware/iwlwifi-ty-a0-gf-a0-71.ucode
	$(ROMFSINST) -e CONFIG_WLAN_VENDOR_INTEL -o '$(CONFIG_USER_LINUX_FIRMWARE_AX210)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/iwlwifi-ty-a0-gf-a0.pnvm /lib/firmware/iwlwifi-ty-a0-gf-a0.pnvm
	@for i in $(SDIR)/mediatek/*7615*; do \
		$(ROMFSINST) -d -e CONFIG_MT7615E -o '$(CONFIG_USER_LINUX_FIRMWARE_MT7915)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' $$i /lib/firmware/mediatek/`basename $$i`; \
	done
	@for i in $(SDIR)/mediatek/*7915*; do \
		$(ROMFSINST) -d -e CONFIG_MT7915E -o '$(CONFIG_USER_LINUX_FIRMWARE_MT7915)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' $$i /lib/firmware/mediatek/`basename $$i`; \
	done
	$(ROMFSINST) -d -e CONFIG_MODULES_NXP_MWIFIEX -o '$(CONFIG_USER_LINUX_FIRMWARE_SDUART_N61X)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d files/nxp/sduart_nw61x_v1.bin.se /lib/firmware/nxp/sduart_nw61x_v1.bin.se

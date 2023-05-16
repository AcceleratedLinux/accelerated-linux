linux-firmware_METHOD = git
linux-firmware_URL = git://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git
linux-firmware_VERSION = 20220509
linux-firmware_FINALTARGET = lndir

AUTOMAKE_ROMFS = linux-firmware_romfs
SDIR = build/linux-firmware-install/lib/firmware

###############################################################################
# Candelatech Qualcomm firmwares
###############################################################################
CT_BASE_URL = https://www.candelatech.com/downloads
FINALTARGET = ct_installed
CT_PACKAGES =

###############################################################################
ifdef CONFIG_USER_LINUX_FIRMWARE_QCA9984_CT

QCA9984_VERSION = 015
QCA9984_FW_FILE = firmware-5-ct-full-community-12.bin-lede.$(QCA9984_VERSION)
QCA9984_URL = $(CT_BASE_URL)/ath10k-9984-10-4b/$(QCA9984_FW_FILE)
QCA9984_DOWNLOADHASH = c2f04da3285aad37baef9c37c9905a31927175dd234cd4378fd56c106e5c9379

QCA9984_DST_DIR_NAME = hw1.0
QCA9984_BOARD_BIN = board-2.bin

CT_PACKAGES += QCA9984
endif

###############################################################################
ifdef CONFIG_USER_LINUX_FIRMWARE_QCA988X_CT

QCA988X_VERSION = 015
QCA988X_FW_FILE = firmware-2-ct-full-community-22.bin.lede.$(QCA988X_VERSION)
QCA988X_URL = $(CT_BASE_URL)/$(QCA988X_FW_FILE)
QCA988X_DOWNLOADHASH = a3a760d0d72707feffa496b6d2266bd9195f09806553b36420b60c0f12b8b87e

QCA988X_DST_DIR_NAME = hw2.0
QCA988X_BOARD_BIN = board.bin

CT_PACKAGES += QCA988X
endif

###############################################################################

# linux-firmware has to be built first, as CT builds are depending on it
AUTOMAKE_y = linux-firmware $(CT_PACKAGES)

include $(ROOTDIR)/tools/automake.inc


define ct_install_dep
$(1)_METHOD = wget
$(1)_DOWNLOADNAME = $($(1)_FW_FILE).$(1)
.PHONY: build/$(1)-ct_installed
build/$(1)-ct_installed: downloads/$($(1)_DOWNLOADNAME)
	$(AT)echo "Verifying $$< using $($(1)_DOWNLOADHASH_METHOD) hash ..."
	$(AT)HASH=`openssl dgst -$($(1)_DOWNLOADHASH_METHOD) $$< | cut -d' ' -f 2`; \
	if [ "$$$$HASH" = "$($(1)_DOWNLOADHASH)" ]; then \
		echo "Download file $$($(1)_DOWNLOADNAME) passed hash verification"; \
	else \
		echo "Download file $$($(1)_DOWNLOADNAME) failed hash verification"; \
		echo "    Expected = $($(1)_DOWNLOADHASH)"; \
		echo "    Got      = $$$$HASH"; \
		exit 1; \
	fi

	$(AT)echo "Copying CT firmware $$< ..."
	$(AT)rm -Rf build/$(1)-install
	$(AT)mkdir -p build/$(1)-install/lib/firmware/ath10k/$(1)/$($(1)_DST_DIR_NAME)
	$(AT)name=$($(1)_FW_FILE); cp $$< build/$(1)-install/lib/firmware/ath10k/$(1)/$($(1)_DST_DIR_NAME)/$$$${name%%-ct*}.bin
	$(AT)cp build/linux-firmware-install/lib/firmware/ath10k/$(1)/$($(1)_DST_DIR_NAME)/$($(1)_BOARD_BIN) build/$(1)-install/lib/firmware/ath10k/$(1)/$($(1)_DST_DIR_NAME)/
	$(AT)touch $$@
endef
$(foreach pkg,$(CT_PACKAGES),$(eval $(call ct_install_dep,$(pkg))))

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

	@# Install only -ct or original firmwares for the ath10k chips
	$(ROMFSINST) -e CONFIG_ATH10K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA988X)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -O '$(CONFIG_USER_LINUX_FIRMWARE_QCA988X_CT)' $(SDIR)/ath10k/QCA988X /lib/firmware/ath10k/QCA988X
	$(ROMFSINST) -e CONFIG_ATH10K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA988X_CT)' build/QCA988X-install/lib/firmware/ath10k/QCA988X /lib/firmware/ath10k/QCA988X
	$(ROMFSINST) -e CONFIG_ATH10K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA9984)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -O '$(CONFIG_USER_LINUX_FIRMWARE_QCA9984_CT)' $(SDIR)/ath10k/QCA9984 /lib/firmware/ath10k/QCA9984
	$(ROMFSINST) -e CONFIG_ATH10K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA9984_CT)' -d files/QCA9984/board-2-ct.bin /lib/firmware/ath10k/QCA9984/hw1.0/board-2.bin
	$(ROMFSINST) -e CONFIG_ATH10K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA9984_CT)' -d files/QCA9984/firmware-5-ct-community.bin /lib/firmware/ath10k/QCA9984/hw1.0/firmware-5.bin
	$(ROMFSINST) -e CONFIG_ATH10K -o '$(CONFIG_USER_LINUX_FIRMWARE_QCA6174)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' files/QCA6174 /lib/firmware/ath10k/QCA6174/hw3.0

	$(ROMFSINST) -e CONFIG_IMX_SDMA -o '$(CONFIG_USER_LINUX_FIRMWARE_SDMA_IMX6Q)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/imx/sdma/sdma-imx6q.bin /lib/firmware/imx/sdma/sdma-imx6q.bin
	$(ROMFSINST) -e CONFIG_MT76x2_COMMON -o '$(CONFIG_USER_LINUX_FIRMWARE_MT7662)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' $(SDIR)/mt7662.bin /lib/firmware/mt7662.bin
	$(ROMFSINST) -e CONFIG_MT76x2_COMMON -o '$(CONFIG_USER_LINUX_FIRMWARE_MT7662_ROM_PATCH)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' $(SDIR)/mt7662_rom_patch.bin /lib/firmware/mt7662_rom_patch.bin
	$(ROMFSINST) -e CONFIG_WLAN_VENDOR_MARVELL -o '$(CONFIG_USER_LINUX_FIRMWARE_MARVELL_SD8887)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/mrvl/sd8887_uapsta.bin /lib/firmware/mrvl/sd8887_uapsta.bin
	$(ROMFSINST) -e CONFIG_WLAN_VENDOR_INTEL -o '$(CONFIG_USER_LINUX_FIRMWARE_AX210)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/iwlwifi-ty-a0-gf-a0-71.ucode /lib/firmware/iwlwifi-ty-a0-gf-a0-71.ucode
	$(ROMFSINST) -e CONFIG_WLAN_VENDOR_INTEL -o '$(CONFIG_USER_LINUX_FIRMWARE_AX210)$(CONFIG_USER_LINUX_FIRMWARE_ALL_FIRMWARE)' -d $(SDIR)/iwlwifi-ty-a0-gf-a0.pnvm /lib/firmware/iwlwifi-ty-a0-gf-a0.pnvm

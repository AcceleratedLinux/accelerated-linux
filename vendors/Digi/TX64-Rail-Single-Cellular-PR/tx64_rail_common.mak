#
# Common makefile for all TX64 Rail variants
#

COMMON_RAIL_DIR = $(ROOTDIR)/vendors/Digi/TX64-Rail-Single-Cellular-PR
SIGNED_GRUB = $(COMMON_RAIL_DIR)/bootx64.efi.signed

include ../TX64/tx64_common.mak

romfs.tx64_rail_common: romfs.common
	$(ROMFSINST) -p 555 $(COMMON_RAIL_DIR)/sim /bin/sim
	$(ROMFSINST) -d -p 555 $(COMMON_DIR)/led_common /etc/action.d/led_common
	$(ROMFSINST) -d -p 555 $(COMMON_RAIL_DIR)/led /etc/action.d/led
	# Add version tag to the GRUB
	sed -e 's/<GRUB_VERSION>/$(VERSIONPKG)/g' <$(COMMON_RAIL_DIR)/grub.cfg.in >$(ROMFSDIR)/boot/grub/grub.cfg

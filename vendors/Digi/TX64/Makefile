#
# Makefile -- Build instructions for Digi/TX64
#

include tx64_common.mak

romfs: romfs.common
	$(ROMFSINST) -p 555 sim /bin/sim1
	$(ROMFSINST) -p 555 -s /bin/sim1 /bin/sim2
	$(ROMFSINST) -d -p 555 led_common /etc/action.d/led
	# Add version tag to the GRUB
	sed -e 's/<GRUB_VERSION>/$(VERSIONPKG)/g' <$(COMMON_DIR)/grub.cfg.in >$(ROMFSDIR)/boot/grub/grub.cfg
	echo "Digi TX64" > $(ROMFSDIR)/etc/issue

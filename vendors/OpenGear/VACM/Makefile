# VACM using qcow2 MBR layout
#
#  Part  Name           Description
#        mbr            fsk86 boot sector 0 + partition table
#  sda1  bootarg        1-block partition to hold setfset vars
#  sda2  fsk            FSK image (kernel+rootfs+trailer)
#  sda3  config         /etc/config (empty/ignored by netflash)
#  sda4  nvlog          (unused)
#
# netflash-vm only reads the 'image' partition when flashing on-target.
# The rest of the qcow2 disk exists just to make use in a vm easier.

ZIMAGE    = $(ROOTDIR)/$(LINUXDIR)/arch/x86_64/boot/bzImage

IMAGE_PARTS = image.hdd
IMAGE_CMD = qemu-img convert -f raw -O qcow2 image.hdd $@

include $(ROOTDIR)/vendors/OpenGear/opengear.mk

# qcow2 disk partitioning
BLOCKS_BOOT     =      4
BLOCKS_FSK      =  65532
BLOCKS_CONFIG   =  65536
BLOCKS_NVLOG    = 131072
BLOCKS_TOTAL    = 262144

# an FSK trailer with block-padding, leaving gap at end for 4-byte checksum
image.trailer:
	$(FSK_TRAILER_HEAD) >$@.0
	dd bs=$$(expr 508 - $$(wc -c <$@.0)) count=1 </dev/zero >$@
	cat >>$@ <$@.0
	rm $@.0

# image.fsk is the sda2 partition content (replaceable by netflash)
image.fsk: $(ZIMAGE) $(ROMFSIMG) image.trailer
	dd conv=sync bs=512 if="$(ROMFSIMG)" of=$@
	dd conv=sync bs=512 if="$(ZIMAGE)" >>$@
	dd if=image.trailer bs=512 seek=$$(expr $(BLOCKS_FSK) - 1) of=$@
	$(FSK_TRAILER_CKSUM) $@ >>$@

image.hdd: fsk86.bin image.fsk bootarg-VACM.txt
	dd if=fsk86.bin >$@
	dd if=image.fsk of=$@ bs=512 seek=$(BLOCKS_BOOT)
	truncate -s $$(expr $(BLOCKS_TOTAL) \* 512) $@
	# update partition table inside sector 0
	{ echo "1,1,7f,-"; \
	  echo "$(BLOCKS_BOOT),$(BLOCKS_FSK),83,*"; \
	  echo ",$(BLOCKS_CONFIG),83,-"; \
	  echo ",$(BLOCKS_NVLOG),83,-"; \
	} | sfdisk $(if $V,,--quiet) --force $@
	# update bootarg sector
	{ printf '\0\0\0\0'; \
	  sed -e '/^#/d;/^$$/d' <bootarg-VACM.txt | tr '\n' '\0'; \
	  printf '\0'; \
	} | dd of=$@ conv=sync,notrunc iflag=fullblock seek=1 count=1 bs=512

clean::
	rm -f image.trailer image.fsk image.hdd
romfs::
	$(ROMFSINST) -A "cons:" -a "cons:unknown:/bin/agetty ttyS0 115200" \
                /etc/default/inittab

image::
	@echo "Boot $(IMAGE) with:"; \
	 echo "  qemu-system-$(ARCH)" \
			-display none \
			-serial stdio \
			-hda $(IMAGE) \
		; \
	 echo "and shutdown with: poweroff -f"

fsk86.o: TARGET_MACH= -m16
fsk86.o: fsk86.S
	$(COMPILE.S) -o fsk86.o fsk86.S
fsk86.elf: LDFLAGS = -Bstatic -nostdlib -Wl,-Ttext=0x7c00
fsk86.elf: TARGET_MACH= -m16
fsk86.elf: fsk86.o
	$(LINK.S) -o fsk86.elf fsk86.o
fsk86.bin: fsk86.elf
	$(OBJCOPY) -O binary -j .text fsk86.elf fsk86.bin

clean::
	rm -f fsk86.o fsk86.elf fsk86.bin

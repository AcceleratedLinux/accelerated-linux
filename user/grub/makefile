
GRUB_VERSION = 2.06
URL = https://ftp.gnu.org/gnu/grub/grub-$(GRUB_VERSION).tar.gz

CONFOPTS += --exec-prefix="/usr"
CONFOPTS += --datarootdir="/usr/share"
CONFOPTS += --program-prefix=""
CONFOPTS += --disable-werror
CONFOPTS += --with-libintl-prefix=/foo

ifdef CONFIG_USER_GRUB_EFI
  CONFOPTS += --with-platform=efi
else
  CFLAGS += -Os
endif
ifdef CONFIG_X86
  # i386 and non EFI auto default to i386-pc and cannot be forced with --target
  ifdef CONFIG_64BIT
    ifdef CONFIG_USER_GRUB_EFI
      CONFOPTS += --target=x86_64
    endif
  endif
else
  CONFOPTS += --target=unknown
endif

CONFOPTS +=   --disable-liblzma --disable-libzfs

ifdef CONFIG_USER_GRUB_DISABLE_VGA_CONSOLE
  CONFOPTS += --disable-vga-console
endif

AUTOMAKE_ROMFS = grub_romfs

AUTORECONF = ./autogen.sh

ifneq (,$(CONFIG_DEFAULTS_DIGI_CONNECTIT16)$(CONFIG_DEFAULTS_DIGI_CONNECTIT48))
CFLAGS += -DDIGI_CONNECTITXX
endif

CFLAGS += -fno-stack-protector

include $(ROOTDIR)/tools/automake.inc

#
# We need to make sure that the grub modules are not stripped.
# The normal automake install through romfs-inst will strip them.
#
grub_romfs:
	cd build/grub-$(GRUB_VERSION)-install/usr/lib/grub/ ; \
	ARCH=`ls` ; \
	cd $$ARCH ; \
	for i in * ; do \
		$(ROMFSINST) -d -S /usr/lib/grub/$$ARCH/$$i ; \
	done

	$(ROMFSINST) -d -p 755 build/grub-$(GRUB_VERSION)-install/usr/bin/grub-editenv /usr/bin/grub-editenv
	$(ROMFSINST) -p 755 build/grub-$(GRUB_VERSION)-install/usr/bin/grub-mkstandalone /usr/bin/grub-mkstandalone
	$(ROMFSINST) -d -p 755 build/grub-$(GRUB_VERSION)-install/usr/sbin/grub-install /usr/sbin/grub-install

	ACL_LICENSE=GPLv2 ACL_URL=https://github.com/AcceleratedLinux/accelerated-linux ACL_PKG=fw_env_emulation $(ROMFSINST) -d -e CONFIG_USER_GRUB_ENVTOOLS -p 755 /bin/fw_printenv
	ACL_LICENSE=GPLv2 ACL_URL=https://github.com/AcceleratedLinux/accelerated-linux ACL_PKG=fw_env_emulation $(ROMFSINST) -e CONFIG_USER_GRUB_ENVTOOLS -s /bin/fw_printenv /bin/fw_setenv

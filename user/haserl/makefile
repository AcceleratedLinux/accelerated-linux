HASERL_VERSION = 0.9.36
URL = https://sourceforge.net/projects/haserl/files/haserl-devel/haserl-$(HASERL_VERSION).tar.gz

AUTOMAKE_ROMFS = romfs_exe

include $(ROOTDIR)/tools/automake.inc

romfs_exe:
	$(ROMFSINST) build/haserl-*-install/bin/haserl /bin/haserl

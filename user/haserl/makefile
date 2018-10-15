HASERL_VERSION = 0.9.35
URL = http://downloads.sourceforge.net/project/haserl/haserl-devel/haserl-$(HASERL_VERSION).tar.gz

AUTOMAKE_ROMFS = romfs_exe

include $(ROOTDIR)/tools/automake.inc

romfs_exe:
	$(ROMFSINST) build/haserl-*-install/bin/haserl /bin/haserl

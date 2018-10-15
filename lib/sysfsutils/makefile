URL = http://downloads.sourceforge.net/project/linux-diag/sysfsutils/2.1.0/sysfsutils-2.1.0.tar.gz

sysfsutils-2.1.0_ROMFS = romfs_fixup

include $(ROOTDIR)/tools/automake.inc

romfs_fixup:
	$(ROMFSINST) -E CONFIG_LIB_SYSFSUTILS_GET_DEVICE -R /bin/get_device
	$(ROMFSINST) -E CONFIG_LIB_SYSFSUTILS_GET_MODULE -R /bin/get_module
	$(ROMFSINST) -E CONFIG_LIB_SYSFSUTILS_GET_DRIVER -R /bin/get_driver
	$(ROMFSINST) -E CONFIG_LIB_SYSFSUTILS_DLIST_TEST -R /bin/dlist_test
	$(ROMFSINST) -E CONFIG_LIB_SYSFSUTILS_SYSTOOL -R /bin/systool


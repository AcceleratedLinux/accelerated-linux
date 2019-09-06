export ACL_LICENSE = NXP Binary License
METHOD = git
URL = git://github.com/NXP/qoriq-engine-pfe-bin.git
VERSION = 58aa0cac630528ed91fe5639700d6b571c0c07b5

BUILDVARS = :
AUTOMAKE_ROMFS = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) -d build/qoriq-engine-pfe-bin/ls1012a/slow_path/ppfe_class_ls1012a.elf /lib/firmware/ppfe_class_ls1012a.elf
	$(ROMFSINST) -d build/qoriq-engine-pfe-bin/ls1012a/slow_path/ppfe_tmu_ls1012a.elf /lib/firmware/ppfe_tmu_ls1012a.elf
	$(ROMFSINST) -d build/qoriq-engine-pfe-bin/NXP-Binary-EULA.txt /lib/firmware/NXP-Binary-EULA.txt

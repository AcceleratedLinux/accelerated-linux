URL = https://www.sqlite.org/2023/sqlite-autoconf-3430200.tar.gz
CONFVARS = ac_cv_func_posix_fallocate=no

# Prerequisite for python & frr; do it quickly
MAKEVARS = -j$(HOST_NCPU)

include $(ROOTDIR)/tools/automake.inc

romfs:
	$(ROMFSINST) -R /bin/sqlite3

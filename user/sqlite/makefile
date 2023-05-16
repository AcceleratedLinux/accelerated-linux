URL = http://www.sqlite.org/2022/sqlite-autoconf-3390400.tar.gz
CONFVARS = ac_cv_func_posix_fallocate=no

include $(ROOTDIR)/tools/automake.inc

sqlite_romfs:
	$(ROMFSINST) -d $(INSTALLDIR)/lib/libsqlite3.so /lib/libsqlite3.so

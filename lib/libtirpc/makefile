URL = https://sourceforge.net/projects/libtirpc/files/libtirpc/1.0.3/libtirpc-1.0.3.tar.bz2

CONFOPTS += --disable-gssapi

AUTORECONF = autoreconf -i

AUTOMAKE_PRESTAGE = libtirpc_prestage

include $(ROOTDIR)/tools/automake.inc

#
# The include files are by default installed in their own sub-directory
# under /usr/include/tirpc. We don't want that, we want them to be glibc
# compatible, so move them to /usr/include.
#
libtirpc_prestage:
	echo "Fixup installed include directory..."
	if [ -d build/libtirpc-*-install/include/tirpc ] ; then \
		rm -f build/libtirpc-*-install/include/*.h ; \
		rm -rf build/libtirpc-*-install/include/r* ; \
		mv build/libtirpc-*-install/include/tirpc/* build/libtirpc-*-install/include/ ; \
		rmdir build/libtirpc-*-install/include/tirpc ; \
	fi


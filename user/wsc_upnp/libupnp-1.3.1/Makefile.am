# $Id: Makefile.am,v 1.1.1.1 2007-07-16 03:05:28 yy Exp $
#
# Top-level "Makefile.am" for libupnp
#
# Copyright (C) 2005 Rémi Turboult <r3mi@users.sourceforge.net>
#

ACLOCAL_AMFLAGS = -I m4

SUBDIRS 	= ixml threadutil upnp


EXTRA_DIST	= libupnp.pc.in LICENSE


# This variable must have 'exec' in its name, in order to be installed 
# by 'install-exec' target (instead of default 'install-data')
pkgconfigexecdir 	= $(libdir)/pkgconfig
pkgconfigexec_DATA 	= libupnp.pc

$(pkgconfigexec_DATA): config.status


CLEANFILES	= IUpnpErrFile.txt IUpnpInfoFile.txt






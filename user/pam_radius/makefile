VERSION = 1_4_0
URL = https://github.com/FreeRADIUS/pam_radius/archive/release_$(VERSION).tar.gz

PKG = pam_radius
BUILDDIR = pam_radius-release_$(VERSION)

pam_radius_INSTALLVARS = :

include $(ROOTDIR)/tools/automake.inc

romfs:
	$(ROMFSINST) -d build/$(BUILDDIR)/pam_radius_auth.so /lib/security/pam_radius_auth.so







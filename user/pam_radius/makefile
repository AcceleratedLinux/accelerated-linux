VERSION = 2.0.0
VERSION_ := $(subst .,_,$(VERSION))
URL = https://github.com/FreeRADIUS/pam_radius/archive/release_$(VERSION_).tar.gz

PKG = pam_radius
BUILDDIR = pam_radius-release_$(VERSION_)

pam_radius_INSTALLVARS = :

include $(ROOTDIR)/tools/automake.inc

romfs:
	$(ROMFSINST) -d build/$(BUILDDIR)/pam_radius_auth.so /lib/security/pam_radius_auth.so







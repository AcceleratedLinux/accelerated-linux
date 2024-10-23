

EXT_DBUS_BINDING_TOOL = $(shell which dbus-binding-tool)
ifeq ($(EXT_DBUS_BINDING_TOOL),)
all romfs:
	@echo "--------------------------------------------------------------" >&2
	@echo "You need to install 'libdbus-glib-1-dev' to build this package" >&2
	@echo "--------------------------------------------------------------" >&2
	@exit 1

else

URL      = https://dbus.freedesktop.org/releases/dbus-glib/dbus-glib-0.112.tar.gz

CONFVARS  = ac_cv_have_abstract_sockets=yes
CONFOPTS  = --disable-gtk-doc \
	    --disable-gtk-doc-html \
	    --with-dbus-binding-tool=$(EXT_DBUS_BINDING_TOOL) \
	    --disable-tests \
	    --disable-bash-completion

ifdef CONFIG_LIB_GLIB_STATIC
CONFVARS  += LIBS="-lffi"
endif

MAKEVARS  = 

ROMFS     = dbus-glib_romfs

include $(ROOTDIR)/tools/automake.inc

dbus-glib_romfs:
	$(ROMFSINST) -R  /bin/dbus-binding-tool

endif

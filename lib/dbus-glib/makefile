

EXT_DBUS_BINDING_TOOL = $(shell which dbus-binding-tool)
ifeq ($(EXT_DBUS_BINDING_TOOL),)
all romfs:
	@echo "--------------------------------------------------------------" >&2
	@echo "You need to install 'libdbus-glib-1-dev' to build this package" >&2
	@echo "--------------------------------------------------------------" >&2
	@exit 1

else

URL      = http://dbus.freedesktop.org/releases/dbus-glib/dbus-glib-0.100.2.tar.gz

CONFVARS  = ac_cv_have_abstract_sockets=yes
CONFOPTS  = --disable-gtk-doc-html
CONFOPTS  = --with-dbus-binding-tool=$(EXT_DBUS_BINDING_TOOL)

ifdef CONFIG_LIB_GLIB_STATIC
CONFVARS  += LIBS="-lffi"
endif

MAKEVARS  = 

ROMFS     = dbus-glib_romfs

include $(ROOTDIR)/tools/automake.inc

dbus-glib_romfs:
	$(ROMFSINST) -R  /libexec/dbus-bash-completion-helper
	$(ROMFSINST) -R  /etc/bash_completion.d/dbus-bash-completion.sh
	$(ROMFSINST) -R  /bin/dbus-binding-tool

endif

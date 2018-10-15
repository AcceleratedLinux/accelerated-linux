
URL      = http://dbus.freedesktop.org/releases/dbus/dbus-1.7.0.tar.gz
CONFOPTS = --disable-x11-autolaunch --without-x
ROMFS    = dbus_romfs
ifdef CONFIG_LIB_GLIB_STATIC
MAKEVARS += LIBS="-lffi -lrt"
else
MAKEVARS += LIBS="-lrt"
endif

include $(ROOTDIR)/tools/automake.inc

dbus_romfs:
	$(ROMFSINST) -R /libexec/dbus-bash-completion-helper
	$(ROMFSINST) -R /share/doc/dbus
	$(ROMFSINST) -R /share/gtk-doc
	$(ROMFSINST) -R /lib/dbus-1.0/include/dbus
	$(ROMFSINST) -R /lib/pkgconfig/dbus*
	$(ROMFSINST) -R /bin/gdbus-codegen


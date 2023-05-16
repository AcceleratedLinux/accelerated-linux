
URL      = http://dbus.freedesktop.org/releases/dbus/dbus-1.13.20.tar.xz
CONFOPTS = --disable-x11-autolaunch --without-x --disable-xml-docs --disable-doxygen-docs
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
ifdef CONFIG_PROP_OGCS
	$(ROMFSINST) -dv -f build/dbus-*/bus/system.conf /etc/default/dbus-1/system.conf
	$(ROMFSINST) -p ug=rxs,o=rx,a+w build/dbus-*-install/libexec/dbus-daemon-launch-helper /libexec/dbus-daemon-launch-helper
endif

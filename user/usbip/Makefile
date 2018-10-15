-include ../user.mak

all: usbip-copied usbip-patched usbip-configured usbip-built

usbip-copied:
	mkdir -p build
	cp -ra src/* build/
	touch usbip-copied

usbip-patched:
	for p in patches/*; do patch -p1 -i ../$$p -d build || exit 1 ; done
	touch usbip-patched

usbip-configured:
	cd build && ./autogen.sh
	cd build && ./configure --host=$(ARCH)
	touch usbip-configured

usbip-built:
	make -C build
	touch usbip-built

romfs:
	$(ROMFSINST) -e CONFIG_USER_USBIP_USBIP  build/src/usbip  /bin/usbip
	$(ROMFSINST) -e CONFIG_USER_USBIP_USBIPD build/src/usbipd /bin/usbipd
	$(ROMFSINST) -d -s /var/run/usb.ids /usr/share/hwdata/usb.ids

clean:
	rm -rf build usbip-copied usbip-patched usbip-configured usbip-built

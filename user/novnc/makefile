URL      = https://github.com/kanaka/noVNC.git
VERSION  = 6a90803feb124791960e3962e328aa3cfb729aeb
PATCHES  =

.PHONY: romfs clean

all: clone build build/patched

build:
	mkdir -p build/noVNC
	cp -r clone/* build/noVNC

build/patched:
	cd build/noVNC ; $(foreach var,$(PATCHES),patch -p1 -fi ../../patches/$(var) ; )
	touch build/patched

romfs:
	# Cherry pick the important files.
	$(ROMFSINST) -d build/noVNC/include/util.js      $(CONFIG_USER_NOVNC_ROMFSDIR)/include/util.js
	$(ROMFSINST) -d build/noVNC/include/webutil.js   $(CONFIG_USER_NOVNC_ROMFSDIR)/include/webutil.js
	$(ROMFSINST) -d build/noVNC/include/base64.js    $(CONFIG_USER_NOVNC_ROMFSDIR)/include/base64.js
	$(ROMFSINST) -d build/noVNC/include/websock.js   $(CONFIG_USER_NOVNC_ROMFSDIR)/include/websock.js
	$(ROMFSINST) -d build/noVNC/include/des.js       $(CONFIG_USER_NOVNC_ROMFSDIR)/include/des.js
	$(ROMFSINST) -d build/noVNC/include/keysymdef.js $(CONFIG_USER_NOVNC_ROMFSDIR)/include/keysymdef.js
	$(ROMFSINST) -d build/noVNC/include/keyboard.js  $(CONFIG_USER_NOVNC_ROMFSDIR)/include/keyboard.js
	$(ROMFSINST) -d build/noVNC/include/input.js     $(CONFIG_USER_NOVNC_ROMFSDIR)/include/input.js
	$(ROMFSINST) -d build/noVNC/include/display.js   $(CONFIG_USER_NOVNC_ROMFSDIR)/include/display.js
	$(ROMFSINST) -d build/noVNC/include/inflator.js  $(CONFIG_USER_NOVNC_ROMFSDIR)/include/inflator.js
	$(ROMFSINST) -d build/noVNC/include/rfb.js       $(CONFIG_USER_NOVNC_ROMFSDIR)/include/rfb.js
	$(ROMFSINST) -d build/noVNC/include/keysym.js    $(CONFIG_USER_NOVNC_ROMFSDIR)/include/keysym.js

clone:
	git clone $(URL) clone
	cd clone &&	git checkout -q $(VERSION)

clean:
	rm -rf build

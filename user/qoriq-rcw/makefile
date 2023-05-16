PKG         = rcw
URL         = git://source.codeaurora.org/external/qoriq/qoriq-components/rcw.git
VERSION     = 1f43bef4b4475d8e81c9b3b8c5bdd6c1ce8cfa6c # 3/4/21

.PHONY: compile romfs

all: clone build build/patched compile

compile:
	make -C build/rcw

build:
	mkdir build
	mv $(PKG) build/
	cp -R platforms/* build/rcw/

build/patched:
	cd build/$(PKG) ; $(foreach var,$(sort $(wildcard ./patches/*)),patch -p1 -fi ../../$(var) ; )
	touch build/patched

romfs:

clone:
	git clone $(URL)
	pushd $(PKG) && git checkout -q $(VERSION) && popd

clean:
	rm -rf build rcw


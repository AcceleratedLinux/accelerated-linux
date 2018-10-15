URL      = https://github.com/kanaka/websockify.git
VERSION  = 8dacbc974cb5a49dd7cddbcfbd1966d1dbbd700f

CFLAGS = -I$(STAGEDIR)/include
LDFLAGS = -L$(STAGEDIR)/libs

.PHONY: compile stage romfs

all: clone build build/patched compile stage

compile:
	make -C build/websockify/other

build:
	mkdir -p build/websockify
	cp -r clone/* build/websockify

build/patched:
	cd build/websockify ; $(foreach var,$(wildcard ./patches/*),patch -p1 -fi ../../$(var) ; )
	touch build/patched

stage:

romfs:
	$(ROMFSINST) -d build/websockify/other/websockify /bin/websockify

clone:
	git clone $(URL) clone
	cd clone &&	git checkout -q $(VERSION)

clean:
	rm -rf build


all: build build/ipsetd

build:
	mkdir build

build/ipsetd:	build/ipsetd.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS$(LDLIBS_$@))

build/%.o:	%.c
	$(CC) $(CFLAGS) -o $@ -c $<

romfs:
	$(ROMFSINST) build/ipsetd /bin/ipsetd

clean:
	rm -rf build


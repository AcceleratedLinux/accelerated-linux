include ../user.mak

CFLAGS += -O2
CFLAGS += -g
CFLAGS += -I.
CFLAGS += -I/include/libnl3
LIBS += -lnl-3
LIBS += -lnl-genl-3
LIBS += -luci

all: swconfig

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $^

libsw.so: swlib.o
	$(CC) $(CFLAGS) -fPIC -shared -o $@ swlib.o

swconfig: libsw.so cli.o uci.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) -L./ -lsw $(LIBS)

romfs:
	$(ROMFSINST) /lib/libsw.so
	$(ROMFSINST) /sbin/swconfig

clean:
	-rm -f swconfig *.elf *.gdb *.o *.so

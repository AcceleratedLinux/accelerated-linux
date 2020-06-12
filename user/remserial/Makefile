all: remserial

remserial: remserial.c stty.c
	$(CC) $(CFLAGS) -o $@ stty.c remserial.c $(LDFLAGS)

romfs:
	$(MAKE) _romfs1 ACL_LICENSE=GPLv2 ACL_PKG=remserial

_romfs1:
	$(ROMFSINST) -p 555 /bin/remserial

clean:
	rm -f remserial

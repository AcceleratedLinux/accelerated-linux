
URL         = https://fossies.org/linux/misc/old/redir-2.2.1.tar.gz
CONFIGURE   = :
MAKEVARS    = CFLAGS="$(CFLAGS)" CC="$(CC)" LDFLAGS="$(LDFLAGS)"
FINALTARGET = built
ROMFS       = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) build/redir-2.2.1/redir /bin/redir

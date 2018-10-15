URL         = https://github.com/devshane/zork.git
METHOD      = git
FINALTARGET = built
zork_ROMFS  = my_romfs

include $(ROOTDIR)/tools/automake.inc

my_romfs:
	$(ROMFSINST) -d build/zork/zork /bin/zork
	$(ROMFSINST) -d build/zork/dtextc.dat /usr/games/lib/dtestc.dat

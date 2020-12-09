#
# Local Makefile for the configurator programs
#

CC = $(HOSTCC)

CPPFLAGS += $(shell $(CONFIG_SHELL) ./check.sh $(CC) $(CFLAGS) $(LDLIBS))

conf-objs := conf.o zconf.tab.o
mconf-objs := mconf.o zconf.tab.o
nconf-objs := nconf.o nconf.gui.o zconf.tab.o
gconf-objs := gconf.o zconf.tab.o
qconf-objs := qconf.o zconf.tab.o
lxdialog-objs := lxdialog/checklist.o lxdialog/inputbox.o lxdialog/menubox.o \
	lxdialog/textbox.o lxdialog/util.o lxdialog/yesno.o

all: conf mconf

conf: $(conf-objs)

mconf: $(mconf-objs) $(lxdialog-objs)
mconf: CPPFLAGS += `pkg-config --cflags ncurses 2>/dev/null || pkg-config --cflags ncursesw` -D LKC_DIRECT_LINK
mconf: LDLIBS += `pkg-config --libs ncurses 2>/dev/null || pkg-config --libs ncursesw` -ldl

nconf: $(nconf-objs)
nconf: CPPFLAGS += `pkg-config --cflags ncurses 2>/dev/null || pkg-config --cflags ncursesw` -D LKC_DIRECT_LINK
nconf: LDLIBS += `pkg-config --libs ncurses menu panel 2>/dev/null || pkg-config --libs ncursesw menuw panelw` -ldl

gconf: $(gconf-objs)
gconf: CPPFLAGS += `pkg-config --cflags gtk+-2.0 gmodule-2.0 libglade-2.0` -D LKC_DIRECT_LINK
gconf: LDLIBS += `pkg-config --libs gtk+-2.0 gmodule-2.0 libglade-2.0` -ldl

%.moc: %.h
	`pkg-config --variable=prefix QtCore 2>/dev/null`/bin/moc -i $< -o $@
qconf.o: qconf.moc
qconf: $(qconf-objs)
qconf: CPPFLAGS += `pkg-config --cflags QtCore QtGui` -D LKC_DIRECT_LINK
qconf: LDLIBS += `pkg-config --libs QtCore QtGui` -ldl -lstdc++

xconf: qconf

zconf.tab.o: zconf.tab.c zconf.lex.c confdata.c

%.c: %.c_shipped
	cp $< $@

clean:
	rm -f conf mconf gconf qconf *.moc *.o .*.o.cmd lxdialog/*.o

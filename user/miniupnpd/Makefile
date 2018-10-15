# $Id: Makefile,v 1.1 2008-09-15 12:28:52 winfred Exp $
# MiniUPnP project
# http://miniupnp.free.fr/
# Author : Thomas Bernard
# for use with GNU Make
# To install use :
# $ PREFIX=/dummyinstalldir make -f Makefile.linux install
# or :
# $ INSTALLPREFIX=/usr/local make -f Makefile.linux install
# or :
# $ make -f Makefile.linux install
#
#CFLAGS = -Wall -O -D_GNU_SOURCE -g -DDEBUG
#CFLAGS = -Wall -Os -D_GNU_SOURCE
IPTB = ../iptables
CFLAGS += -I$(IPTB)/include -I$(IPTB)/build/include
#CC = gcc
RM = rm -f
INSTALL = install

INSTALLPREFIX ?= $(PREFIX)/usr
SBININSTALLDIR = $(INSTALLPREFIX)/sbin
ETCINSTALLDIR = $(PREFIX)/etc/miniupnpd

BASEOBJS = miniupnpd.o upnphttp.o upnpdescgen.o upnpsoap.o \
           upnpreplyparse.o minixml.o \
	   upnpredirect.o getifaddr.o daemonize.o upnpglobalvars.o \
           options.o upnppermissions.o minissdp.o natpmp.o \
           upnpevents.o

LNXOBJS = linux/getifstats.o
NETFILTEROBJS = netfilter/iptcrdr.o

ALLOBJS = $(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS)

LDLIBS = -liptc
#LIBS = $(IPTB)/libiptc/libiptc.a

TESTUPNPDESCGENOBJS = testupnpdescgen.o upnpdescgen.o

EXECUTABLES = miniupnpd testupnpdescgen testgetifstats \
              testupnppermissions miniupnpdctl

.PHONY:	all clean install depend genuuid

all:	$(EXECUTABLES)

romfs:
	$(ROMFSINST) /bin/miniupnpd
	$(ROMFSINST) /etc_ro/miniupnpd.conf

clean:
	$(RM) $(ALLOBJS)
	$(RM) $(EXECUTABLES)
	$(RM) testupnpdescgen.o testgetifstats.o
	$(RM) testupnppermissions.o
	$(RM) miniupnpdctl.o

install:	miniupnpd genuuid
	$(INSTALL) -d $(SBININSTALLDIR)
	$(INSTALL) miniupnpd $(SBININSTALLDIR)
	$(INSTALL) -d $(ETCINSTALLDIR)
	$(INSTALL) netfilter/iptables_init.sh $(ETCINSTALLDIR)
	$(INSTALL) netfilter/iptables_removeall.sh $(ETCINSTALLDIR)
	$(INSTALL) --mode=0644 miniupnpd.conf $(ETCINSTALLDIR)
	$(INSTALL) -d $(PREFIX)/etc/init.d
	$(INSTALL) linux/miniupnpd.init.d.script $(PREFIX)/etc/init.d/miniupnpd

# genuuid is using the uuidgen CLI tool which is part of libuuid
# from the e2fsprogs
genuuid:
	sed -i -e "s/^uuid=[-0-9a-f]*/uuid=`(genuuid||uuidgen) 2>/dev/null`/" miniupnpd.conf

miniupnpd:	$(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS) $(LIBS)

testupnpdescgen:	$(TESTUPNPDESCGENOBJS)

testgetifstats:	testgetifstats.o linux/getifstats.o

testupnppermissions:	testupnppermissions.o upnppermissions.o

miniupnpdctl:	miniupnpdctl.o

config.h:	genconfig.sh
	./genconfig.sh

depend:	config.h
	makedepend -f$(MAKEFILE_LIST) -Y \
	$(ALLOBJS:.o=.c) $(TESTUPNPDESCGENOBJS:.o=.c) \
	testgetifstats.c 2>/dev/null

# DO NOT DELETE

miniupnpd.o: config.h upnpglobalvars.h upnppermissions.h miniupnpdtypes.h
miniupnpd.o: upnphttp.h upnpdescgen.h miniupnpdpath.h getifaddr.h upnpsoap.h
miniupnpd.o: options.h minissdp.h upnpredirect.h daemonize.h natpmp.h
miniupnpd.o: commonrdr.h
upnphttp.o: config.h upnphttp.h upnpdescgen.h miniupnpdpath.h upnpsoap.h
upnphttp.o: upnpevents.h
upnpdescgen.o: upnpdescgen.h config.h miniupnpdpath.h upnpglobalvars.h
upnpdescgen.o: upnppermissions.h miniupnpdtypes.h upnpdescstrings.h
upnpsoap.o: config.h upnpglobalvars.h upnppermissions.h miniupnpdtypes.h
upnpsoap.o: upnphttp.h upnpsoap.h upnpreplyparse.h upnpredirect.h getifaddr.h
upnpsoap.o: getifstats.h
upnpreplyparse.o: upnpreplyparse.h minixml.h
minixml.o: minixml.h
upnpredirect.o: config.h upnpredirect.h upnpglobalvars.h upnppermissions.h
upnpredirect.o: miniupnpdtypes.h netfilter/iptcrdr.h commonrdr.h
getifaddr.o: getifaddr.h
daemonize.o: daemonize.h config.h
upnpglobalvars.o: upnpglobalvars.h upnppermissions.h config.h
upnpglobalvars.o: miniupnpdtypes.h
options.o: options.h config.h upnppermissions.h upnpglobalvars.h
options.o: miniupnpdtypes.h
upnppermissions.o: config.h upnppermissions.h
minissdp.o: config.h upnpdescstrings.h miniupnpdpath.h upnphttp.h
minissdp.o: upnpglobalvars.h upnppermissions.h miniupnpdtypes.h minissdp.h
natpmp.o: config.h natpmp.h upnpglobalvars.h upnppermissions.h
natpmp.o: miniupnpdtypes.h getifaddr.h upnpredirect.h commonrdr.h
upnpevents.o: config.h upnpevents.h miniupnpdpath.h upnpglobalvars.h
upnpevents.o: upnppermissions.h miniupnpdtypes.h upnpdescgen.h
linux/getifstats.o: getifstats.h config.h
netfilter/iptcrdr.o: netfilter/iptcrdr.h commonrdr.h config.h
testupnpdescgen.o: config.h upnpdescgen.h
upnpdescgen.o: upnpdescgen.h config.h miniupnpdpath.h upnpglobalvars.h
upnpdescgen.o: upnppermissions.h miniupnpdtypes.h upnpdescstrings.h
testgetifstats.o: getifstats.h

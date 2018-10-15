# $Id: Makefile,v 1.1 2008-09-15 12:28:53 winfred Exp $
# made for GNU Make
CFLAGS = -Wall -g
EXECUTABLES = testgetifstats

all:	$(EXECUTABLES)

clean:
	rm -f *.o $(EXECUTABLES)

testobsdrdr.o:	testobsdrdr.c obsdrdr.h

testgetifstats:	testgetifstats.o getifstats.o -lkvm


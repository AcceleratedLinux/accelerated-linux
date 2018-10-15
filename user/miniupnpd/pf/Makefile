# $Id: Makefile,v 1.1 2008-09-15 12:28:53 winfred Exp $
# made for GNU Make
CFLAGS = -Wall -g
EXECUTABLES = testobsdrdr

all:	$(EXECUTABLES)

clean:
	rm -f *.o $(EXECUTABLES)

testobsdrdr:	testobsdrdr.o obsdrdr.o

obsdrdr.o:	obsdrdr.c obsdrdr.h

testobsdrdr.o:	testobsdrdr.c obsdrdr.h


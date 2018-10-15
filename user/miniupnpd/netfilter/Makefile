# $Id: Makefile,v 1.1 2008-09-15 12:28:53 winfred Exp $
#CFLAGS=-Wall -g

all:	iptcrdr.o testiptcrdr

clean:
	$(RM) *.o testiptcrdr

testiptcrdr:	testiptcrdr.o iptcrdr.o -liptc

iptcrdr.o:	iptcrdr.c iptcrdr.h


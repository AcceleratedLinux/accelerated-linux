#
# Makefile
#
# This is the Makefile for libpvf.
#
# $Id: Makefile,v 1.6 2006/02/22 17:25:38 gert Exp $
#

PVFOBJS=au.o fft.o lib.o linear.o multitech.o rockwell.o usr.o voc.o wav.o \
	zyxel.o zyxel-o56k.o

.c.o:
	$(CC) $(CFLAGS) -c $*.c

all: libpvf.a

libpvf.a: $(PVFOBJS)
	$(RM) -f libpvf.a
	$(AR) $(ARFLAGS) libpvf.a $(PVFOBJS)
	-$(RANLIB) libpvf.a

install:

clean:
	@$(RM) -f libpvf.a $(PVFOBJS)

include depend

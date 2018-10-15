#
# Makefile
#
# This is the Makefile for libutil.
#
# $Id: Makefile,v 1.6 2006/02/22 17:25:39 gert Exp $
#

UTILOBJS=check_system.o config.o no_modem.o make_path.o strsep.o variables.o \
         access.o wildmat.o ../../config.o ../../logfile.o

.c.o:
	$(CC) $(CFLAGS) -c $*.c
 
all: libutil.a test_wildmat

config.o: config.c
	$(CC) $(CFLAGS) -DCONF_DIR=\"$(CONFDIR)\" -c $*.c

libutil.a: $(UTILOBJS)
	$(RM) -f libutil.a
	$(AR) $(ARFLAGS) libutil.a $(UTILOBJS)
	-$(RANLIB) libutil.a

test_wildmat: test_wildmat.o wildmat.o
	$(CC) $(LDFLAGS) -o test_wildmat test_wildmat.o wildmat.o

install:

clean:
	@$(RM) -f libutil.a test_wildmat test_wildmat.o $(UTILOBJS)

include depend

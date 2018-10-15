# Makefile for the "t/" subtree
# of mgetty+sendfax
#
# CVS: $Id: Makefile,v 1.6 2006/09/25 22:23:56 gert Exp $ Copyright (c) Gert Doering
#

CC=gcc
CFLAGS=-O2 -I.. -g  #-DT_LOG_VERBOSE

TEST_PROGRAMS=t_config t_ring
TEST_HELP_PGM=t_g3f_c
TEST_SCRIPTS=t_echo t_trap t_g3file

test: $(TEST_PROGRAMS) $(TEST_HELP_PGM)
	@for f in $(TEST_PROGRAMS) ; \
	do \
	    echo "run t/$$f..." ; \
	    ./$$f || exit 1 ; \
	done
	@for f in $(TEST_SCRIPTS) ; \
	do \
	    echo "run t/$$f..." ; \
	    rm -f $$f ; ../sedscript <$$f.in >$$f ; chmod +x $$f ; \
	    ./$$f || exit 1 ; \
	done
	@echo "OK"

clean:
	rm -f $(TEST_PROGRAMS)
	rm -f $(TEST_SCRIPTS)
	rm -f *.o

t_config.o: t_config.c ../mgetty.h ../conf_mg.h ../config.h 

t_ring: t_ring.o ../ring.o ../cnd.o
	$(CC) -o t_ring t_ring.o ../ring.o ../cnd.o $(LDFLAGS)

T_G3F_OBJS=t_g3f_c.o ../g3file.o ../faxlib.o ../hyla_nsf.o ../modem.o ../io.o
t_g3f_c: $(T_G3F_OBJS)
	$(CC) -o t_g3f_c $(T_G3F_OBJS) $(LDFLAGS)

CC = gcc
CFLAGS = -W -Wall -O2 -g
PROGS = mkcramfs cramfsck

all: $(PROGS)

mkcramfs:
	$(CC) -o $@ ../mkcramfs.c -lz

cramfsck:
	$(CC) -o $@ ../cramfsck.c -lz

distclean clean:
	rm -f $(PROGS)

.PHONY: all clean distclean


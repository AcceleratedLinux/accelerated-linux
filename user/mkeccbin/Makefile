
CC = gcc
CXX = g++
AR = ar
CFLAGS = -W -Wall -O2 -g

INCLUDEDIR = .
ifndef ROOTDIR
ROOTDIR = ../../..
export ROOTDIR
endif

CFLAGS := -I$(INCLUDEDIR)

all: mkeccbin

mkeccbin: mkeccbin.o bch.o hexdump.o
	$(CC) -o mkeccbin mkeccbin.o bch.o hexdump.o

clean:
	rm -f mkeccbin *.o 


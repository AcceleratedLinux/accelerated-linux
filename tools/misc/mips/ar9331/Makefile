
XCC = mips-linux-20071101-gcc
XLD = mips-linux-20071101-ld
CFLAGS = -mlong-calls -mno-abicalls -fno-pic -O

OBJS = start.o flasher.o


all: flasher

flasher: $(OBJS)
	$(XLD) -T flasher.ld -static -nostdlib -o flasher $(OBJS)

%.o: %.S
	$(XCC) $(CFLAGS) -c $<

%.o: %.c
	$(XCC) $(CFLAGS) -c $<

clean:
	rm -f flasher *.o boot.elf


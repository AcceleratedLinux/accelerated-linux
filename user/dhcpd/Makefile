
EXEC = dhcpd
OBJS = dhcpd.o arpping.o socket.o options.o files.o debug.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

$(OBJS): dhcpd.h socket.h options.h files.h debug.h


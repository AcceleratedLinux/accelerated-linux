ACL_LICENSE = Digi
ACL_URL     = http://www.digi.com/
ACL_PKG     = Digi Accelerated Linux
export ACL_PKG ACL_URL ACL_LICENSE

EXEC = setfset
OBJS = setfset.o fsetv.o

CFLAGS += -I.
LDLIBS += -lopengear

ACL_LICENSE = Unknown

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o


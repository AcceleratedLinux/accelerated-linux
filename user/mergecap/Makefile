export ACL_LICENSE = GPLv2

EXEC = mergecap
SRCS = $(EXEC).c
OBJS = $(EXEC).o

all: $(EXEC)

$(OBJS): $(SRCS)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))

clean:
	rm -f $(EXEC) *.elf *.gdb *.o

romfs:
	$(ROMFSINST) /sbin/$(EXEC)
	$(ROMFSINST) /sbin/generate_packets

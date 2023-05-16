export ACL_LICENSE = GPLv2

EXEC = ledcmd
OBJS = ledcmd.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))

romfs:
	$(ROMFSINST) -e CONFIG_PROP_CONFIG_LEDCMD_WRAPPER $(EXEC) /bin/$(EXEC).new
	$(ROMFSINST) -E CONFIG_PROP_CONFIG_LEDCMD_WRAPPER /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o


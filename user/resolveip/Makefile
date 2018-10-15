
EXEC1 = resolveip
OBJS1 = resolveip.o

all: $(EXEC1)

$(EXEC1): $(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $(OBJS1) $(LDLIBS$(LDLIBS_$@))

romfs:
	$(ROMFSINST) /bin/$(EXEC1)

clean:
	rm -f $(EXEC1) *.elf *.gdb *.o


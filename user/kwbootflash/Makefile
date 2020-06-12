-include ../user.mak

EXEC = kwbootflash
OBJS = kwbootflash.o ecc_kw.o

CFLAGS += -D_GNU_SOURCE

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o


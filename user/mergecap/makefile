SRC=mergecap.c
PROGRAM=mergecap

all: $(SRC)
	$(CC) $(SRC) -o $(PROGRAM)

clean:
	-rm $(PROGRAM); fi

romfs:
	$(ROMFSINST) /sbin/$(PROGRAM)
	$(ROMFSINST) /sbin/generate_packets

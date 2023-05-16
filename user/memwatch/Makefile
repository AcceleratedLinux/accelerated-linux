#
# Makefile -- build memwatch
#

EXEC = memwatch
SRC = memwatch.c
OBJS = $(patsubst %.c,%.o,$(SRC))

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -static $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean:
	rm -f $(EXEC) $(OBJS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

#
# Makefile -- build the ble-provisioning-app app
#
VERSION = 5.46

CFLAGS += -Wall -O2
CFLAGS += -I../bluez/build/bluez-$(VERSION)/ -I../bluez/build/bluez-$(VERSION)/lib

LDFLAGS += -L../bluez/build/bluez-$(VERSION)/lib -L../bluez/build/bluez-$(VERSION)/src
LDLIBS += -lshared-mainloop -lbluetooth-internal

EXEC = ble-provisioning-app
SRC = $(EXEC).c subprocess.c
OBJS = $(patsubst %.c,%.o,$(SRC))

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean:
	rm -f $(EXEC) $(OBJS)

romfs:
	$(ROMFSINST) -p 555 /bin/$(EXEC)

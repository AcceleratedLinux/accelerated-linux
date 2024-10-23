export ACL_LICENSE = GPLv2

-include ../user.mak

EXEC = setmac
OBJS = setmac.o

ifneq ("",$(CONFIG_USER_SETMAC_PREFIX))
CFLAGS += -DSETMAC_PREFIX=$(strip $(CONFIG_USER_SETMAC_PREFIX))
endif

ifdef CONFIG_IXP400_ETH
ifndef CONFIG_IXP425_IXP_AS_ETH
CFLAGS += -DETHPREFIX=\"ixp\"
endif
endif

ifdef CONFIG_USER_IPROUTE2_IP
CFLAGS += -DUSE_IP=1
endif

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o


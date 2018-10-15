-include ../user.mak

EXEC1 = init
OBJS1 = simpleinit.o

EXEC2 = expand
OBJS2 = expand.o

ifdef CONFIG_UCLINUX
BINDIR = /bin
else
BINDIR = /sbin
endif
ifdef CONSOLE_BAUD_RATE
CFLAGS += -DCONSOLE_BAUD_RATE=$(CONSOLE_BAUD_RATE)
endif

ifdef CONFIG_LIB_UC_LIBC_TIMEZONE
ifndef CONFIG_LIB_UC_LIBC_TZ_FROM_FILE
CFLAGS += -DINCLUDE_TIMEZONE
endif
endif

ifdef CONFIG_LIB_LIBSSL_FIPS
CFLAGS += -DINCLUDE_FIPS
endif

all: $(EXEC1) $(EXEC2)

$(EXEC1): $(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $(OBJS1) $(LIBCRYPT) $(LDLIBS$(LDLIBS_$@))
	cp init simpleinit

$(EXEC2): $(OBJS2)
	$(CC) $(LDFLAGS) -o $@ $(OBJS2) $(LDLIBS$(LDLIBS_$@))

romfs:
	$(ROMFSINST) -e CONFIG_USER_INIT_EXPAND /bin/$(EXEC2)
	$(ROMFSINST) -e CONFIG_USER_INIT_INIT /bin/$(EXEC1)
ifdef CONFIG_USER_INIT_OVERLAY
	mkdir -p $(ROMFSDIR)/overlay
endif
	$(ROMFSINST) -e CONFIG_USER_INIT_OVERLAY -p 755 /bin/init-overlay
ifdef CONFIG_USER_INIT_DEFAULT_ENTRIES
	$(ROMFSINST) -e CONFIG_USER_INIT_INIT -e CONFIG_USER_BUSYBOX_SYSLOGD \
			-a "slog:unknown:${BINDIR}/syslogd -n" /etc/inittab
	$(ROMFSINST) -e CONFIG_USER_INIT_INIT -e CONFIG_USER_BUSYBOX_KLOGD \
			-a "klog:unknown:${BINDIR}/klogd -n" /etc/inittab
	$(ROMFSINST) -e CONFIG_USER_INIT_INIT -e CONFIG_USER_SYSKLOGD \
			-a "slog:unknown:/usr/sbin/syslogd -n" /etc/inittab
	$(ROMFSINST) -e CONFIG_USER_INIT_INIT -e CONFIG_USER_SYSKLOGD \
			-a "klog:unknown:/usr/sbin/klogd -n -x" /etc/inittab
endif

clean:
	rm -f simpleinit $(EXEC1) $(EXEC2) *.elf *.gdb *.o


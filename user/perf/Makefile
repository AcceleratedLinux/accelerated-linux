
PERFSRCDIR := $(ROOTDIR)/$(LINUXDIR)/tools/perf

all: perf

perf: perfbuild
	ln -sf $(PERFSRCDIR)/perf perf

perfbuild:
	make WERROR=0 -C $(PERFSRCDIR)

clean:
	rm -f perf
	make -C $(PERFSRCDIR) clean

romfs:
	$(ROMFSINST) /bin/perf


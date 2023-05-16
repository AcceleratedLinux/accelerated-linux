
CFLAGS = -Wall -g
LDFLAGS = -g

ifeq ($(CONFIG_GLIBC_2_5),y)
 CPPFLAGS += -DNO_ENDIAN_H
endif

VENDOR   =  $(or $(HW_COMPAT_VENDOR),$(HW_VENDOR),$(CONFIG_VENDOR))
PRODUCT  =  $(or $(HW_COMPAT_PRODUCT),$(HW_PRODUCT),$(CONFIG_PRODUCT))
VERSION  =  $(VERSIONPKG)

version.o: CPPFLAGS += -DVENDOR=\"$(VENDOR)\"
version.o: CPPFLAGS += -DPRODUCT=\"$(PRODUCT)\"
version.o: CPPFLAGS += -DVERSION=\"$(VERSION)\"

OBJS += dospart.o
OBJS += fd.o
OBJS += membuf.o
OBJS += multipart.o
OBJS += netflash.o
OBJS += proc.o
OBJS += progress.o
OBJS += qcow2.o
OBJS += range.o
OBJS += version.o

netflash: $(OBJS)
	$(LINK.c) -o $@ $(OBJS)

TESTS += test/t-membuf
test/t-membuf: membuf.o test/t-membuf.o test/mock-vfile.o

TESTS += test/t-multipart
test/t-multipart: multipart.o test/t-multipart.o test/mock-vfile.o range.o

test/%,check: test/%; $(TEST_EXEC) $^
check: $(TESTS:=,check)
.PHONY: check test/%,check

clean:
	rm -f $(TESTS) *.o test/*.o netflash

romfs:
	$(ROMFSINST) /bin/netflash

.PHONY: clean romfs default


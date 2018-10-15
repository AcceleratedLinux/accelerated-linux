#
# Makefile for the KSZ8692 Ethernet driver
#


TARGETNAME := ksz8692_net


HEADERS = device.h hardware.h target.h ks_phy.h Makefile

OBJS := device.o interrupt.o transmit.o hardware.o target.o \
	ks8001.o vsc8201.o genericphy.o

ifdef CONFIG_SIM_IPSEC
OBJS += test_ipsec.o
endif


EXTRA_CFLAGS += -DDEF_LINUX

ifndef DEBUG
EXTRA_CFLAGS += -DINLINE
endif

ifdef DEBUG
ifdef INLINE
EXTRA_CFLAGS += -DINLINE
endif
EXTRA_CFLAGS += -DDBG
endif

ifdef CONFIG_MII
EXTRA_CFLAGS += -DMII
endif

EXTRA_CFLAGS += -DNO_STATS


# Build within Linux kernel

ifdef PATCHLEVEL

ifeq ($(VERSION).$(PATCHLEVEL),2.4)
O_TARGET := $(TARGETNAME).o

obj-y	:= $(OBJS)

obj-m   := $(O_TARGET)

include $(TOPDIR)/Rules.make

else
ifndef CONFIG_ARM_KS8692_ETHER
CONFIG_ARM_KS8692_ETHER=m
endif

obj-$(CONFIG_ARM_KS8692_ETHER) += $(TARGETNAME).o

$(TARGETNAME)-objs := $(OBJS)
endif

else

# Find out if GCC tools are needed.
NO_CC := $(shell if [ "$(MAKECMDGOALS)" = "clean" ]  || \
    [ "$(MAKECMDGOALS)" = "zip" ]  || \
    [ "$(MAKECMDGOALS)" = "distclean" ]; then \
    echo "yes"; else echo ""; fi)
ifneq ("$(NO_CC)", "yes")

CROSS ?=
KERNELDIR ?= /usr/src/linux-2.4
LD_FLAGS ?=
CFLAGS :=


GCC = $(CROSS)gcc
STRIP = $(CROSS)strip
LD = $(CROSS)ld


# Check which kernel is used

MAKE_FILE = $(KERNELDIR)/Makefile
PATCHLEVEL := $(shell grep "^PATCHLEVEL =" $(MAKE_FILE) | cut -d" " -f3)
SUBLEVEL := $(shell grep "^SUBLEVEL =" $(MAKE_FILE) | cut -d" " -f3)

ifeq ($(PATCHLEVEL),4)
O_TARGET := $(TARGETNAME).o

default: $(O_TARGET)

$(O_TARGET) : $(OBJS) $(HEADERS)
	@$(LD) -r $(OBJS) -o $@

else
O_TARGET := $(TARGETNAME).ko
TARGET := $(TARGETNAME).o

default: $(O_TARGET)

$(TARGET) : $(OBJS) $(HEADERS)
	@$(LD) $(LD_FLAGS) -r $(OBJS) -o $@

$(TARGETNAME).mod.c: $(TARGETNAME).o
	@if [ -d $(KERNELDIR)/scripts/mod ]; then \
	$(KERNELDIR)/scripts/mod/modpost -i \
		$(KERNELDIR)/Module.symvers $<; \
	else \
	$(KERNELDIR)/scripts/modpost -i \
		$(KERNELDIR)/Module.symvers $(KERNELDIR)/vmlinux $<; \
	fi
	touch $@

$(TARGETNAME).mod.o: .c=.o

$(O_TARGET): $(TARGETNAME).o $(TARGETNAME).mod.o
	$(LD) $(LD_FLAGS) -EL -r -o $@ $^
endif
endif


CFLAGS := $(CFLAGS) -D__KERNEL__ -DMODULE -I$(KERNELDIR)/include -O2
#CFLAGS += -Wall
CFLAGS += -Wstrict-prototypes -fomit-frame-pointer -fno-common
#CFLAGS += -march=k8 -mtune=nocona -m64 -mcmodel=kernel

ifeq ($(PATCHLEVEL),6)
ifeq ($(SUBLEVEL),15)
CFLAGS += -D"KBUILD_STR(s)=\#s"
basename_flags = -D"KBUILD_BASENAME=KBUILD_STR($(*F))"
CFLAGS += -D"KBUILD_MODNAME=KBUILD_STR($(TARGETNAME))"
else
basename_flags = -DKBUILD_BASENAME=$(*F)
CFLAGS += -DKBUILD_MODNAME=$(TARGETNAME)
endif
else
basename_flags = -DKBUILD_BASENAME=$(*F)
endif

CFLAGS += $(EXTRA_CFLAGS)

SYSLIB = /usr/lib

%.o: %.c
	$(GCC) $(CFLAGS) $(basename_flags) $(CPPFLAGS) -c $<


hardware.o:     $(HEADERS)
target.o:       $(HEADERS)
device.o:       $(HEADERS)
interrupt.o:    $(HEADERS)
transmit.o:     $(HEADERS)

ks8001.o:	$(HEADERS)
vcs8201.o:	$(HEADERS)
genericphy.o:	$(HEADERS)


clean:
	@$(RM) *.o *.a *~ *.out .*.flags
	@rm -f *.ko *.mod.c .*.cmd Modules.symvers
	@rm -fr .tmp_versions
endif

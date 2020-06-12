##############################################################################
# Copyright 2019 Digi International Inc., All Rights Reserved
#
# This software contains proprietary and confidential information of Digi
# International Inc.  By accepting transfer of this copy, Recipient agrees
# to retain this software in confidence, to prevent disclosure to others,
# and to make no use of this software other than that for which it was
# delivered.  This is an unpublished copyrighted work of Digi International
# Inc.  Except as permitted by federal law, 17 USC 117, copying is strictly
# prohibited.
#
# Restricted Rights Legend
#
# Use, duplication, or disclosure by the Government is subject to
# restrictions set forth in sub-paragraph (c)(1)(ii) of The Rights in
# Technical Data and Computer Software clause at DFARS 252.227-7031 or
# subparagraphs (c)(1) and (2) of the Commercial Computer Software -
# Restricted Rights at 48 CFR 52.227-19, as applicable.
#
# Digi International Inc., 9350 Excelsior Blvd., Suite 700, Hopkins, MN 55343
##############################################################################


DESTDIR = $(INSTALL_ROOT)
CFLAGS+=-DDESTDIR=$(DESTDIR)


LIBOBJS = pcapng_read.o \
		  pcapng_write.o \
		  block_options.o \
		  enhanced_packet_block.o \
		  file.o \
		  interface_block.o \
		  interfaces.o \
		  section_block.o \
		  utils.o

LIBRARY = libpcapng.so

PCAPNG_HDR = pcapng.h

# INCLUDE is required when building for the host.
INCLUDE = -Ilib/libpcap

CFLAGS += -fPIC -MD

ifeq ($(DEBUG),true)
CFLAGS += -g -O0
else
CFLAGS += -O2
endif

LFLAGS += $(LIBPATH)

.PHONY: all clean install

all: $(LIBRARY)

# Note that the version-script option is used to get the shared object library to only expose those
# API functions that we want the user to see. The functions in question are all listed in pcapng.h.
# Update file "version_script" when the public API functions are changed.
$(LIBRARY): $(LIBOBJS)
	$(CC) $(LDFLAGS) -Wl,-soname,$(LIBRARY) -Wl,--version-script=version_script -shared $^ -o $@ $(LFLAGS)

# Dependancy rule
-include $(LIBOBJS:.o=.d)

$(LIBOBJS): %.o: %.c
	$(COMPILE.c) $(DEFINES) -MD -fPIC -o $@ $<

romfs: $(LIBRARY)
	$(ROMFSINST) -p 0755 /lib/$(LIBRARY)
	$(ROMFSINST) -p 0644 /lib/$(PCAPNG_HDR)

clean:
	-rm -f $(LIBRARY) $(LIBOBJS) *.d

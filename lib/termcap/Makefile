## -*- text -*- ####################################################
#								   #
# Makefile for termcap replacement libbrary.			   #
#								   #
####################################################################

# Copyright (C) 1996 Free Software Foundation, Inc.     

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA.

# ensure static lib can be used by shared libs
ifdef CONFIG_LIB_TERMCAP_SHARED
CFLAGS += -fPIC
TARGET = libtermcap.so
else
TARGET = libtermcap.a
endif
OBJS = termcap.o tparam.o version.o

all: $(TARGET)

libtermcap.a: $(OBJS)
	$(AR) cr $@ $(OBJS)

libtermcap.so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS)

romfs:
	$(ROMFSINST) -e CONFIG_LIB_TERMCAP_SHARED /lib/libtermcap.so

clean:
	$(RM) *.o libtermcap.a core core.*

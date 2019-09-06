#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

CFLAGS += -DUSE_HOSTCC
CFLAGS += -Iinclude

OBJS = crc32.o
OBJS += ctype.o
OBJS += env_attr.o
OBJS += env_flags.o
OBJS += fw_env.o
OBJS += fw_env_main.o
OBJS += linux_string.o

all: fw_printenv fw_setenv

fw_printenv: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))

fw_setenv: fw_printenv
	ln -f fw_printenv fw_setenv

romfs:
	$(ROMFSINST) /bin/fw_printenv
	$(ROMFSINST) -s /bin/fw_printenv /bin/fw_setenv
	$(ROMFSINST) -e CONFIG_USER_UBOOT_ENVTOOLS_DEFAULT_CONFIG_FILE /etc/fw_env.config

clean:
	rm -f fw_printenv fw_setenv *.o


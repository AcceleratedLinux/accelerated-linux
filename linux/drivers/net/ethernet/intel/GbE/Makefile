# GPL LICENSE SUMMARY
# 
#   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
# 
#   This program is free software; you can redistribute it and/or modify 
#   it under the terms of version 2 of the GNU General Public License as
#   published by the Free Software Foundation.
# 
#   This program is distributed in the hope that it will be useful, but 
#   WITHOUT ANY WARRANTY; without even the implied warranty of 
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
#   General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License 
#   along with this program; if not, write to the Free Software 
#   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
#   The full GNU General Public License is included in this distribution 
#   in the file called LICENSE.GPL.
# 
#   Contact Information:
#   Intel Corporation
# 
#  version: Embedded.L.1.0.3-144

###########################################################################

EXTRA_CFLAGS += -DCONFIG_UTM2000=1 -DEXTERNAL_MDIO=1

obj-$(CONFIG_IEGBE) += iegbe.o gcu.o

iegbe-objs :=	iegbe_main.o iegbe_hw.o iegbe_param.o \
		iegbe_oem_phy.o iegbe_ethtool.o kcompat.o
gcu-objs :=	gcu_main.o gcu_if.o


#!/bin/sh

# $Id: config-vlan.sh,v 1.27 2009-01-14 11:49:07 winfred Exp $
#
# usage: config-vlan.sh <switch_type> <vlan_type>
#   switch_type: 0=IC+, 1=vtss
#   vlan_type: 0=no_vlan, 1=vlan, LLLLW=wan_4, WLLLL=wan_0
# 

. /sbin/config.sh

usage()
{
	echo "Usage:"
	echo "  $0 0 0 - restore IC+ to no VLAN partition"
	echo "  $0 0 LLLLW - config IC+ with VLAN and WAN at port 4"
	echo "  $0 0 WLLLL - config IC+ with VLAN and WAN at port 0"
	echo "  $0 1 0 - restore Vtss to no VLAN partition"
	echo "  $0 1 1 - config Vtss with VLAN partition"
	echo "  $0 2 0 - restore RT3052 to no VLAN partition"
	echo "  $0 2 LLLLW - config RT3052 with VLAN and WAN at port 4"
	echo "  $0 2 WLLLL - config RT3052 with VLAN and WAN at port 0"
	echo "  $0 2 W1234 - config RT3052 with VLAN 5 at port 0 and VLAN 1~4 at port 1~4"
	echo "  $0 2 12345 - config RT3052 with VLAN 1~5 at port 0~4"
	echo "  $0 2 GW - config RT3052 with WAN at Giga port"
	exit 0
}

config175C()
{
	mii_mgr -s -p 29 -r 23 -v 0x07c2
	mii_mgr -s -p 29 -r 22 -v 0x8420

	if [ "$1" = "LLLLW" ]; then
		mii_mgr -s -p 29 -r 24 -v 0x1
		mii_mgr -s -p 29 -r 25 -v 0x1
		mii_mgr -s -p 29 -r 26 -v 0x1
		mii_mgr -s -p 29 -r 27 -v 0x1
		mii_mgr -s -p 29 -r 28 -v 0x2
		mii_mgr -s -p 30 -r 9 -v 0x1089
		if [ "$CONFIG_RALINK_VISTA_BASIC" == "y" ]; then
			mii_mgr -s -p 30 -r 1 -v 0x2f3f
		else
			mii_mgr -s -p 30 -r 1 -v 0x2f00
		fi
		mii_mgr -s -p 30 -r 2 -v 0x0030
	elif [ "$1" = "WLLLL" ]; then
		mii_mgr -s -p 29 -r 24 -v 0x2
		mii_mgr -s -p 29 -r 25 -v 0x1
		mii_mgr -s -p 29 -r 26 -v 0x1
		mii_mgr -s -p 29 -r 27 -v 0x1
		mii_mgr -s -p 29 -r 28 -v 0x1
		mii_mgr -s -p 30 -r 9 -v 0x0189
		if [ "$CONFIG_RALINK_VISTA_BASIC" == "y" ]; then
			mii_mgr -s -p 30 -r 1 -v 0x3e3f
		else
			mii_mgr -s -p 30 -r 1 -v 0x3e00
		fi
		mii_mgr -s -p 30 -r 2 -v 0x0021
	else
		echo "LAN WAN layout $0 is not suported"
	fi
}

restore175C()
{
	mii_mgr -s -p 29 -r 23 -v 0x0
	mii_mgr -s -p 29 -r 22 -v 0x420
	mii_mgr -s -p 29 -r 24 -v 0x1
	mii_mgr -s -p 29 -r 25 -v 0x1
	mii_mgr -s -p 29 -r 26 -v 0x1
	mii_mgr -s -p 29 -r 27 -v 0x1
	mii_mgr -s -p 29 -r 27 -v 0x2
	mii_mgr -s -p 30 -r 9 -v 0x1001
	mii_mgr -s -p 30 -r 1 -v 0x2f3f
	mii_mgr -s -p 30 -r 2 -v 0x3f30
}

restore175D()
{
	mii_mgr -s -p 20 -r  4 -v 0xa000
	mii_mgr -s -p 20 -r 13 -v 0x20
	mii_mgr -s -p 21 -r  1 -v 0x1800
	mii_mgr -s -p 22 -r  0 -v 0x0
	mii_mgr -s -p 22 -r  2 -v 0x0
	mii_mgr -s -p 22 -r 10 -v 0x0
	mii_mgr -s -p 22 -r 14 -v 0x1
	mii_mgr -s -p 22 -r 15 -v 0x2
	mii_mgr -s -p 23 -r  8 -v 0x0
	mii_mgr -s -p 23 -r 16 -v 0x0

	mii_mgr -s -p 22 -r 4 -v 0x1
	mii_mgr -s -p 22 -r 5 -v 0x1
	mii_mgr -s -p 22 -r 6 -v 0x1
	mii_mgr -s -p 22 -r 7 -v 0x1
	mii_mgr -s -p 22 -r 8 -v 0x1
	mii_mgr -s -p 23 -r 0 -v 0x3f3f
}

config175D()
{
	mii_mgr -s -p 20 -r  4 -v 0xa000
	mii_mgr -s -p 20 -r 13 -v 0x21
	mii_mgr -s -p 21 -r  1 -v 0x1800
	mii_mgr -s -p 22 -r  0 -v 0x27ff
	mii_mgr -s -p 22 -r  2 -v 0x20
	mii_mgr -s -p 22 -r  3 -v 0x8100
	mii_mgr -s -p 22 -r 10 -v 0x3
	mii_mgr -s -p 22 -r 14 -v 0x1001
	mii_mgr -s -p 22 -r 15 -v 0x2002
	mii_mgr -s -p 23 -r  8 -v 0x2020
	mii_mgr -s -p 23 -r 16 -v 0x1f1f
	if [ "$1" = "LLLLW" ]; then
		mii_mgr -s -p 22 -r 4 -v 0x1
		mii_mgr -s -p 22 -r 5 -v 0x1
		mii_mgr -s -p 22 -r 6 -v 0x1
		mii_mgr -s -p 22 -r 7 -v 0x1
		mii_mgr -s -p 22 -r 8 -v 0x2
		mii_mgr -s -p 23 -r 0 -v 0x302f
	elif [ "$1" = "WLLLL" ]; then
		mii_mgr -s -p 22 -r 4 -v 0x2
		mii_mgr -s -p 22 -r 5 -v 0x1
		mii_mgr -s -p 22 -r 6 -v 0x1
		mii_mgr -s -p 22 -r 7 -v 0x1
		mii_mgr -s -p 22 -r 8 -v 0x1
		mii_mgr -s -p 23 -r 0 -v 0x213e
	else
		echo "LAN WAN layout $0 is not suported"
	fi
}

configVtss()
{
	spicmd vtss vlan
}

restoreVtss()
{
	spicmd vtss novlan
}

config3052()
{
	switch reg w 14 405555
	switch reg w 50 2001
	switch reg w 98 7f3f
	switch reg w e4 3f
	if [ "$1" = "LLLLW" ]; then
		switch reg w 40 1001
		switch reg w 44 1001
		switch reg w 48 1002
		switch reg w 70 ffff506f
	elif [ "$1" = "WLLLL" ]; then
		switch reg w 40 1002
		switch reg w 44 1001
		switch reg w 48 1001
		switch reg w 70 ffff417e
	elif [ "$1" = "W1234" ]; then
		switch reg w 40 1005
		switch reg w 44 3002
		switch reg w 48 1004
		switch reg w 70 50484442
		switch reg w 74 ffffff41
	elif [ "$1" = "12345" ]; then
		switch reg w 40 2001
		switch reg w 44 4003
		switch reg w 48 1005
		switch reg w 70 7e7e7e41
		switch reg w 74 ffffff7e
	elif [ "$1" = "GW" ]; then
		switch reg w 40 1001
		switch reg w 44 1001
		switch reg w 48 2001
		switch reg w 70 ffff605f
	fi
}

restore3052()
{
	switch reg w 14 5555
	switch reg w 40 1001
	switch reg w 44 1001
	switch reg w 48 1001
	switch reg w 4c 1
	switch reg w 50 2001
	switch reg w 70 ffffffff
	switch reg w 98 7f7f
	switch reg w e4 7f
}

if [ "$1" = "0" ]; then
	#isc is used to distinguish between 175C and 175D
	isc=`mii_mgr -g -p 29 -r 31`
	if [ "$2" = "0" ]; then
		if [ "$isc" = "Get: phy[29].reg[31] = 175c" ]; then
			restore175C
		else
			restore175D
		fi
	elif [ "$2" = "LLLLW" ]; then
		if [ "$isc" = "Get: phy[29].reg[31] = 175c" ]; then
			config175C "LLLLW"
		else
			config175D "LLLLW"
		fi
	elif [ "$2" = "WLLLL" ]; then
		if [ "$isc" = "Get: phy[29].reg[31] = 175c" ]; then
			config175C "WLLLL"
		else
			config175D "WLLLL"
		fi
	else
		echo "unknown vlan type $2"
		echo ""
		usage $0
	fi
elif [ "$1" = "1" ]; then
	if [ "$2" = "0" ]; then
		restoreVtss
	elif [ "$2" = "1" ]; then
		configVtss
	else
		echo "unknown vlan type $2"
		echo ""
		usage $0
	fi
elif [ "$1" = "2" ]; then
	if [ "$2" = "0" ]; then
		restore3052
	elif [ "$2" = "LLLLW" ]; then
		config3052 LLLLW
	elif [ "$2" = "WLLLL" ]; then
		config3052 WLLLL
	elif [ "$2" = "W1234" ]; then
		config3052 W1234
	elif [ "$2" = "12345" ]; then
		config3052 12345
	elif [ "$2" = "GW" ]; then
		config3052 GW
	else
		echo "unknown vlan type $2"
		echo ""
		usage $0
	fi
else
	echo "unknown swith type $1"
	echo ""
	usage $0
fi


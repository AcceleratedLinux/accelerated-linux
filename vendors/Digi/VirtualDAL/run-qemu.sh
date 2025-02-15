#!/bin/sh

OPTIONS=

DISK=VirtualDAL.img

usage() {
	echo "usage: $(basename $0) [-d DISK] [-p PID_FILE]

	-d	DISK		- Defines which disk image to use with this drive
	-p	PID_FILE	- Store the QEMU process PID in file
"
	exit 1
}

while true
do
	case "$1" in
		-d)
			shift
			DISK=$1
			shift
			;;
		-p)
			shift
			PID_FILE=$1
			shift
			;;
		-h|-\?)
			usage
			;;

		*)
			break
			;;

	esac
done

# first NIC,  gets default address 10.0.2.15
# ssh available as "ssh -p 2222 admin@localhost"
# https available as "https://localhost:4443/"
OPTIONS="$OPTIONS -netdev user,id=e1,hostfwd=tcp::2222-:22,hostfwd=tcp::4443-:443 -device e1000,netdev=e1"

# second NIC,  gets default address 10.0.3.15
OPTIONS="$OPTIONS -netdev user,id=e2,net=10.0.3.0/24 -device e1000,netdev=e2"

# memory size in Megs
OPTIONS="$OPTIONS -m 1024"

# hook up serial port to stin/stdout
OPTIONS="$OPTIONS -serial mon:stdio"

# The disk image
OPTIONS="$OPTIONS -drive id=diskA,file=$DISK,if=none,cache=unsafe"
OPTIONS="$OPTIONS -device ahci,id=ahci"
OPTIONS="$OPTIONS -device ide-hd,drive=diskA,bus=ahci.0"

# PID file
[ -n "$PID_FILE" ] && OPTIONS="$OPTIONS -pidfile $PID_FILE"

# Use kvm if we can
kvm-ok >/dev/null && OPTIONS="$OPTIONS -enable-kvm"

qemu-system-x86_64 $OPTIONS

#!/bin/sh

usage_and_exit()
{
	echo "Usage:"
	echo "First start VirtualDAL:"
	echo
	echo "    $(basename $0) virtualdal [<virtualdal-image>]"
	echo
	echo "And then start the Raspberry Pi:"
	echo
	echo "    $(basename $0) raspi <raspi-dir>"
	echo

	exit 1
}

start_virtualdal()
{
	local disk_image="VirtualDAL.img"
	[ $# -gt 0 ] && disk_image="$1"

	local options=

	# LAN interface: connect to it using another qemu instance with socket
	# backend, e.g. "-netdev socket,connect=127.0.0.1:1234".
	options="$options -netdev socket,id=lan,listen=:1234 -device e1000,netdev=lan"

	# WAN interface: gets default address 10.0.2.15.
	# ssh is available as "ssh -p 2222 admin@localhost".
	# https is available as "https://localhost:4443/".
	options="$options -netdev user,id=wan,hostfwd=tcp::2222-:22,hostfwd=tcp::4443-:443 -device e1000,netdev=wan"

	# Memory size in Mbytes.
	options="$options -m 1G"

	# Redirect serial port to stdin/stdout.
	options="$options -serial mon:stdio -nographic"

	# The disk image.
	options="$options -drive id=diskA,file=$disk_image,if=none,cache=unsafe"
	options="$options -device ahci,id=ahci"
	options="$options -device ide-hd,drive=diskA,bus=ahci.0"

	# Use KVM if we can.
	kvm-ok >/dev/null && options="$options -enable-kvm"

	qemu-system-x86_64 $options
}

start_raspi()
{
	[ $# -ne 1 ] && usage_and_exit
	local raspi_dir="$1"

	local options=

	# SD card, kernel and device tree images.
	options="$options -sd $raspi_dir/raspberrypi-image-shrink.img"
	options="$options -kernel $raspi_dir/kernel8.img"
	options="$options -dtb $raspi_dir/bcm2710-rpi-3-b-plus.dtb"

	# Machine.
	options="$options -machine raspi3"

	# Kernel command line.
	options="$options -append 'rw earlyprintk loglevel=8 console=ttyAMA0,115200 dwc_otg.lpm_enable=0 root=/dev/mmcblk0p2 rootdelay=1'"

	# Memory size.
	options="$options -m 1G"

	# Number of CPUs.
	options="$options -smp 4"

	# Redirect serial port to stdin/stdout.
	options="$options -serial mon:stdio -nographic"

	# Connect eth0 to a VirtualDAL instance.
	options="$options -netdev socket,id=eth0,connect=127.0.0.1:1234 -device usb-net,netdev=eth0"

	# eth1: gets default address 10.0.2.15.
	# ssh is available as "ssh -p 22120 digi@localhost".
	# Also the following ports at localhost, which the Pi forwards to
	# VirtualDAL:
	#   ssh: 2220 -> 2222
	#   https: 44320 -> 443
	#   snmp: 16120 -> 1610
	options="$options -netdev user,id=eth1,restrict=on"
	options="$options,hostfwd=tcp::22120-:22"
	options="$options,hostfwd=tcp::2220-:2222"
	options="$options,hostfwd=tcp::44320-:443"
	options="$options,hostfwd=udp::16120-:1610"
	options="$options -device usb-net,netdev=eth1"

	eval qemu-system-aarch64 $options
}

[ $# -lt 1 ] && usage_and_exit

case "$1" in
	virtualdal)
		shift
		start_virtualdal "$@"
		;;
	raspi)
		shift
		start_raspi "$@"
		;;
	*)
		usage_and_exit
		;;
esac

exit 0

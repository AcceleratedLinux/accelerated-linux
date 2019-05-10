#!/bin/sh
#
# first time setup for MAC/serial numbers
#

set_macs() {
	new=$1
	c=0
	for n in ethaddr eth1addr eth2addr eth3addr eth4addr; do
		mac=$(printf "00:27:04:%02x:%02x:%02x" $((new / 0x10000)) $(( (new & 0xff00) >> 8 )) $((new & 0xff)) )
		echo "Setting MAC$c: $mac ..."
		fw_setenv $n $mac
		new=$((new + 1))
		c=$((c + 1))
	done
}

echo "================================================="
echo ""
while :; do
	echo -n "Enter Serial Number: "
	read t
	case "$t" in
	940001??????????)
		echo "Setting serial number to: $t ..."
		fw_setenv serial "$t"
		break
		;;
	*)
		echo "Invalid serial number"
		;;
	esac
done
echo ""
while :; do
	echo -n "Enter first MAC address: "
	read t
	case "$t" in
	00:27:04:??:??:??|002704??????)
		st=$(echo "$t" | sed 's/://g')
		st=$(expr "$st" : "002704\(......\)")
		num=$(printf "%d" 0x$st)
		set_macs $num
		break
		;;
	*)
		echo "Invalid serial number"
		;;
	esac
done

echo "================================================="
exit 0

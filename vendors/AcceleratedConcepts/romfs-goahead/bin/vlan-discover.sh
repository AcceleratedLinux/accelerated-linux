#!/bin/sh
ip link set dev br0 down
brctl delbr br0
tcpdump -s 100 -eni ${eth2} 1>/tmp/vlan-spy 2>&1 &
sleep 1
udhcpc -i ${eth2} -T 0 -t 1 -A0 -q -n -s /dev/null -f
sleep 1
# $! not supported, can't kill by pid
pkill tcpdump
vlanid=$(grep vlan /tmp/vlan-spy | tail -n 1)
vlanid=${vlanid##*vlan }
vlanid=${vlanid%%,*}
rm /tmp/vlan-spy

vconfig add ${eth2} $vlanid
udhcpc -i ${eth2}.$vlanid -T 5 -t 5 -A 0 -s /sbin/udhcpc.sh
smx-query.sh 0 skip_autoconfig
reboot

#!/bin/sh

# Description:
# unload wifi driver and apps to free memory for firmware upload purpose

# Pitfall:
# In 2.4.x the ip of br0 was determined by min(raxx, eth2x),
# so two possible condtions:
# 1) br0 mac = eth2x mac .... just unload wifi driver
# 2) br0 mac = raxx mac  .... unload br0 and wifi driver
#                             then mirror br0 to eth2x
#
# In 2.6.17 later the kernel supports to change br0 MAC with 
# ifconfig command so this script is not needed anymore.
#

. /sbin/global.sh

#kill_apps="udhcpd udhcpc syslogd klogd zebra ripd wscd rt2860apd rt61apd inadyn \
#iwevent stupid-ftpd smbd ated ntpclient lld2d igmpproxy dnsmasq telnetd"
kill_apps="udhcpd udhcpc syslogd klogd zebra ripd wscd inadyn iwevent \
	stupid-ftpd smbd ated ntpclient lld2d igmpproxy dnsmasq telnetd"

bssidnum=`flashconfig.sh get BssidNum`
is_ra0_in_br0=`brctl show | sed -n "/${ra0}/p"`
is_eth21_in_br0=`brctl show | sed -n "/${eth2}\\.1/p"`
is_usb0_in_br0=`brctl show | sed -n '/usb0/p'`
br0_mirror=${eth2}

#unload_ra0()
#{
#	ifconfig ${ra0} down
#	ifconfig ${eth2}.2 down
#	rmmod rt2860v2_ap 2>/dev/null
#	rmmod rt2860v2_sta 2>/dev/null
#}

unload_ra0br0()
{
	br0_mac=`ifconfig br0 | sed -n '/HWaddr/p' | sed -e 's/.*HWaddr \(.*\)/\1/'`
	br0_ip=`ifconfig br0 | sed -n '/inet addr:/p' | sed -e 's/ *inet addr:\(.*\)  Bcast.*/\1/'`
	br0_netmask=`ifconfig br0 | sed -n '/inet addr:/p' | sed -e 's/.*Mask:\(.*\)/\1/'`
	ra0_mac=`ifconfig ${ra0} | sed -n '/HWaddr/p' | sed -e 's/.*HWaddr\ \(.*\)/\1/'`

	if [ "$ra0_mac" = "$br0_mac" ]; then
		#destory br0
		ifconfig br0 down
		brctl delbr br0

		#unload_ra0

		#mirror br0 to eth2x
		ifconfig $1 down
		ifconfig $1 hw ether $br0_mac
		ifconfig $1 $br0_ip netmask $br0_netmask
		ifconfig $1 up
	#else
	#	unload_ra0
	fi
}

# unload apps
for apps in $kill_apps
do
	killall -q $apps
done
for apps in $kill_apps
do
	killall -q -9 $apps
done


# unload wifi driver
if [ "$is_ra0_in_br0" == "" ]; then
	#unload_ra0
	exit 1
else
	if [ "$is_usb0_in_br0" != "" ]; then
		exit 1
	fi

	if [ "$is_eth21_in_br0" != "" ]; then
		br0_mirror=${eth2}.1
	fi

	unload_ra0br0 $br0_mirror
	exit 1
fi

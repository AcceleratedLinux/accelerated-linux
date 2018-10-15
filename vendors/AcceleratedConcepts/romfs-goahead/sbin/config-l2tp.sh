#!/bin/sh
#
# $Id: config-l2tp.sh,v 1.6 2008-12-26 10:36:21 steven Exp $
#
# usage: config-l2tp.sh <mode> <mode_params> <server> <user> <password>
#

. /sbin/config.sh
. /sbin/global.sh
Status_dhcpc=1
dhcpc_time_out=10

#james add
waitting_dhcpc () {

        local devcount=0;sleep_time=1
        while [ ! -e  /etc/resolv.conf ]
        do
          sleep $sleep_time
          if [ $devcount -gt $1 ]; then
             ##DWAN_DEBUG echo "over $1 sec !!!"  >/dev/console
             return 0
          else
             devcount=`expr $devcount + $sleep_time`
          fi
        done
        return 1
}


usage()
{
	echo "Usage:"
	echo "  $0 <mode> <mode_params> <server> <user> <password>"
	echo "Modes:"
	echo "  static - <mode_params> = <wan_if_name> <wan_ip> <wan_netmask> <gateway>"
	echo "  dhcp - <mode_params> = <wan_if_name>"
	echo "Example:"
	echo "  $0 static eth2.2 10.10.10.254 255.255.255.0 10.10.10.253 192.168.1.1 user pass"
	echo "  $0 dhcp eth2.2 192.168.1.1 user pass"
	exit 1
}

if [ "$5" = "" ]; then
	echo "$0: insufficient arguments"
	usage $0
fi

if [ "$1" = "static" ]; then
	if [ "$7" = "" ]; then
		echo "$0: insufficient arguments"
		usage $0
	fi
	ifconfig $2 $3 netmask $4
	if [ ! -e /var/run/stop.run ]; then #kent_chang, 20091222, don't delete current wan's default gw when try master wan
		route del default
		if [ "$5" != "0.0.0.0" ]; then
			route add default gw $5
		fi
	fi
	l2tp_srv=$6
	l2tp_u=$7
	l2tp_pw=$8
	l2tp_opmode=$9
	l2tp_optime=${10}
elif [ "$1" = "dhcp" ]; then
	killall -q udhcpc
	udhcpc -i $2 -s /sbin/udhcpc.sh -p /var/run/udhcpd.pid &
	l2tp_srv=$3
	l2tp_u=$4
	l2tp_pw=$5
	l2tp_opmode=$6
	l2tp_optime=$7
else
	echo "$0: unknown connection mode: $1"
	usage $0
fi

if [ "$CONFIG_PPPOL2TP" == "y" ]; then
rm -f /var/run/openl2tpd.pid
openl2tp.sh $l2tp_u $l2tp_pw $l2tp_srv $l2tp_opmode $l2tp_optime
openl2tpd
else
l2tp.sh $l2tp_u $l2tp_pw $l2tp_srv $l2tp_opmode $l2tp_optime

#james add
if [ "$1" = "dhcp" ]; then
    waitting_dhcpc $dhcpc_time_out
    if [ $? = 0 ]; then
       echo "james: dhcpc failed"
       Status_dhcpc=88
    fi
fi

l2tpd
sleep 1
l2tp-control "start-session $l2tp_srv"
fi
return $Status_dhcpc


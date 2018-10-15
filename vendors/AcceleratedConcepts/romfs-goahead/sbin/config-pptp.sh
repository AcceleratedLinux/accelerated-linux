#!/bin/sh
#
# $Id: config-pptp.sh,v 1.6 2009-01-07 09:48:14 steven Exp $
#
# usage: config-pptp.sh <mode> <mode_params> <server> <user> <password>
#

. /sbin/config.sh
. /sbin/global.sh
dhcpc_time_out=10
Status_dhcpc=1
wait_dhcpc=/tmp/wan_pptp.wait

wan2=`flashconfig.sh get wan2`

#james add
waitting_dhcpc () {

        local devcount=0;sleep_time=1
        ##while [ ! -e  "$wait_dhcpc" ]
	while [ -e "$wait_dhcpc" ]
        do
          sleep $sleep_time
          if [ $devcount -gt $1 ]; then
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
	if [ ! -e /var/run/stop.run ]; then 
	route del default
	if [ "$5" != "0.0.0.0" ]; then
		route add default gw $5
	fi
	fi
	pptp_srv=$6
	pptp_u=$7
	pptp_pw=$8
	pptp_opmode=$9
	pptp_optime=${10}
elif [ "$1" = "dhcp" ]; then
	killall -q udhcpc
        [ ! -e "$wait_dhcpc" ] && touch $wait_dhcpc
	udhcpc -i $2 -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid &
        #[ ! -e "$wait_dhcpc" ] && touch $wait_dhcpc
	pptp_srv=$3
	pptp_u=$4
	pptp_pw=$5
	pptp_opmode=$6
	pptp_optime=$7
else
	echo "$0: unknown connection mode: $1"
	usage $0
fi

if [ "$CONFIG_PPPOPPTP" == "y" ]; then
accel-pptp.sh $pptp_u $pptp_pw $pptp_srv $pptp_opmode $pptp_optime
else
pptp.sh $pptp_u $pptp_pw $pptp_srv $pptp_opmode $pptp_optime
fi

#james add
if [ "$1" = "dhcp" ]; then
    waitting_dhcpc $dhcpc_time_out
    if [ $? = 0 ]; then
       [ -e /tmp/wan_pptp.wait ] && rm -f /tmp/wan_pptp.wait
       echo "dhcpc failed"
       Status_dhcpc=88
       if [ $wan2 != 99 ]; then
           return $Status_dhcpc 
       fi
    fi
fi

#pppd here is 2.4.4
#/usr/sbin/pppd file /etc/options.pptp &
/bin/pppd file /etc/options.pptp &
return $Status_dhcpc

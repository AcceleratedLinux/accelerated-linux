#!/bin/sh
#
# $Id: lan.sh,v 1.22.6.1 2008-10-02 12:57:42 winfred Exp $
#
# usage: lan_pre_wan.sh
#

. /sbin/global.sh

fname="/etc/udhcpd.conf"
echo "=====lan-pre-wan.sh:start======"

# stop all
killall -q udhcpd
killall -q lld2d
killall -q igmpproxy
killall -q upnpd
killall -q radvd
killall -q pppoe-relay
killall -q dnsmasq
rm -rf /var/run/lld2d-*
echo "" > /var/udhcpd.leases

# ip address
ip=`flashconfig.sh get lan_ipaddr`
nm=`flashconfig.sh get lan_netmask`
[ ! "`ifconfig $lan_if 2>&1 >/dev/null`" ] && ifconfig $lan_if down
[ ! "`ifconfig $lan_if 2>&1 >/dev/null`" ] && ifconfig $lan_if $ip netmask $nm


[ "`ifconfig -a | grep br0:9 2>&1`" ] && ifconfig "br0:9" down
[ "`ifconfig -a | grep ${eth2}:9 2>&1`" ] && ifconfig "${eth2}:9" down
lan2enabled=`flashconfig.sh get Lan2Enabled`
if [ "$lan2enabled" = "1" ]; then
	ip_2=`flashconfig.sh get lan2_ipaddr`
	nm_2=`flashconfig.sh get lan2_netmask`
	if [ "$opmode" = "0" ]; then
		[ ! "`ifconfig br0:9 2>&1 >/dev/null`" ] && ifconfig "br0:9" "$ip_2" netmask "$nm_2"
		echo "ifconfig "br0:9" "$ip_2" netmask "$nm_2""
	elif [ "$opmode" = "1" ]; then
		[ ! "`ifconfig br0:9 2>&1 >/dev/null`" ] && ifconfig "br0:9" "$ip_2" netmask "$nm_2"
		echo "ifconfig "br0:9" "$ip_2" netmask "$nm_2""
	elif [ "$opmode" = "2" ]; then
		[ ! "`ifconfig ${eth2}:9 2>&1 >/dev/null`" ] && ifconfig "${eth2}:9" "$ip_2" netmask "$nm_2"
		echo "ifconfig "${eth2}:9" "$ip_2" netmask "$nm_2""
	elif [ "$opmode" = "3" ]; then
		[ ! "`ifconfig br0:9 2>&1 >/dev/null`" ] && ifconfig "br0:9" "$ip_2" netmask "$nm_2"
		echo "ifconfig "br0:9" "$ip_2" netmask "$nm_2""
	fi
fi


# 2009/5/25 simon_ho
# remove old udhcpd.conf everytime
# 2009/5/26 simon_ho
# detect if dhcp server config exists then remove
# dhcp server
if [ "$opmode" = "1" ]; then
dhcp=`flashconfig.sh get dhcpEnabled`
if [ "$dhcp" = "1" ]; then
	if [ -f $fname ]; then
		rm -f $fname
	fi
	start=`flashconfig.sh get dhcpStart`
	end=`flashconfig.sh get dhcpEnd`
	mask=`flashconfig.sh get dhcpMask`
	pd=`flashconfig.sh get dhcpPriDns`
	sd=`flashconfig.sh get dhcpSecDns`
	gw=`flashconfig.sh get dhcpGateway`
	lease=`flashconfig.sh get dhcpLease`
	static1=`flashconfig.sh get dhcpStatic1 | sed -e 's/;/ /'`
	static2=`flashconfig.sh get dhcpStatic2 | sed -e 's/;/ /'`
	static3=`flashconfig.sh get dhcpStatic3 | sed -e 's/;/ /'`
	config-udhcpd.sh -s $start
	config-udhcpd.sh -e $end
	config-udhcpd.sh -i $lan_if
	config-udhcpd.sh -m $mask
	#if [ "$pd" != "" -o "$sd" != "" ]; then
		config-udhcpd.sh -d $pd $sd
	#fi
	if [ "$gw" != "" ]; then
		config-udhcpd.sh -g $gw
	fi
	if [ "$lease" != "" ]; then
		config-udhcpd.sh -t $lease
	fi
	if [ "$static1" != "" ]; then
		config-udhcpd.sh -S $static1
	fi
	if [ "$static2" != "" ]; then
		config-udhcpd.sh -S $static2
	fi
	if [ "$static3" != "" ]; then
		config-udhcpd.sh -S $static3
	fi
	config-udhcpd.sh -r
fi
fi

# stp
if [ "$wan_if" = "br0" -o "$lan_if" = "br0" ]; then
	stp=`flashconfig.sh get stpEnabled`
	if [ "$stp" = "1" ]; then
		brctl setfd br0 15
		brctl stp br0 1
	else
		brctl setfd br0 1
		brctl stp br0 0
	fi
fi
echo "=====lan-pre-wan.sh:end======"

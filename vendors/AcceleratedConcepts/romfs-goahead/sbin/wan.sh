#!/bin/sh
#
# $Id: wan.sh,v 1.18 2009-02-09 13:29:37 michael Exp $
#
# usage: wan.sh
#

. /sbin/global.sh

# stop all
killall -q syslogd
killall -q udhcpc
killall -q dhcpcd
killall -q pppd
killall -q l2tpd
killall -q openl2tpd


clone_en=`flashconfig.sh get macCloneEnabled`
clone_mac=`flashconfig.sh get macCloneMac`
#MAC Clone: bridge mode doesn't support MAC Clone
if [ "$opmode" != "0" -a "$clone_en" = "1" ]; then
	ifconfig $wan_if down
	#if [ "$opmode" = "2" ]; then
	#	rmmod rt2860v2_sta
	#	insmod rt2860v2_sta mac=$clone_mac
	#else
	#	ifconfig $wan_if hw ether $clone_mac
	#fi
	ifconfig $wan_if hw ether $clone_mac
	ifconfig $wan_if up
fi

if [ "$wanmode" = "STATIC" -o "$opmode" = "0" ]; then
	#always treat bridge mode having static wan connection
	ip=`flashconfig.sh get wan_ipaddr`
	nm=`flashconfig.sh get wan_netmask`
	gw=`flashconfig.sh get wan_gateway`
	pd=`flashconfig.sh get wan_primary_dns`
	sd=`flashconfig.sh get wan_secondary_dns`

	smxgw=`flashconfig.sh get smx_defgw`
  if [ "$smxgw" != '' ]; then
    gw="$smxgw"
    pd="$smxgw"
    sd="$smxgw"
  fi

	#lan and wan ip should not be the same except in bridge mode
	if [ "$opmode" != "0" ]; then
		lan_ip=`flashconfig.sh get lan_ipaddr`
		if [ "$ip" = "$lan_ip" ]; then
			echo "wan.sh: warning: WAN's IP address is set identical to LAN"
			exit 0
		fi
	else
		#use lan's ip address instead
		ip=`flashconfig.sh get lan_ipaddr`
		nm=`flashconfig.sh get lan_netmask`
	fi
  if [ ! "$vlanid_unit" ]; then
	  ifconfig $wan_if $ip netmask $nm
  fi
	route del default
	if [ "$gw" != "" ]; then
	route add default gw $gw
	fi
	config-dns.sh $pd $sd
elif [ "$wanmode" = "DHCP" ]; then
	dhcp_host=`flashconfig.sh get HostName`
	if  [ "$dhcp_host" = "" ]; then
		udhcpc -i $wan_if -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid &
	else
		udhcpc -i $wan_if -H $dhcp_host -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid &
	fi
elif [ "$wanmode" = "PPPOE" ]; then
	u=`flashconfig.sh get wan_pppoe_user`
	pw=`flashconfig.sh get wan_pppoe_pass`
	pppoe_opmode=`flashconfig.sh get wan_pppoe_opmode`
	if [ "$pppoe_opmode" = "" ]; then
		echo "pppoecd $wan_if -u $u -p $pw"
		pppoecd $wan_if -u "$u" -p "$pw"
	else
		pppoe_optime=`flashconfig.sh get wan_pppoe_optime`
		config-pppoe.sh $u $pw $wan_if $pppoe_opmode $pppoe_optime
	fi
elif [ "$wanmode" = "L2TP" ]; then
	srv=`flashconfig.sh get wan_l2tp_server`
	u=`flashconfig.sh get wan_l2tp_user`
	pw=`flashconfig.sh get wan_l2tp_pass`
	mode=`flashconfig.sh get wan_l2tp_mode`
	l2tp_opmode=`flashconfig.sh get wan_l2tp_opmode`
	l2tp_optime=`flashconfig.sh get wan_l2tp_optime`
	if [ "$mode" = "0" ]; then
		ip=`flashconfig.sh get wan_l2tp_ip`
		nm=`flashconfig.sh get wan_l2tp_netmask`
		gw=`flashconfig.sh get wan_l2tp_gateway`
		if [ "$gw" = "" ]; then
			gw="0.0.0.0"
		fi
		config-l2tp.sh static $wan_if $ip $nm $gw $srv $u $pw $l2tp_opmode $l2tp_optime
	else
		config-l2tp.sh dhcp $wan_if $srv $u $pw $l2tp_opmode $l2tp_optime
	fi
elif [ "$wanmode" = "PPTP" ]; then
	srv=`flashconfig.sh get wan_pptp_server`
	u=`flashconfig.sh get wan_pptp_user`
	pw=`flashconfig.sh get wan_pptp_pass`
	mode=`flashconfig.sh get wan_pptp_mode`
	pptp_opmode=`flashconfig.sh get wan_pptp_opmode`
	pptp_optime=`flashconfig.sh get wan_pptp_optime`
	if [ "$mode" = "0" ]; then
		ip=`flashconfig.sh get wan_pptp_ip`
		nm=`flashconfig.sh get wan_pptp_netmask`
		gw=`flashconfig.sh get wan_pptp_gateway`
		if [ "$gw" = "" ]; then
			gw="0.0.0.0"
		fi
		config-pptp.sh static $wan_if $ip $nm $gw $srv $u $pw $pptp_opmode $pptp_optime
	else
		config-pptp.sh dhcp $wan_if $srv $u $pw $pptp_opmode $pptp_optime
	fi
elif [ "$wanmode" = "3G" ]; then
        dev=`flashconfig.sh get wan_3g_dev`
        3g.sh $dev &
else
	echo "wan.sh: unknown wan connection type: $wanmode"
	exit 1
fi


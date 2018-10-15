#!/bin/sh
#
# $Id: nat.sh,v 1.2 2009-02-09 13:29:22 michael Exp $
#
# usage: nat.sh
#

. /sbin/global.sh

# Jason, 20090726
Module_type_3g="SMCWBR14S-3GN"
Module_type="SMCWBR14S-N3"

#Platform=`flashconfig.sh get Platform`
Platform="SMCWBR14S-3GN"

if [ "$Platform" = "$Module_type_3g" ]; then
	# Jason, 20090726
	[ -e /var/run/stop.run ] && exit 0

	# Jason, 20090726
	RC_CONF=/var/run/rc.conf
	IN_CONF=/etc/init.d/load_balance.in
	# Jason, 20090726
	. $RC_CONF
	. $IN_CONF
fi
lan_ip=`flashconfig.sh get lan_ipaddr`
nat_en=`flashconfig.sh get natEnabled`


echo 1 > /proc/sys/net/ipv4/ip_forward

if [ "$nat_en" = "1" ]; then

start () {
   num=`iptables -t nat -n --line-numbers -L PREROUTING | grep "LOVEJ" | cut -d' ' -f1`
   if [ "X$num" != "X" ]; then 
      iptables -t nat -D PREROUTING $num
      iptables -t nat -I PREROUTING $num -i $wan_if -d $lan_ip/24 -m comment --comment "LOVEJ" -j DROP
   else
      iptables -t nat -A PREROUTING -i $wan_if -d $lan_ip/24 -m comment --comment "LOVEJ" -j DROP
   fi
   
   num=`iptables -t nat -n --line-numbers -L POSTROUTING | grep "LOVEC" | cut -d' ' -f1`
   if [ "X$num" != "X" ]; then 
      iptables -t nat -D POSTROUTING $num
      iptables -t nat -I POSTROUTING $num -s $lan_ip/24 -o $wan_if -m comment --comment "LOVEC" -j MASQUERADE
   else
      iptables -t nat -A POSTROUTING -s $lan_ip/24 -o $wan_if -m comment --comment "LOVEC" -j MASQUERADE
   fi
}

set_wan_inf () {
# if [ "$wanmode" = "PPPOE" -o "$wanmode" = "L2TP" -o "$wanmode" = "PPTP" ]; then
 if [ "$wanmode" = "PPPOE" -o "$wanmode" = "L2TP" -o "$wanmode" = "PPTP" -o "$wanmode" = "G3G" ]; then
  wan_if="ppp0"
 else
  wan_if="${eth2}.2"
 fi
}


	echo 180 > /proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeout
	echo 180 > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established
	if [ "$Platform" = "$Module_type_3g" -o "$Platform" = "$Module_type_3g_Qisda" ]; then
  		check_backup_mode
  		if [ "$?" = "1" ]; then
   			run_interface
   			wan_if=$run_if
  		else
   			set_wan_inf
  		fi
 	else
  		set_wan_inf
 	fi
 
[ "$1" = "start" -o "X$1" = "X" ] && start
fi


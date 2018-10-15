#!/bin/sh
#
# $Id: static_ip.sh,v 1.11 2009-07-23 06:39:13 steven Exp $
#
# usage: static_ip.sh
#
SCRIPT=$0

RC_CONF=/var/run/rc.conf
IN_CONF=/etc/init.d/load_balance.in
dual_wan_start_detect_network=/var/run/start_detect

. /sbin/global.sh
. $IN_CONF
. $RC_CONF

check_backup_mode
if [ "$?" = "0" ]; then
 wanmode=`flashconfig.sh get wanConnectionMode`
 if [ "$wanmode" = "STATIC" ]; then
    wan_ip_assignment=0
 else
    wan_ip_assignment=$wanmode
 fi
fi

start () {
 if [ "$wan_ip_assignment" = "0" -a "$opmode" != "0" ]; then
	#always treat bridge mode having static wan connection
	ip=`flashconfig.sh get wan_ipaddr`
	nm=`flashconfig.sh get wan_netmask`
	gw=`flashconfig.sh get wan_gateway`
	pd=`flashconfig.sh get wan_primary_dns`
	sd=`flashconfig.sh get wan_secondary_dns`

	if [ -z "$gw" ]; then
		gw='10.10.10.1'
	fi
	if [ -z "$pd" ]; then
		pd='10.10.10.1'
	fi
	if [ -z "$sd" ]; then
		sd='10.10.10.1'
	fi

	#lan and wan ip should not be the same except in bridge mode
	if [ "$opmode" != "0" ]; then
		lan_ip=`flashconfig.sh get lan_ipaddr`
		if [ "$ip" = "$lan_ip" ]; then
			echo "wan.sh: warning: WAN's IP address is set identical to LAN"
      return 1
		fi
	else
		#use lan's ip address instead
		ip=`flashconfig.sh get lan_ipaddr`
		nm=`flashconfig.sh get lan_netmask`
	fi
	[ ! "`ifconfig $wan_if 2>&1 >/dev/null`" ] && ifconfig $wan_if $ip netmask $nm
	if [ "$gw" != "" -a ! -e /var/run/stop.run ]; then
	   route del default >/dev/null 2>&1
	   route add default gw $gw >/dev/null 2>&1
        fi
  if [ ! -e /var/run/stop.run ]; then
     config-dns.sh $pd $sd
  fi
  [ "$wan_dual_wan_backup" != "99" ] && touch $dual_wan_start_detect_network > /dev/null
  return 0
 fi
 return 1
}
 
stop () {
  ifconfig $wan_if 0.0.0.0
}

usage () {
        echo "$0 [start|stop]"
        exit 1
}

# main ##########################################################

[ -z "$1" ] && usage;

err=0

for action in $*; do
	case $action in
		start)		start;;
		stop)		stop;;
		*)		usage;;
	esac
	if [ $? = "0" ] ; then
		echo $SCRIPT $action ok
	else
		echo $SCRIPT $action error
		err=1
	fi
done

exit $err

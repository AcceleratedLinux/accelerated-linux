#!/bin/sh

# udhcpc script edited by Tim Riker <Tim@Rikers.org>

[ -z "$1" ] && echo "Error: should be called from udhcpc" && exit 1

env > /tmp/smx-dhcp.log

. /sbin/config.sh
. /sbin/global.sh

# Jason, 20090726
Module_type_3g="SMCWBR14S-3GN"

#Platform=`flashconfig.sh get Platform`
Platform="SMCWBR14S-3GN"

if [ "$Platform" = "$Module_type_3g" ]; then
	# Jason, 20090726
	RC_CONF=/var/run/rc.conf
	dual_wan_start_detect_network=/var/run/start_detect
	. $RC_CONF
fi

RESOLV_CONF="/etc/resolv.conf"
[ -n "$broadcast" ] && BROADCAST="broadcast $broadcast"
[ -n "$subnet" ] && NETMASK="netmask $subnet"

case "$1" in
    deconfig)
        /sbin/ifconfig $interface 0.0.0.0
        ;;

    renew|bound)
        /sbin/ifconfig $interface $ip $BROADCAST $NETMASK

        if [ -n "$router" ] ; then
            echo "deleting routers"
            while route del default gw 0.0.0.0 dev $interface ; do
                :
            done

            metric=0
            for i in $router ; do
                metric=`expr $metric + 1`
                [ ! -e /var/run/stop.run ] && route add default gw $i dev $interface metric $metric
            done
        fi

	# Jason, 20090726
	if [ ! -e /var/run/stop.run ]; then
        echo -n > $RESOLV_CONF
        [ -n "$domain" ] && echo search $domain >> $RESOLV_CONF
        for i in $dns ; do
            echo adding dns $i
            echo nameserver $i >> $RESOLV_CONF
        done
        
# Added dns setting (custom) [2009/08/15] - start
  dhcpc_PriDns=`flashconfig.sh get wan_primary_dns`
	dhcpc_SecDns=`flashconfig.sh get wan_secondary_dns`
	
	echo adding dns by customer $dhcpc_PriDns $dhcpc_SecDns

	if  [ "$dhcpc_PriDns" != "" ]; then
		echo nameserver $dhcpc_PriDns > /etc/resolv.conf
		
		if  [ "$dhcpc_SecDns" != "" ]; then
			echo nameserver $dhcpc_SecDns >> /etc/resolv.conf
		fi
	elif  [ "$dhcpc_SecDns" != "" ]; then
		echo nameserver $dhcpc_SecDns > /etc/resolv.conf
	fi
# Added dns setting (custom) [2009/08/15] - End	

	# notify goahead when the WAN IP has been acquired. --yy
	killall -SIGUSR2 goahead

	# restart igmpproxy daemon
	config-igmpproxy.sh 
	
	# 
	#[ -e /tmp/wan_pptp.wait ] && rm -f /tmp/wan_pptp.wait
 	fi  
	[ -e /tmp/wan_pptp.wait ] && rm -f /tmp/wan_pptp.wait
	# Jason 20090727 for dual-wan detect netwrok
	[ "$wan_dual_wan_backup" != "99" ] && touch $dual_wan_start_detect_network > /dev/null
        ;;
esac
exit 0

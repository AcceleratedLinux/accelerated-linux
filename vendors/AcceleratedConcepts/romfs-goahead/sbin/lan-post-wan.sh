#!/bin/sh
#
# $Id: lan.sh,v 1.22.6.1 2008-10-02 12:57:42 winfred Exp $
#
# usage: wan.sh
#
echo "=====lan-post-wan.sh:start======"
# Jason, 20090726
[ -e /var/run/stop.run ] && exit 0

. /sbin/global.sh

# stop all
#killall -q udhcpd
killall -q lld2d
killall -q igmpproxy
killall -q upnpd
killall -q radvd
killall -q pppoe-relay
killall -q dnsmasq
rm -rf /var/run/lld2d-*
#echo "" > /var/udhcpd.leases


opmode=`flashconfig.sh get OperationMode`
if [ "$opmode" = "0" ]; then
	gw=`flashconfig.sh get wan_gateway`
	pd=`flashconfig.sh get wan_primary_dns`
	sd=`flashconfig.sh get wan_secondary_dns`

	smxgw=`flashconfig.sh get smx_defgw`
  if [ "$smxgw" != '' ]; then
    gw="$smxgw"
    pd="$smxgw"
    sd="$smxgw"
  fi

	route del default
	route add default gw $gw
	config-dns.sh $pd $sd
fi

# lltd
lltd=`flashconfig.sh get lltdEnabled`
if [ "$lltd" = "1" ]; then
	lld2d $lan_if
fi

# igmpproxy
igmp=`flashconfig.sh get igmpEnabled`
if [ "$igmp" = "1" ]; then
#	config-igmpproxy.sh $wan_if $lan_if
  killall igmp_watch.sh
  igmp_watch.sh&
fi

# upnp
if [ "$opmode" = "0" -o "$opmode" = "1" ]; then
	upnp=`flashconfig.sh get upnpEnabled`
	if [ "$upnp" = "1" ]; then
		route add -net 239.0.0.0 netmask 255.0.0.0 dev $lan_if
		upnp_xml.sh $ip
		upnpd -f $wan_ppp_if $lan_if &
	fi
fi

# radvd
radvd=`flashconfig.sh get radvdEnabled`
[ ! "`ifconfig sit0 2>&1 >/dev/null`" ] && ifconfig sit0 down
# Jason, 20090726
#echo "0" > /proc/sys/net/ipv6/conf/all/forwarding
if [ "$radvd" = "1" ]; then
	echo "1" > /proc/sys/net/ipv6/conf/all/forwarding
	[ ! "`ifconfig sit0 2>&1 >/dev/null`" ] && ifconfig sit0 up
	[ ! "`ifconfig sit0 2>&1 >/dev/null`" ] && ifconfig sit0 add 2002:1101:101::1101:101/16
	route -A inet6 add 2000::/3 gw ::17.1.1.20 dev sit0
	route -A inet6 add 2002:1101:101:0::/64 dev br0
	radvd -C /etc_ro/radvd.conf -d 1 &
fi

# pppoe-relay
pppr=`flashconfig.sh get pppoeREnabled`
if [ "$pppr" = "1" ]; then
	pppoe-relay -S $wan_if -B $lan_if
fi

# dns proxy
dnsp=`flashconfig.sh get dnsPEnabled`
if [ "$dnsp" = "1" ]; then
	dnsmasq &
fi
echo "=====lan-post-wan.sh:end======"

#!/bin/sh
#
# $Id: lan.sh,v 1.24.2.1 2009-04-21 12:49:37 michael Exp $
#
# usage: wan.sh
#

. /sbin/global.sh

##-3G Dual WAN Add Start:  
  LAN_PRE="yes"
  LAN_POST="yes"
  
if [ "$1" = "PRE" ]; then
   echo "===lan pre-wan==="
   LAN_PRE="yes"
   LAN_POST="no"
fi
if [ "$1" = "POST" ]; then
   echo "====lan post-wan==="
   LAN_PRE="no"
   LAN_POST="yes"
fi
##-3G Dual WAN Add End:  

#------lan-pre-wan start-----
if [ $LAN_PRE = "yes" ]; then
   echo "====== LAN_PRE ========="
# stop all
killall -q udhcpd
killall -q lld2d
killall -q igmpproxy
killall -q upnpd
killall -q miniupnpd
killall -q radvd
killall -q pppoe-relay
killall -q dnsmasq
rm -rf /var/run/lld2d-*
echo "" > /var/udhcpd.leases

echo "" > /etc/udhcpd.conf

# ip address
ip=`flashconfig.sh get lan_ipaddr`
nm=`flashconfig.sh get lan_netmask`
ifconfig $lan_if down
ifconfig $lan_if $ip netmask $nm
ifconfig $lan_if up
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

ifconfig "br0:9" down
ifconfig "${eth2}:9" down
lan2enabled=`flashconfig.sh get Lan2Enabled`
if [ "$lan2enabled" = "1" ]; then
	ip_2=`flashconfig.sh get lan2_ipaddr`
	nm_2=`flashconfig.sh get lan2_netmask`
	if [ "$opmode" = "0" ]; then
		ifconfig "br0:9" "$ip_2" netmask "$nm_2"
		echo "ifconfig "br0:9" "$ip_2" netmask "$nm_2""
	elif [ "$opmode" = "1" ]; then
		ifconfig "br0:9" "$ip_2" netmask "$nm_2"
		echo "ifconfig "br0:9" "$ip_2" netmask "$nm_2""
	elif [ "$opmode" = "2" ]; then
		ifconfig "${eth2}:9" "$ip_2" netmask "$nm_2"
		echo "ifconfig "${eth2}:9" "$ip_2" netmask "$nm_2""
	elif [ "$opmode" = "3" ]; then
		ifconfig "br0:9" "$ip_2" netmask "$nm_2"
		echo "ifconfig "br0:9" "$ip_2" netmask "$nm_2""
	fi
fi

# hostname
host=`flashconfig.sh get HostName`
if [ "$host" = "" ]; then
	host="NetReach"
	flashconfig.sh set HostName NetReach
fi
hostname $host
echo "127.0.0.1 localhost.localdomain localhost" > /etc/hosts
echo "$ip $host.accelecon.com $host" >> /etc/hosts

# dhcp server
dhcp=`flashconfig.sh get dhcpEnabled`
if [ "$dhcp" = "1" ]; then
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
	if [ "$pd" != "" -o "$sd" != "" ]; then
		config-udhcpd.sh -d $pd $sd
	fi
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
	config-udhcpd.sh -r 1
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
   echo "====== LAN_PRE:end ========="
fi
#------lan-pre-wan end-----
#------lan-post-wan start-----
if [ $LAN_POST = "yes" ]; then
   echo "====== LAN_POST ========="
################ kill lld2d ###############
echo "## stop lld2d ##"
for pid in `pidof lld2d`; do
        	if [ ! -z "$pid" ]; then
                	found=`ps | grep $pid | grep "lld2d"`
                	if [ ! -z "$found" ]; then
             			kill -TERM $pid || err=1
			fi
		fi
done
rm -rf /var/run/lld2d-*
# restart lltd
echo "## restart lld2d ##"
lltd=`flashconfig.sh get lltdEnabled`
if [ "$lltd" = "1" ]; then
	lld2d $lan_if
fi

################ kill igmpproxy ###############
echo "## stop igmpproxy ##"
for pid in `pidof igmpproxy`; do
        	if [ ! -z "$pid" ]; then
                	found=`ps | grep $pid | grep "igmpproxy"`
                	if [ ! -z "$found" ]; then
             			kill -TERM $pid || err=1
			fi
		fi
done
# restart igmpproxy
echo "## restart igmpproxy ##"
igmp=`flashconfig.sh get igmpEnabled`
if [ "$igmp" = "1" ]; then
	config-igmpproxy.sh $wan_if $lan_if
fi

################ kill miniupnpd ###############
echo "## stop miniupnpd ##"
for pid in `pidof miniupnpd`; do
        	if [ ! -z "$pid" ]; then
                	found=`ps | grep $pid | grep "miniupnpd"`
                	if [ ! -z "$found" ]; then
             			kill -TERM $pid || err=1
			fi
		fi
done
# restart upnp
echo "## restart miniupnpd ##"
if [ "$opmode" = "0" -o "$opmode" = "1" ]; then
	upnp=`flashconfig.sh get upnpEnabled`
	if [ "$upnp" = "1" ]; then
		route add -net 239.0.0.0 netmask 255.0.0.0 dev $lan_if
		/bin/miniupnpd_config_create.sh $wan_ppp_if $lan_if
		/sbin/miniupnpd_init.sh $wan_ppp_if $lan_if
#		upnp_xml.sh $ip
#		upnpd -f $wan_ppp_if $lan_if &
	fi
fi

################ kill radvd ###############
echo "## stop radvd ##"
for pid in `pidof radvd`; do
        	if [ ! -z "$pid" ]; then
                	found=`ps | grep $pid | grep "radvd"`
                	if [ ! -z "$found" ]; then
             			kill -TERM $pid || err=1
			fi
		fi
done
# restart radvd
echo "## restart radvd ##"
radvd=`flashconfig.sh get radvdEnabled`
ifconfig sit0 down
echo "0" > /proc/sys/net/ipv6/conf/all/forwarding
if [ "$radvd" = "1" ]; then
	echo "1" > /proc/sys/net/ipv6/conf/all/forwarding
	ifconfig sit0 up
	ifconfig sit0 add 2002:1101:101::1101:101/16
	route -A inet6 add 2000::/3 gw ::17.1.1.20 dev sit0
	route -A inet6 add 2002:1101:101:0::/64 dev br0
	radvd -C /etc_ro/radvd.conf -d 1 &
fi

################ kill pppoe-relay ###############
echo "## stop pppoe-relay ##"
for pid in `pidof pppoe-relay`; do
        	if [ ! -z "$pid" ]; then
                	found=`ps | grep $pid | grep "pppoe-relay"`
                	if [ ! -z "$found" ]; then
             			kill -TERM $pid || err=1
			fi
		fi
done
# restart pppoe-relay
echo "## restart pppoe-relay ##"
pppr=`flashconfig.sh get pppoeREnabled`
if [ "$pppr" = "1" ]; then
	pppoe-relay -S $wan_if -B $lan_if
fi

################ kill dnsmasq ###############
echo "## stop dnsmasq ##"
for pid in `pidof dnsmasq`; do
        	if [ ! -z "$pid" ]; then
                	found=`ps | grep $pid | grep "dnsmasq"`
                	if [ ! -z "$found" ]; then
             			kill -TERM $pid || err=1
			fi
		fi
done
# resstart dns proxy
echo "## restart dnsmasq ##"
dnsp=`flashconfig.sh get dnsPEnabled`
if [ "$dnsp" = "1" ]; then
	dnsmasq &
fi
   echo "====== LAN_POST:end ========="
fi
#------lan-post-wan end-----

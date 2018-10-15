#!/bin/sh

#iptables -t nat -F MINIUPNPD > /dev/null 2>&1
#iptables -t nat -D PREROUTING -i $1 -j MINIUPNPD > /dev/null 2>&1
#iptables -t nat -X MINIUPNPD > /dev/null 2>&1
	
#iptables -t filter -F MINIUPNPD > /dev/null 2>&1
#iptables -t filter -D FORWARD -i $1 -o ! $1 -j MINIUPNPD > /dev/null 2>&1
#iptables -t filter -X MINIUPNPD > /dev/null 2>&1

#iptables -t nat -N MINIUPNPD > /dev/null 2>&1
#iptables -t nat -A PREROUTING -i $1 -j MINIUPNPD > /dev/null 2>&1
#iptables -t filter -N MINIUPNPD > /dev/null 2>&1
#iptables -t filter -A FORWARD -i $1 -o ! $1 -j MINIUPNPD > /dev/null 2>&1

iptables -t filter -N port_forward > /dev/null 2>&1
# move to firewall_start, minglin, 2010/01/08
#iptables -t filter -A FORWARD -i $1 -o ! $1 -j port_forward > /dev/null 2>&1
#move to firewall_start, minglin, 2010/01/08

miniupnpd -f /var/miniupnpd.conf &

#!/bin/sh
#
# $Id: firewall.sh,v 1.1 2007-09-26 02:25:17 winfred Exp $
#
# usage: firewall.sh init|fini
#

IPPORT_FILTER_CHAIN="ipport_block"
MAC_FILTER_CHAIN="mac_block"
DMZ_CHAIN="DMZ"
PORT_FORWARD_CHAIN="port_forward"


usage()
{
	echo "Usage:"
	echo "  $0 init|fini"
	exit 1
}

iptablesAllFilterClear()
{
	iptables -F -t filter 1>/dev/null 2>&1

	iptables -D FORWARD -j $IPPORT_FILTER_CHAIN 1>/dev/null 2>&1
	iptables -F $IPPORT_FILTER_CHAIN 1>/dev/null 2>&1

	iptables -D FORWARD -j $MAC_FILTER_CHAIN 1>/dev/null 2>&1
	iptables -F $MAC_FILTER_CHAIN 1>/dev/null 2>&1

	iptables -P INPUT ACCEPT
	iptables -P OUTPUT ACCEPT
	iptables -P FORWARD ACCEPT
}

init()
{
	iptablesAllFilterClear

	iptables -t filter -N $IPPORT_FILTER_CHAIN 1>/dev/null 2>&1
	iptables -t filter -N $MAC_FILTER_CHAIN 1>/dev/null 2>&1
	iptables -t filter -A FORWARD -j $MAC_FILTER_CHAIN 1>/dev/null 2>&1
	iptables -t filter -A FORWARD -j $IPPORT_FILTER_CHAIN 1>/dev/null 2>&1

	#iptablesAllFilterRun();

	# init NAT(DMZ)
	# We use -I instead of -A here to prevent from default MASQUERADE NAT rule being in front of us.
	# So "port forward chain" has the highest priority in the system, and "DMZ chain" is the second.
	iptables -t nat -D PREROUTING -j $PORT_FORWARD_CHAIN 1>/dev/null 2>&1
	iptables -t nat -F $PORT_FORWARD_CHAIN 1>/dev/null 2>&1
	iptables -t nat -X $PORT_FORWARD_CHAIN 1>/dev/null 2>&1

	iptables -t nat -N $PORT_FORWARD_CHAIN 1>/dev/null 2>&1
	iptables -t nat -I PREROUTING 1 -j $PORT_FORWARD_CHAIN 1>/dev/null 2>&1
	iptables -t nat -N $DMZ_CHAIN 1>/dev/null 2>&1
	iptables -t nat -I PREROUTING 2 -j $DMZ_CHAIN 1>/dev/null 2>&1

	#iptablesAllNATRun();
}

fini()
{
}

if [ "$1" = "" ]; then
	echo "$0: insufficient arguments"
	usage $0
elif [ "$1" = "init" ]; then
	init
elif [ "$1" = "fini" ]; then
	fini
else
	echo "$0: unknown argument: $1"
	usage $0
fi


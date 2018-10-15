#!/bin/sh
#
# $Id: config-pppoe.sh,v 1.4 2008-07-04 12:39:13 chhung Exp $
#
# usage: config-pppoe.sh <user> <password> <wan_if_name>
#

usage()
{
	echo "Usage:"
	echo "  $0 <user> <password> <wan_if_name>"
	exit 1
}

if [ "$3" = "" ]; then
	echo "$0: insufficient arguments"
	usage $0
fi


#syslogd -m 0
pppoe.sh $1 $2 $3 $4 $5
pppd file /etc/options.pppoe &
#iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu



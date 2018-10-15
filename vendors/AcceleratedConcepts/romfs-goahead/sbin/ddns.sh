#!/bin/sh
#
# $Id: ddns.sh,v 1.1 2007-09-24 09:34:52 winfred Exp $
#
# usage: ddns.sh
#

srv=`flashconfig.sh get DDNSProvider`
ddns=`flashconfig.sh get DDNS`
u=`flashconfig.sh get DDNSAccount`
pw=`flashconfig.sh get DDNSPassword`


killall -q inadyn

if [ "$srv" = "" -o "$srv" = "none" ]; then
	exit 0
fi
if [ "$ddns" = "" -o "$u" = "" -o "$pw" = "" ]; then
	exit 0
fi

# debug
echo "srv=$srv"
echo "ddns=$ddns"
echo "u=$u"
echo "pw=$pw"


if [ "$srv" = "dyndns.org" ]; then
	inadyn -u $u -p $pw -a $ddns --dyndns_system dyndns@$srv &
elif [ "$srv" = "freedns.afraid.org" ]; then
	inadyn -u $u -p $pw -a $ddns --dyndns_system default@$srv &
elif [ "$srv" = "zoneedit.com" ]; then
	inadyn -u $u -p $pw -a $ddns --dyndns_system default@$srv &
elif [ "$srv" = "no-ip.com" ]; then
	inadyn -u $u -p $pw -a $ddns --dyndns_system default@$srv &
else
	echo "$0: unknown DDNS provider: $srv"
	exit 1
fi


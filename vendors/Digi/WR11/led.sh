#!/bin/sh
#
# take care of 6300-CX/6300-LX specific LED setting
#
##############################################################
# allow script override
[ -x /etc/config/led.sh ] && exec /etc/config/led.sh "$@"
##############################################################
#exec 2>> /tmp/led.log
#set -x

usage()
{
	[ "$1" ] && echo "$1"
	echo "usage: $0 <interface_lan|interface_modem|signal|rat> <setup|teardown|up|down>"
	exit 1
}

##############################################################

eth_led()
{
	LINK=$(ip link show eth0 2> /dev/null | grep -c "state UP")
	if [ $LINK = 1 ] ; then
		ledcmd -o ETH
	else
		ledcmd -O ETH
	fi
}

modem_led()
{
	STATE="$1"
	[ "$STATE" = "up" ] && ledcmd -o ONLINE
	[ "$STATE" = "setup" ] && ledcmd -f ONLINE
	[ "$STATE" = "down" ] && ledcmd -O ONLINE
	[ "$STATE" = "teardown" ] && ledcmd -O ONLINE
}

##############################################################

[ $# -ne 2 ] && usage "Wrong number of arguments"

CMD="$1"
shift

case "$CMD" in
*lan*)             eth_led "$@" ;;
*interface_modem*) modem_led "$@" ;;
*)                 exit 1 ;;
esac

exit 0

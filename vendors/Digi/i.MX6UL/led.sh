#!/bin/sh
#
# take care of LED setting
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
}

modem_led()
{
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

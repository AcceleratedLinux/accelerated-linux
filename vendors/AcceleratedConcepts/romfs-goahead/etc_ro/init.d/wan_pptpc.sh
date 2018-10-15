#!/bin/sh
#PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
RC_CONF=/var/run/rc.conf
IN_CONF=/etc/init.d/load_balance.in
SCRIPT=$0
echo "">/dev/null

# include the enviro's
. $RC_CONF
. $IN_CONF
. /sbin/global.sh

#PPPD_BIN=/usr/sbin/pppd
#PPTP_PROFILE=/etc/ppp/peers/pptp
#PPTP_CHAP=/etc/ppp/chap-secrets
#NAT_SHELL=/etc/init.d/nat.sh
DHCP_SHELL=/etc/init.d/wan_udhcpc.sh
STATICIP_SHELL=/etc/init.d/static_ip.sh
#RESOLV_CONF=/var/tmp/resolv.conf
#PPP_RESOLV_CONF=/etc/ppp/resolv.conf
#PPTP_TMP=/tmp/tmp.root
#MAX_TIME=10
#SLEEP_TIME=1
#COUNT=1
#PUNIT=0
dual_wan_start_detect_network=/var/run/start_detect
time_out=15

# routines ##########################################################


start () {
	[ ! -z "`ps | grep pptp | grep pppd | cut -d'r' -f1`" ] && return 1   # already running?

	server=`flashconfig.sh get wan_pptp_server`
	user=`flashconfig.sh get wan_pptp_user`
	password=`flashconfig.sh get wan_pptp_pass`
	mode=`flashconfig.sh get wan_pptp_mode`
	pptp_opmode=`flashconfig.sh get wan_pptp_opmode`
	pptp_optime=`flashconfig.sh get wan_pptp_optime`
	echo "mode=$mode (flashconfig.sh get wan_pptp_mode)"
	if [ "$mode" = "0" ]; then
		ipaddr=`flashconfig.sh get wan_pptp_ip`
		netmask=`flashconfig.sh get wan_pptp_netmask`
		gateway=`flashconfig.sh get wan_pptp_gateway`
		if [ "$gateway" = "" ]; then
			gateway="0.0.0.0"
		fi
		echo "config-pptp.sh static $wan_if $ipaddr $netmask $gateway $server $user $password $pptp_opmode $pptp_optime"
		config-pptp.sh static $wan_if $ipaddr $netmask $gateway $server $user $password $pptp_opmode $pptp_optime
	else
		echo "config-pptp.sh dhcp $wan_if $server $user $password $pptp_opmode $pptp_optime"
		config-pptp.sh dhcp $wan_if $server $user $password $pptp_opmode $pptp_optime
		if [ $? = 88 ]; then
                    return 0
                fi
	fi

        run_interface
	echo "james: the result of run_interface() is $run_if"
	waitting_interface $run_if $time_out 
	if [ $? = 0 ]; then
		[ "$wan_dual_wan_backup" = "99" ] && return 0
		echo "james: step into stop when doing pptp connection"
		stop
		return 0
	fi

	# Jason 2009728 for dual-wan detect netwrok
	[ "$wan_dual_wan_backup" != "99" ] && touch $dual_wan_start_detect_network > /dev/null
	return 0
}

stop () {
	local err; err=0

	#pid=`ps | grep pppd | grep options.pptp | cut -d'r' -f1`
	#[ ! -z "$pid" ] && ( kill -9 $pid || err=1)
	#for pid in `cat $PID_FILE`; do
	#	[ ! -z "$pid" ] && ( kill -9 $pid || err=1)
	#	break
	#done
	for pid in `pidof pppd`; do
        	if [ ! -z "$pid" ]; then
                	found=`ps | grep $pid | grep "pptp"`
                	if [ ! -z "$found" ]; then
             			kill -TERM $pid || err=1
			fi
		fi
	done

	mode=`flashconfig.sh get wan_pptp_mode`
	if [ "$mode" = "0" ]; then 
		$STATICIP_SHELL stop	
	else
		$DHCP_SHELL stop	
	fi
	# Jason 20090728 for dual-wan detect netwrok
	[ "$wan_dual_wan_backup" != "99" ] && rm -f $dual_wan_start_detect_network > /dev/null

	return $err
}

usage () {
        echo "$0 [start|stop|restart]"
        exit 1
}

# main ##########################################################

[ -z "$1" ] && usage;

err=0

for action in $*; do
	case $action in
		start)		start;;
		stop)		stop;;
		restart)	stop; sleep 1; start;;
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

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

#PPPOE_BIN=/usr/sbin/pppoecd
#PPPD_PIDFILE=/var/run/ppp0.pid
#PPPOEIDLE=$wan_pppoe_maxidletime
time_out=15
#PUNIT=0
dual_wan_start_detect_network=/var/run/start_detect
WANIF="${eth2}.2"

# routines ##########################################################

start () {
	#pidof pppd>/dev/null && return 1
	[ ! -z "`ps | grep pppoe | grep pppd | cut -d'r' -f1`" ] && return 1   # already running?
	ifconfig $WANIF 0.0.0.0

	#       Begin: added by carl to fixed SIT bug 010
	#       pppoed_watch.sh&
	#       End: added by carl to fixed SIT bug 010
	#       Begin: removed by carl to fixed SIT bug 010
	user=`flashconfig.sh get wan_pppoe_user`
	password=`flashconfig.sh get wan_pppoe_pass`
	pppoe_opmode=`flashconfig.sh get wan_pppoe_opmode`
	#pri_dns=`flashconfig.sh get wan_primary_dns`
	#sec_dns=`flashconfig.sh get wan_secondary_dns`
	# use pppd instead of pppoecd, because pppd works well.
	if [ "$pppoe_opmode" = "" ]; then
		#               echo "pppoecd $wan_if -u $u -p $pw"
		#               pppoecd $wan_if -u "$u" -p "$pw"
		pppoe_opmode = "KeepAlive"
		pppoe_optime = "60"
		config-pppoe.sh $user $password $wan_if $pppoe_opmode $pppoe_optime
	else
		pppoe_optime=`flashconfig.sh get wan_pppoe_optime`
		config-pppoe.sh $user $password $wan_if $pppoe_opmode $pppoe_optime
	fi
	#       End: removed by carl to fixed SIT bug 010

	run_interface
	waitting_interface $run_if $time_out 
	if [ $? = 0 ]; then
		[ "$wan_dual_wan_backup" = "99" ] && return 0
		stop
		return 0
	fi

	# Jason 2009728 for dual-wan detect netwrok
	[ "$wan_dual_wan_backup" != "99" ] && touch $dual_wan_start_detect_network > /dev/null
	return 0

	#/flash/firmware/fw_setenv wan_pppoe_username $wan_pppoe_username > /dev/null
	#/flash/firmware/fw_setenv wan_pppoe_password $wan_pppoe_password > /dev/null	
}

stop () {
	local pid
	local err; err=0

	#if [ -f $PPPD_PIDFILE ] ; then
	#	kill -9 `cat $PPPD_PIDFILE` || err=1
	#	[ -f $PPPD_PIDFILE ] && (rm $PPPD_PIDFILE || err=1)
	#fi
	#pid=`ps | grep pppoe | grep pppd | cut -d'r' -f1`
	#[ ! -z "$pid" ] && ( kill -9 $pid || err=1)
	for pid in `pidof pppd`; do
        	if [ ! -z "$pid" ]; then
                	found=`ps | grep $pid | grep "pppoe"`
                	if [ ! -z "$found" ]; then
             			kill -TERM $pid || err=1
			fi
		fi
	done

	# Jason 20081112 for dual-wan detect netwrok
	[ "$wan_dual_wan_backup" != "99" ] && rm -f $dual_wan_start_detect_network > /dev/null

	return $err
}

usage () {
        echo "$0 [start|stop|restart|reload|config|manualstart]"
        exit 1
}

# main ##########################################################

[ -z "$1" ] && usage;

err=0

for action in $*; do
	case $action in
		config)		;;
		start)		start;;
		stop)		stop;;
		reload)		;;
		restart)	stop; sleep 1; start;;
		manualstart)  stop; sleep 1; start;;
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

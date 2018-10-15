#!/bin/sh
#PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
RC_CONF=/var/run/rc.conf
SCRIPT=$0
echo "">/dev/null

# include the enviro's
. $RC_CONF
. /sbin/global.sh

UDHCPC_BIN=/sbin/udhcpc
UDHCPC_PID_FILE=/var/run/udhcpc.pid
UDHCPC_LEASE_FILE=/var/udhcpc.leases
dual_wan_start_detect_network=/var/run/start_detect

#dhcp_host=`flashconfig.sh get wan_dhcp_host`
# get host name by flash.
dhcp_host=`flashconfig.sh get HostName`
# routines ##########################################################

start () {
	# we don't check if old udhcpc is gone or not as the old udhcpc may be in dying stage
	# (stopping by wan.sh stop) which should not stop us from running new udhcpc
	# pidof udhcpc>/dev/null && return 1	# already running?
	
	/sbin/ifconfig $wan_if 0.0.0.0

	if  [ "$dhcp_host" = "" ]; then
		udhcpc -i $wan_if -s /sbin/udhcpc.sh -p $UDHCPC_PID_FILE &
	else
		udhcpc -i $wan_if -H $dhcp_host -s /sbin/udhcpc.sh -p $UDHCPC_PID_FILE &
	fi
}

renew() {
	local err; err=0
	if [ -f $UDHCPC_PID_FILE ]; then
		/bin/kill -SIGUSR1 `cat $UDHCPC_PID_FILE` || err=1
	fi
	sleep 3
	return $err
}

release() {
	local err; err=0
	if [ -f $UDHCPC_PID_FILE ]; then
		/bin/kill -SIGUSR2 `cat $UDHCPC_PID_FILE` || err=1
	fi
	return $err
}

stop () {
	local err; err=0
	local pid i

	if [ -f $UDHCPC_PID_FILE ]; then
		kill -TERM `cat $UDHCPC_PID_FILE` || err=1
		[ -f $UDHCPC_PID_FILE ] && (rm -f $UDHCPC_PID_FILE || err=1)
	fi

	for i in 1 2 3 4 5; do
		pid=`pidof udhcpc`
		[ -z "$pid" ] && break;
		kill -TERM $pid || err=1
		sleep 1;
	done

	[ -f $UDHCPC_LEASE_FILE ] && (rm -f $UDHCPC_LEASE_FILE || err=1)
	#james: for switching single/dual wan configuration from web ui directly
	[ ! -e /var/run/stop.run ] && [ -f /etc/resolv.conf ] && (rm -f /etc/resolv.conf || err=1)

	# we don't use ifconfig down here to avoid $WANIF:0 be shutdown unexpectedly
	ifconfig $wan_if 0.0.0.0 || err=1
	# Jason 20081112 for dual-wan detect netwrok
	[ "$wan_dual_wan_backup" != "99" ] && rm -f $dual_wan_start_detect_network > /dev/null
	return $err
}

usage () {
        echo "$0 [start|stop|restart|reload|config|release|renew]"
        exit 1
}

# main ##########################################################

[ -z "$1" ] && usage;

err=0

for action in $*; do
	case $action in
		config)		;;
		release)	release;;
		renew)		renew;;
		start)		start;;
		stop)		stop;;
		reload)		;;
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

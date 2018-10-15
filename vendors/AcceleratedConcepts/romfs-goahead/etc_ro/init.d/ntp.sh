#!/bin/sh
#Tom rewrite ntp.sh for 3G function 04-23-2009 
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
SCRIPT=$0
echo "">/dev/null
. /sbin/global.sh

CROND_LOG_LEVEL=5

#temporarily settings
datetime_ntp_enable=1
datetime_ntp_updateinterval=24

# Currently igmp is always disabled.

smx_time_restart() {
	killall -q igmpproxy
	/sbin/smx-time.sh /etc_ro/init.d/ntp.sh restart
	date +"%m%d%H%M%Y.%S" > /etc/ntp.date
	sync
	# igmpproxy
	igmp=`flashconfig.sh get igmpEnabled`
	if [ "$igmp" = "1" ]; then
		config-igmpproxy.sh $wan_if $lan_if
	fi
	# fix NTP time offset for PPPD
	# if NTP time synchronization  isn't fixed by 3G subsystem, 
	# touch a file for PPPD in case 3G interface is used later
	if [ "$wan_ip_assignment" != "3" ];then
		echo "1" > /var/run/ppp-ntp-updated
	fi
}

config() {
	if [ "$datetime_ntp_enable" = "1" ] ; then
		smx_time_restart
	else
		return 0
	fi
}

crond_restart() {
	/bin/kill -9 `/bin/pidof crond`
	echo "0 */"$datetime_ntp_updateinterval" * * *     /etc_ro/init.d/ntp.sh config > /dev/null" > $CROND_FILE
	/sbin/crond -c /var/spool/cron/crontabs -l $CROND_LOG_LEVEL
	echo '>>> /etc_ro/init.d/ntp.sh restarted crond!'
}

# Goahead will call this, but we can ignore it because goahead is
# also going to call date.sh where we will handle things.

smx_time_tzonly() {
#	/sbin/smx-time.sh /etc_ro/init.d/ntp.sh tzonly
	return 0
}

usage () {
	echo "$0 [config]"
	exit 1
}

# main ##########################################################

[ -z "$1" ] && usage;

err=0

for action in $*; do
	case $action in
		start)		start;;
		config)		config;;
		force)		crond_restart; smx_time_restart;;
		crond_restart)  crond_restart;;
		ntp_update)	smx_time_restart;;
		tz_update)	smx_time_tzonly;;
		*)		usage;;
	esac
	if [ $? = "0" ] ; then
		echo ">>> $SCRIPT $action ok"
	else
		echo ">>> $SCRIPT $action error"
		err=1
	fi
done

exit $err


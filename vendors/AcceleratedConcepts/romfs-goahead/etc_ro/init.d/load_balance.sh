#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
RC_CONF=/var/run/rc.conf
IN_CONF=/etc/init.d/load_balance.in

SCRIPT=$0

# include the enviro's
. $RC_CONF
. $IN_CONF

TMP_CONF=/tmp/tmp.conf
WAN_SHELL=/sbin/wan_dual.sh
CRON_SHELL=/etc/init.d/cron.sh
RUNNING=/var/run/load_balance.running
STOP=/tmp/load_balance.stop
STATUS_FILE=/var/run/lb.status
SCRIPT_START=/var/run/wan.start
MAX_COUNT_WAN=3
MAX_COUNT_3G=40
count=0
maxcount=5
master_check=3
LOGGER_BIN=/usr/bin/logger
LOGGER_MSG="Process Load Balance[`pidof load_balance.sh`]:"
LOG_INFO=6
interface_counter=0
dual_wan_start_detect_network=/var/run/start_detect

update_rc_conf() {
echo "sed -e 's/'$1'='$2'/'$1'='$3'/g' $RC_CONF > $TMP_CONF"
sed -e 's/'$1'='$2'/'$1'='$3'/g' $RC_CONF > $TMP_CONF
# Using semafore to control critical section
wait_count=1
while [ -f /var/run/rc_conf_prot ]
do
   sleep 1
   if [ $wait_count -gt 10 ]; then 
       break
   else
       wait_count=`expr $wait_count + 1`
   fi
done
if [ ! -f /var/run/rc_conf_prot ];then
    touch /var/run/rc_conf_prot
    mv -f $TMP_CONF $RC_CONF
    . $RC_CONF
    rm -f /var/run/rc_conf_prot
else
    mv -f $TMP_CONF $RC_CONF
    . $RC_CONF
fi
}

stop_run_file () {
case $1 in
	0)
		rm -f /var/run/stop.run
		;;
	1)
		touch /var/run/stop.run
		;;
esac
sync
}
check_alive () {
if [ "$wan_dual_wan_fallback_enable" = "1" ]; then
	stop_load_balance
	[ "$?" = "1" ] && exit 0
	update_rc_conf "wan_ip_assignment" $wan_ip_assignment $wan_dual_wan_master
	stop_run_file 1
	rm -Rf  $SCRIPT_START 2>/dev/null
	rm -f $dual_wan_start_detect_network 2>/dev/null
	$WAN_SHELL lb_start $wan_dual_wan_master
	check_interface $wan_dual_wan_backup
	stop_run_file 0
	get_count ""$wan_dual_wan_master" = "3" -o "$wan_dual_wan_master" = "7""
	##DWAN_DEBUG echo "---$wan_dual_wan_master:$master_if:$max_count----"
	connection_process $wan_dual_wan_master $master_if $max_count
	if [ "$?" = "0" ]; then
		stop_run_file 1
		$WAN_SHELL lb_stop $wan_dual_wan_master
		mkdir $SCRIPT_START 2>/dev/null
		stop_run_file 0
		update_rc_conf "wan_ip_assignment" $wan_ip_assignment $wan_dual_wan_backup
		touch $dual_wan_start_detect_network 2>/dev/null
	else
		$WAN_SHELL lb_stop $wan_dual_wan_master
		mkdir $SCRIPT_START 2>/dev/null
		update_rc_conf "wan_ip_assignment" $wan_ip_assignment $wan_dual_wan_backup
		touch $dual_wan_start_detect_network 2>/dev/null
		stop_load_balance
		[ "$?" = "1" ] && exit 0
		change_int
		return 0
	fi 	
	stop_load_balance
	[ "$?" = "1" ] && exit 0
fi
}
Set_check_alive_Timer () {
	if [ $1 -ne 0 ]; then
		sleep $1 && kill -l ALRM $$ &
	fi
}
stop_load_balance () {
	if [ -e $STOP ]; then
		echo "$wan_ip_assignment"  > $STATUS_FILE
		rm -f $RUNNING $STOP $STATUS_FILE 
		$LOGGER_BIN -t $LOGGER_MSG -p $LOG_INFO "Unload load balance features successfully"
		$CRON_SHELL restart
		return 1
	else
		return 0
	fi
}
connection_process () {
	waitting_interface $2 $maxcount
	if [ "$?" = "1" ]; then 
		counting_connection $1 $2 $3
		[ "$?" = "1" ] && return 1
	fi
	return 0
}
get_count () {
	if [ $1 ]; then
		max_count=$MAX_COUNT_3G
	else
		max_count=$MAX_COUNT_WAN
	fi	
}
backup_mode () {
		stop_load_balance
		[ "$?" = "1" ] && exit 0
		active_interface $wan_ip_assignment
		get_count ""$wan_ip_assignment" = "3""
		##DWAN_DEBUG echo "===$wan_ip_assignment:$tmp_if:$max_count==="
		connection_process $wan_ip_assignment $tmp_if $max_count
		if [ "$?" = "0" ]; then
			# fix so sensitive network detecting
			if [ "$interface_counter" = "1" ]; then
				change_int
				[ "$?" = "0" ] && interface_counter=0
			else
				interface_counter=`expr $interface_counter + 1`
			fi
			return 0
		else
			interface_counter=0
		fi
		return 1
}
change_int() {
				stop_load_balance
				[ "$?" = "1" ] && exit 0
				if [ "$wan_ip_assignment" = "$wan_dual_wan_master" ]; then 	
	 				ip_assignment=$wan_dual_wan_backup
				else
	 				ip_assignment=$wan_dual_wan_master
				fi
				$LOGGER_BIN -t $LOGGER_MSG -p $LOG_INFO "Switchng WAN interface .........."
				#======== WAN.SH stop ========
				$WAN_SHELL lb_stop $wan_ip_assignment

				$LOGGER_BIN -t $LOGGER_MSG -p $LOG_INFO "Shutdown current WAN interface successfully"

				#======== Write meta file ====
				update_rc_conf "wan_ip_assignment" $wan_ip_assignment $ip_assignment
				echo "$wan_ip_assignment"  > $STATUS_FILE

				$LOGGER_BIN -t $LOGGER_MSG -p $LOG_INFO "Start WAN interface successfully"
				#======== WAN.SH start =======
				[ $wan_dual_wan_backup -eq 3 ] && sleep 3
				$WAN_SHELL lb_start $wan_ip_assignment

				#backup_mode
				active_interface $wan_ip_assignment
				get_count ""$wan_ip_assignment" = "3""
				##DWAN_DEBUG echo "===$wan_ip_assignment:$tmp_if:$max_count==="
				connection_process $wan_ip_assignment $tmp_if $max_count
				if [ "$?" = "1" ]; then 
						$LOGGER_BIN -t $LOGGER_MSG -p $LOG_INFO "WAN interface connected"
						#minglin, 2009/12/17, in Dual_Wan mode fix firewell start
						goahead fw_finit 
						goahead fw_init
						#minglin, 2009/12/17, in Dual_Wan mode fix firewell end
						# minglin, 2010/01/12, fix restart miniupnp... issue
						lan.sh POST
						# minglin, 2010/01/12, fix restart miniupnp... issue
						return 0
				else
						return 1
				fi
				# original code
				#stop_load_balance
				#[ "$?" = "1" ] && exit 0
				#$LOGGER_BIN -t $LOGGER_MSG -p $LOG_INFO "WAN interface disconnected"
				#change_int
}
Set_backup_mode_Timer () {
	if [ $1 -ne 0 ]; then
		sleep $1 && kill -l ALRM $$ &
	fi
}

start () {
	check_backup_mode
	if [ "$?" = "1" ]; then
		if [ -f $RUNNING ]; then
			$LOGGER_BIN -t $LOGGER_MSG -p $LOG_INFO "Load balance backup features is running already"
			return 0
		else 
			$LOGGER_BIN -t $LOGGER_MSG -p $LOG_INFO "Load balance backup features start successfully"
			touch $RUNNING
		fi
		$CRON_SHELL restart 2
		echo "$wan_ip_assignment"  > $STATUS_FILE
		#trap check_alive ALRM
		while :
		do
			stop_load_balance
			[ "$?" = "1" ] && exit 0
		  backup_mode
			if [  "$?" = "1" -a "$wan_dual_wan_fallback_enable" = "1" ]; then
				if [ ! $wan_ip_assignment = $wan_dual_wan_master ]; then
					#Set_check_alive_Timer $master_check
					#wait $!
					sleep $master_check
					check_alive
				fi
			fi		
		done
 	else
		$LOGGER_BIN -t $LOGGER_MSG -p $LOG_INFO "It is not load balance backup mode. exiting ........."
	fi
	return 0
}
stop () {
	if [ -f $RUNNING ]; then
		touch $STOP
		$LOGGER_BIN -t $LOGGER_MSG -p $LOG_INFO "Send stop message successfully"
	else
		$LOGGER_BIN -t $LOGGER_MSG -p $LOG_INFO "Load balance not run"
	fi
	return 0
}

usage () {
        echo "$0 [start|stop]"
        exit 1
}

# main ##########################################################

[ -z "$1" ] && usage;

err=0

for action in $*; do
	case $action in
		start)		start;;
		stop)		stop;;
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

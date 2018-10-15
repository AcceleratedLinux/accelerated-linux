#!/bin/sh
SCRIPT=$0
echo "">/dev/null

CRON_BIN=/usr/sbin/crond
CRON_FILE=/var/spool/cron/crontabs/admin
GAP_SCRIPT=/sbin/greenap.sh

staqueryint=$(flashconfig.sh get StaQueryInt)

CRON_0="0 1 * * * /sbin/smx-query.sh > /dev/null"
CRON_1="*/10 * * * * /etc_ro/init.d/statusquery.sh > /dev/console"
CRON_1_OLD="0,32 * * * * /etc/init.d/ntp.sh config > /dev/null"
CRON_2="*/1 * * * * /etc/init.d/3g.sh cron_3g_restart > /dev/null"
CRON_3="*/1 * * * * /etc/init.d/3g.sh cron_3g_budget_control > /dev/null"
if [ "$staqueryint" != '' ]; then
  if [ "$staqueryint" -gt '60' ]; then
    staqueryint=$((staqueryint/60))
    CRON_4="* */$staqueryint * * * /sbin/sta-query.sh > /dev/console"
  else
    CRON_4="*/$staqueryint * * * * /sbin/sta-query.sh > /dev/console"
  fi
fi

# routines ##########################################################
# Jason, 20090726
greenap_add () {
	cronebl="0"

	action=`flashconfig.sh get GreenAPAction1`
	if [ "$action" != "Disable" -a "$action" != "" ]; then
		cronebl="1"
	fi

	action=`flashconfig.sh get GreenAPAction2`
	if [ "$action" != "Disable" -a "$action" != "" ]; then
		cronebl="1"
	fi

	action=`flashconfig.sh get GreenAPAction3`
	if [ "$action" != "Disable" -a "$action" != "" ]; then
		cronebl="1"
	fi

	action=`flashconfig.sh get GreenAPAction4`
	if [ "$action" != "Disable" -a "$action" != "" ]; then
		cronebl="1"
	fi

	if [ "$cronebl" = "1" ]; then
		$GAP_SCRIPT setchk
		#crond
	fi
}

config() {
		wan_bridge_enable=`flashconfig.sh get OperationMode`
		wan_connect_type=`flashconfig.sh get wanConnectionMode`
		wan1_type=`flashconfig.sh get wan1`
		wan2_type=`flashconfig.sh get wan2`
		wan_3gbudget_control=`flashconfig.sh get wan_3g_budget_control_enable`  
		#modify by daniel hung 20091019 ,for bridge mode can use ntp
		echo "$CRON_0"  > $CRON_FILE
		echo "$CRON_1" >> $CRON_FILE
		echo "$CRON_4" >> $CRON_FILE
#		echo "$CRON_1_OLD" >> $CRON_FILE
		if [ "$wan_bridge_enable" = "1" ]; then
	        #modify by daniel hung 20091019, only 3G use it.
	        if [ "$wan2_type" = "G3G" -a "$wan1_type" = "99" ]; then
	        	echo "$CRON_2" >> $CRON_FILE
	        fi
	        
	        if [ "$wan2_type" = "G3G" -a "$wan_3gbudget_control" = "1" ]; then
	        	echo "$CRON_3" >> $CRON_FILE
	        fi
	        	
	        #local i=0
	        #rm -rf $CRON_FILE
	        #for line in \
	        #        "$CRON_1_OLD" "$CRON_2" "$CRON_3" "$CRON_4" "$CRON_5" \
	        #        "$CRON_6" "$CRON_7" "$CRON_8" "$CRON_9" "$CRON_10" ; do
	        #        [ -z "$line" ] && break
	        #        i=$(( $i + 1 ))
	        #        if [ "$i" = "$1" ]; then
	        #                continue
	        #        else
	        #                echo "$line" >> $CRON_FILE
	        #        fi
	        #done
                fi
# Jason , 200907226
	greenap_add 
		
}

start () {
	config $1
	pid=`pidof crond`
        [ ! -z $pid ] && stop
	[ ! -e $CRON_FILE ] && config $1 
        $CRON_BIN -c /var/spool/cron/crontabs -l 5 > /dev/null 
	[ $? = 0 ] && return 0
        return 1
}

stop () {
	pid=`pidof crond`
        [ -z $pid ] && return 0
        kill -9 $pid
	[ $? = 0 ] && return 0
        return 1
}

usage () {
        echo "$0 [start|stop|restart|config]"
        exit 1
}

# main ##########################################################

[ -z "$1" ] && usage;

err=0

for action in $1; do
	case $action in
		config)		config $2;;
		start)		start $2;;
		stop)		stop;;
		restart)	stop; start $2;;
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

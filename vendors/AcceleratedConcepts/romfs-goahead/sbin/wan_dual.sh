#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin

IN_CONF=/etc/init.d/load_balance.in
RC_CONF=/var/run/rc.conf

SCRIPT=$0
echo "">/dev/null

# include the enviro's
. $RC_CONF
. $IN_CONF
. /sbin/global.sh

SCRIPT_RUNNING=/var/run/wan.running
SCRIPT_START=/var/run/wan.start
#SCRIPT_CRON=/etc/init.d/cron.sh
bypass=0
##wan_ip_assignment static:0, dhcp 1......
type=$wan_ip_assignment
STOP=/tmp/load_balance.stop
run_lb=0
bridge_mode=0
#---JC Note:
# wan_bridge_enable: 0- bridge  1- Router
# bridge_mode: 0- Router  1- bridge
bridge_mode=$wan_bridge_enable

dual_wan_start_detect_network=/var/run/start_detect
#JCYU-add for debug dual-wan start/stop
wan_dual_arg=$*
# routines #################################################

wan_static_ip (){

 if [ "$1" = "start" ]; then
	#lan and wan ip should not be the same except in bridge mode
	if [ "$wan_bridge_enable" != "1" ]; then
		if [ "$wan_ipaddr" = "$lan_ip" ]; then
			echo "wan.sh: warning: WAN's IP address is set identical to LAN"
			return 1
		fi
	fi
	[ ! "`ifconfig $wan_if 2>&1 >/dev/null`" ] && ifconfig $wan_if $wan_ipaddr netmask $wan_netmask
	route del default >/dev/null 2>&1
	if [ "$wan_gateway" != "" ]; then
	route add default gw $wan_gateway >/dev/null 2>&1
	fi
	config-dns.sh $pd $sd
	[ "$wan_dual_wan_backup" != "99" ] && touch $dual_wan_start_detect_network > /dev/null
 fi
 if [ "$1" = "stop" ]; then
  ifconfig $wan_if 0.0.0.0
  route del default gw $wan_gateway
 fi
}

ioctl_process() {
	if [ $bridge_mode = 0 ]; then
        	check_backup_mode
               	if [ $? = 1 ]; then
                       	if [ "$bypass" = "0" ]; then
                               	/sbin/ioctl $dev_wan_interface 3 35 0
				run_lb=1
                       	fi
               	else
               		if [ "$type" = "3" -o "$type" = "7" -o "$wan_dual_wan_backup" != "99"  ]; then
               			/sbin/ioctl $dev_wan_interface 3 35 0
			else
				/sbin/ioctl $dev_wan_interface 3 35 1
			fi
               	fi
	fi
}
run_process() {
   case $1 in
			0)	/etc/init.d/static_ip.sh $2;;
			1)	/etc/init.d/wan_udhcpc.sh $2;;
			2)	/etc/init.d/wan_pppoe.sh $2;;
			3)	rm -rf /var/3gs/*;rm -rf /var/wi3g/*;
				#kent_chang 20091208, use GPIO_11 to turn off USB Power and then turn on USB Power
                                gpio l 11 0 4000 0 1 4000; sleep 1; gpio l 11 4000 0 1 0 4000; sleep 5;
				/etc/init.d/3g.sh $2;;
			4)	/etc/init.d/wan_pptpc.sh $2;;
			5)	/etc/init.d/wan_l2tpc.sh $2;;
			6)	/etc/init.d/bigpond.sh $2;;
			7)	/etc/init.d/wifiwan.sh $2;;
			*)	/etc/init.d/staticip.sh $2;;
		esac
}

start() {
	local err; err=0

#	if [ -f /var/run/provision.start ]; then	## means start update firmware
#		return $err	
#	fi

	if mkdir $SCRIPT_START 2>/dev/null; then
		#/etc/init.d/resolv.sh config
		#ioctl_process
		if [ $bridge_mode = 0 ]; then
		   check_backup_mode
       if [ $? = 1 ]; then
          if [ "$bypass" = "0" ]; then
             run_lb=1
          fi
       fi
    fi


               #james add, !!!I'm not sure any side effect introduced!!!
                #echo "==james: james add, !!!I'm not sure any side effect introduced!!!"
                #echo "$wan_ip_assignment"  > $STATUS_FILE


		#case $wan_ip_assignment in
		run_process $type "start"
		
		# minglin, 2010/01/12
		check_backup_mode
		[ $? = 0 -o $run_lb = 1 ] && lan.sh POST
		# minglin, 2010/01/12
		
		nat.sh
	
		# reset hostname and resolv.conf to values in meta file
		# /etc/init.d/hostname.sh config
		mkdir -p /var/spool/cron/crontabs/

    if [ $bridge_mode = 0 -a $run_lb = 1 ]; then
 			 /etc/init.d/load_balance.sh start&
		#else 
		#	if [ $bridge_mode = 0 ]; then
		#		check_backup_mode
		#		if [ $? = 0 -a $type = 3 ]; then 
		#			$SCRIPT_CRON restart
		#		else
		#			$SCRIPT_CRON restart 2
		#		fi
		#	fi
		fi

	else
		##DWAN_DEBUG echo "$SCRIPT is already running?"
		err=1
	fi

	return $err
}

stop () {
	local err; err=0
	#if [ -f /var/run/provision.start ]; then	## means start update firmware
	#	return $err	
	#fi

	if rm -R $SCRIPT_START 2>/dev/null; then
		if [ "$bridge_mode" = "0" ]; then
       			check_backup_mode
       			[ $? = 1 ] && stop_lb_process $bypass $STOP
		fi
		run_process $type "stop"
		#case $wan_ip_assignment in
		# reset hostname and resolv.conf to values in meta file
		#/etc/init.d/hostname.sh config
		#/etc/init.d/resolv.sh config

		# clean net info, used by udhcpc.sh
		#rm -f /var/run/net.now /var/run/net.old
	else
		echo "$SCRIPT is not running, stop skipped."
	fi

	return $err
}

usage () {
        #echo "$0 [start|stop|restart|reload|lb_start|lb_stop|lb_restart]"
#JCYU-Add to re-start the load_balance.sh to prevent the memory leakage
        echo "$0 input:[$wan_dual_arg]"
        if [ "$bridge_mode" = "0" ]; then
        check_backup_mode 
        if [ "$?" = "1" -a "$1" = "lb_stop" ]; then
                /etc/init.d/load_balance_re.sh start&
            fi
        else
            /etc/init.d/load_balance_re.sh stop&  
       fi
#JCYU-Add:End
        #exit 1
}

# main ##########################################################

[ -z "$1" ] && usage && exit 1;

owner=0
err=0
if [ $bridge_mode = 0 ]; then
	check_backup_mode
	[ ! $? = 0 -a ! "X"$2 = "X" ] && type=$2
fi

if [ $bridge_mode = 0 ]; then
        echo $* > /tmp/arg
        check_backup_mode
        if [ "$?" = "1" ]; then
                for tmp in `cat /tmp/arg`; do
                        if [ "$tmp" = "stop" ]; then
                                stop_lb 60 $SCRIPT_RUNNING
				break
                        fi
                done
        fi
        rm -f /tmp/arg
fi

if mkdir $SCRIPT_RUNNING 2>/dev/null; then
	owner=1
fi

echo $* >> $SCRIPT_RUNNING/cmdQ


if [ "$owner" = "1" ]; then
	while mv $SCRIPT_RUNNING/cmdQ $SCRIPT_RUNNING/actQ 2>/dev/null; do
		for action in `cat $SCRIPT_RUNNING/actQ`; do
			case $action in
				config)		;;
				start)		start;;
				stop)		stop;;
				reload)		;;
				restart)	stop; start;;
        			lb_start)       bypass=1; start;;
        			lb_stop)        bypass=1; stop;;
        			lb_restart)     bypass=1; stop; start;;	
				*)		usage;;
			esac
			if [ $? = "0" ] ; then
				##DWAN_DEBUG echo $SCRIPT $action ok
                                echo "" > /dev/null
			else
				##DWAN_DEBUG echo $SCRIPT $action error
				err=1
			fi
		done
		rm $SCRIPT_RUNNING/actQ
	done
	rm -R $SCRIPT_RUNNING	
fi

exit $err

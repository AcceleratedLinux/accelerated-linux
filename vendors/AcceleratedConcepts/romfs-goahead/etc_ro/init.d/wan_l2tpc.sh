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

#PPPD_PIDFILE=/etc/ppp0.pid
#L2TP_PROFILE=/etc/xl2tpd/xl2tpd.conf
#L2TP_PROFILE_SAMPLE=/etc/xl2tpd/xl2tpd.conf.sample
#L2TP_OPTIONS=/etc/ppp/options.xl2tpd
#L2TP_OPTIONS_SAMPLE=/etc/ppp/options.xl2tpd.sample
#L2TP_CONTROL=/var/run/xl2tpd/l2tp-control
#L2TP_CONTROL_DIR=/var/run/xl2tpd
#PPTP_CHAP=/etc/ppp/chap-secrets
#L2TP_BIN=/usr/sbin/xl2tpd
#NAT_SHELL=/etc/init.d/nat.sh
DHCP_SHELL=/etc/init.d/wan_udhcpc.sh
STATICIP_SHELL=/etc/init.d/static_ip.sh
#RESOLV_CONF=/var/tmp/resolv.conf
#PPP_RESOLV_CONF=/etc/ppp/resolv.conf
#PPTP_TMP=/tmp/tmp.root
#MAX_TIME=10
#SLEEP_TIME=1
#COUNT=1
time_out=15
dual_wan_start_detect_network=/var/run/start_detect

# routines ##########################################################


start () {
	#[ ! -z "`ps | grep pptp | grep pppd | cut -d'r' -f1`" ] && return 1   # already running?
        #[ ! -d $L2TP_CONTROL_DIR ] && mkdir $L2TP_CONTROL_DIR
        #[ `pidof xl2tpd` ] && return 1

	server=`flashconfig.sh get wan_l2tp_server`
	user=`flashconfig.sh get wan_l2tp_user`
	password=`flashconfig.sh get wan_l2tp_pass`
	mode=`flashconfig.sh get wan_l2tp_mode`
	l2tp_opmode=`flashconfig.sh get wan_l2tp_opmode`
	l2tp_optime=`flashconfig.sh get wan_l2tp_optime`
	if [ "$mode" = "0" ]; then
		ipaddr=`flashconfig.sh get wan_l2tp_ip`
		netmask=`flashconfig.sh get wan_l2tp_netmask`
		gateway=`flashconfig.sh get wan_l2tp_gateway`
		if [ "$gateway" = "" ]; then
			gateway="0.0.0.0"
		fi
		config-l2tp.sh static $wan_if $ipaddr $netmask $gateway $server $user $password $l2tp_opmode $l2tp_optime
	else
		config-l2tp.sh dhcp $wan_if $server $user $password $l2tp_opmode $l2tp_optime
		if [ $? = 88 ]; then
                    return 0
                fi
	fi

        run_interface
	echo "james: the result of run_interface( ) is $run_if"
	waitting_interface $run_if $time_out 
	if [ $? = 0 ]; then
		[ "$wan_dual_wan_backup" = "99" ] && return 0
		stop
		return 0
	fi

	# Jason 2009728 for dual-wan detect netwrok
	[ "$wan_dual_wan_backup" != "99" ] && touch $dual_wan_start_detect_network > /dev/null
	return 0
}

stop () {
        local err; err=0

	#pid=`ps | grep pppd | grep l2tp | cut -d'r' -f1`
	#[ ! -z "$pid" ] && ( kill -9 $pid || err=1)
	for pid in `pidof pppd`; do
        	if [ ! -z "$pid" ]; then
                	found=`ps | grep $pid | grep "l2tp"`
                	if [ ! -z "$found" ]; then
             			kill -TERM $pid || err=1
			fi
		fi
	done

	pid=`pidof l2tpd`
	[ ! -z "$pid" ] && ( kill -9 $pid || err=1)
	
	mode=`flashconfig.sh get wan_l2tp_mode`
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
                start)          start;;
                stop)           stop;;
                restart)        stop; sleep 1; start;;
                *)              usage;;
        esac
        if [ $? = "0" ] ; then
                echo $SCRIPT $action ok
        else
                echo $SCRIPT $action error
                err=1
        fi
done

exit $err

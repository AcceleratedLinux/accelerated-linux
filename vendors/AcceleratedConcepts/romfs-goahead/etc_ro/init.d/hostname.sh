#!/bin/sh
#PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
RC_CONF=/var/run/rc.conf
IN_CONF=/etc/init.d/load_balance.in
SCRIPT=$0
echo "">/dev/null

# include the enviro's
. $RC_CONF
. $IN_CONF

HOSTNAME_BIN=hostname
HOSTS_FILE=/etc/hosts
NSLOOKUP=/usr/bin/nslookup
HOSTS_TMP_FILE=/var/run/hosts_tmp
NSLOOKUP_TMP=/var/run/nslookup_tmp

# routines #################################################

get_ip () {
	local tip
	local tmp
	$NSLOOKUP $1 > $NSLOOKUP_TMP 2>/dev/null
	tip=`cat $NSLOOKUP_TMP 2>/dev/null | tail -n +3 | grep "Address" | awk -F" " '{print $3}'`
	if [ ! -z "$tip" ]; then
		for tmp in $tip
		do
			echo "$tmp	$1"
			break
		done
	#else
	#	tip=`cat $NSLOOKUP_TMP 2>/dev/null | tail -n +3 | grep "Address:" | awk -FAddress: '{print $2}' | sed 's/,//g'`
	#	if [ ! -z "$tip" ]; then
	#		echo "$tip	$1"
	#	fi
	fi
	rm -f $NSLOOKUP_TMP
}

detect_ip () {
	check_backup_mode 
	if [ "$?" = "1" ]; then
		get_ip_type $1
		if [ "$?" = "0" ]; then
			echo `get_ip $2`
		fi
	fi
}

config () {
	local ip_domain
	local add_host_ip
	if [ "$wan_bridge_enable" = "1" ]; then
		cp -r $HOSTS_FILE $HOSTS_TMP_FILE
		ip_domain=`cat $HOSTS_TMP_FILE 2>/dev/null | grep $2`
		[ ! -z "$ip_domain" ] && rm -f $HOSTS_TMP_FILE && return 0

		# Add host ip & domain for Dual WAN, Jason 2009/01/17
		[ ! -e /var/run/stop.run ] && add_host_ip=`detect_ip $1 $2`
		[ ! -z "$add_host_ip" ] && echo $add_host_ip >> $HOSTS_TMP_FILE
		mv -f $HOSTS_TMP_FILE $HOSTS_FILE
	fi
}

normal_config() {
	wan_hostname=`flashconfig.sh get HostName`
	wan_domainname="smc.com"
	lan_ipaddr=`flashconfig.sh get lan_ipaddr`
	$HOSTNAME_BIN $wan_hostname.$wan_domainname && \
	echo "127.0.0.1	localhost.localdomain localhost
$lan_ipaddr	$wan_hostname.$wan_domainname $wan_hostname" > $HOSTS_FILE
}

usage () {
        echo "$0 [normal_config|wan_config|backup_wan_config]"
        exit 1
}

# main ##########################################################

[ -z "$1" ] && usage;

err=0

for action in $*; do
	case $action in
		wan_config)		config $wan_dual_wan_master $wan_dual_wan_detect_ip;;
		backup_wan_config)	config $wan_dual_wan_backup $wan_dual_backup_wan_detect_ip;;
		normal_config)		normal_config;;
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

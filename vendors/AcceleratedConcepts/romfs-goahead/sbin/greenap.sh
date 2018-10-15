Module_type_3g="SMCWBR14S-3GN"
Module_type_3g_Qisda="WP3800K"
Module_type="SMCWBR14S-N3"
#Platform=`flashconfig.sh get Platform`
Platform="SMCWBR14S-3GN"

setGreenAP()
{
	start=`flashconfig.sh get GreenAPStart1`
	end=`flashconfig.sh get GreenAPEnd1`
	action=`flashconfig.sh get GreenAPAction1`
	if [ "$action" = "WiFiOFF" ]; then
		echo "$start * * * ifconfig ${ra0} down" >> /var/spool/cron/crontabs/admin
		echo "$end * * * ifconfig ${ra0} up" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX25" ]; then
		echo "$start * * * greenap.sh txpower 25" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX50" ]; then
		echo "$start * * * greenap.sh txpower 50" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX75" ]; then
		echo "$start * * * greenap.sh txpower 75" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	fi
	start=`flashconfig.sh get GreenAPStart2`
	end=`flashconfig.sh get GreenAPEnd2`
	action=`flashconfig.sh get GreenAPAction2`
	if [ "$action" = "WiFiOFF" ]; then
		echo "$start * * * ifconfig ${ra0} down" >> /var/spool/cron/crontabs/admin
		echo "$end * * * ifconfig ${ra0} up" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX25" ]; then
		echo "$start * * * greenap.sh txpower 25" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX50" ]; then
		echo "$start * * * greenap.sh txpower 50" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX75" ]; then
		echo "$start * * * greenap.sh txpower 75" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	fi
	start=`flashconfig.sh get GreenAPStart3`
	end=`flashconfig.sh get GreenAPEnd3`
	action=`flashconfig.sh get GreenAPAction3`
	if [ "$action" = "WiFiOFF" ]; then
		echo "$start * * * ifconfig ${ra0} down" >> /var/spool/cron/crontabs/admin
		echo "$end * * * ifconfig ${ra0} up" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX25" ]; then
		echo "$start * * * greenap.sh txpower 25" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX50" ]; then
		echo "$start * * * greenap.sh txpower 50" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX75" ]; then
		echo "$start * * * greenap.sh txpower 75" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	fi
	start=`flashconfig.sh get GreenAPStart4`
	end=`flashconfig.sh get GreenAPEnd4`
	action=`flashconfig.sh get GreenAPAction4`
	if [ "$action" = "WiFiOFF" ]; then
		echo "$start * * * ifconfig ${ra0} down" >> /var/spool/cron/crontabs/admin
		echo "$end * * * ifconfig ${ra0} up" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX25" ]; then
		echo "$start * * * greenap.sh txpower 25" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX50" ]; then
		echo "$start * * * greenap.sh txpower 50" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	elif [ "$action" = "TX75" ]; then
		echo "$start * * * greenap.sh txpower 75" >> /var/spool/cron/crontabs/admin
		echo "$end * * * greenap.sh txpower normal" >> /var/spool/cron/crontabs/admin
	fi
}

case $1 in
	"init")
# Jason, 20090726
		if [ "$Platform" = "$Module_type_3g" -o "$Platform" = "$Module_type_3g_Qisda" ]; then
			# Jason add CROND 20090724
			wan_bridge_enable=`flashconfig.sh get OperationMode`
			wan_dual_wan_backup=`flashconfig.sh get wan_dual_wan_backup`
			wan_ip_assignment=`flashconfig.sh get wan_ip_assignment`
			if [ "$wan_bridge_enable" = "1" ]; then
				if [ "$wan_dual_wan_backup" = "99" -a "$wan_ip_assignment" = "3" ]; then
					/etc/init.d/cron.sh start
				else
					/etc/init.d/cron.sh start 2
			fi
			else
				/etc/init.d/cron.sh start 2
			fi
		else
		killall -q crond
		mkdir -p /var/spool/cron/crontabs
		rm -f /var/spool/cron/crontabs/admin
		cronebl="0"
		action=`flashconfig.sh get GreenAPAction1`
		if [ "$action" != "Disable" -a "$action" != "" ]; then
			start=`flashconfig.sh get GreenAPStart1`
			cronebl="1"
			greenap.sh setchk $start
		fi
		action=`flashconfig.sh get GreenAPAction2`
		if [ "$action" != "Disable" -a "$action" != "" ]; then
			start=`flashconfig.sh get GreenAPStart2`
			cronebl="1"
			greenap.sh setchk $start
		fi
		action=`flashconfig.sh get GreenAPAction3`
		if [ "$action" != "Disable" -a "$action" != "" ]; then
			start=`flashconfig.sh get GreenAPStart3`
			cronebl="1"
			greenap.sh setchk $start
		fi
		action=`flashconfig.sh get GreenAPAction4`
		if [ "$action" != "Disable" -a "$action" != "" ]; then
			start=`flashconfig.sh get GreenAPStart4`
			cronebl="1"
			greenap.sh setchk $start
		fi
		if [ "$cronebl" = "1" ]; then
			crond
		fi
		fi
# end
		;;
	"setchk")
		setGreenAP
		;;
#M Tom scheduling  cron job 04-30-2009  begin
	"3gconnection")
		echo "*/1  * * * *  /etc/init.d/3g.sh cron_3g_restart > /dev/null" >> /var/spool/cron/crontabs/admin
		killall -q crond
		/usr/sbin/crond -c /var/spool/cron/crontabs -l 8
		;;
	"3gBCctrl")
		echo "*/1  * * * *  /etc/init.d/3g.sh cron_3g_budget_control > /dev/null" >> /var/spool/cron/crontabs/admin
		killall -q crond
		/usr/sbin/crond -c /var/spool/cron/crontabs -l 8
		;;
	"NTPcronjob")
		echo "0,31 * * * *  /etc_ro/init.d/ntp.sh config > /dev/null" >> /var/spool/cron/crontabs/admin
		killall -q crond
		/usr/sbin/crond -c /var/spool/cron/crontabs -l 8
		;;
#	"chkntp")
#		cat /var/spool/cron/crontabs/admin | sed '/ifconfig/d' > /var/spool/cron/crontabs/admin
#		cat /var/spool/cron/crontabs/admin | sed '/txpower/d' > /var/spool/cron/crontabs/admin
#		index=1
#		while [ "$index" -le 10 ]
#		do
#			ntpvalid=`flashconfig.sh get NTPValid`
#			if [ "$ntpvalid" = "1" ]; then
#				setGreenAP
#				break;
#			else
#				index=`expr $index + 1`
#				sleep 5
#			fi
#		done
#		killall -q crond
#		crond
#		;;
	"txpower")
		if [ "$2" = "normal" ]; then
			ralink_init gen 2860
			ifconfig ${ra0} down
			ifconfig ${ra0} up
		else
			cat /etc/Wireless/RT2860/RT2860.dat | sed '/TxPower/d' > /etc/Wireless/RT2860/RT2860.dat
			echo "TxPower=$2" >> /etc/Wireless/RT2860/RT2860.dat
			ifconfig ${ra0} down
			ifconfig ${ra0} up
		fi
		;;
esac

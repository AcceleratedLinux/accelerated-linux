#!/bin/sh
#
# take the nvram settings and produce a hostapd.conf
# david.mccullough@accelecon.com
#
# usage: internet.sh
#

. /sbin/config.sh
. /sbin/global.sh

#########################################################################

# helper function to write settings to hostap
# simpleconfig flash-name hostapd-name
simple_config()
{
	val=`flashconfig.sh get $1`
	[ "$val" ] && echo "$2=$val" >&9
}

# wmm_server_config flash-name hostapd-sub-name
wmm_server_config() {
	vals=`flashconfig.sh get $1 | sed 's/;/ /g'`
	n=0
	for i in $vals; do
		case "$i" in
		${3:-*})
			case "$n" in
			0) d="data2" ;; # be
			1) d="data3" ;; # bk
			2) d="data1" ;; # vi
			3) d="data0" ;; # vo
			esac
			echo "tx_queue_${d}_${2}=$i" >&9
			;;
		esac
		n=`expr $n + 1`
	done
}

# wmm_client_config flash-name hostapd-sub-name
wmm_client_config() {
	vals=`flashconfig.sh get $1 | sed 's/;/ /g'`
	n=0
	for i in $vals; do
		case "$i" in
		${3:-*})
			case "$n" in
			0) d="be" ;;
			1) d="bk" ;;
			2) d="vi" ;;
			3) d="vo" ;;
			esac
			echo "wmm_ac_${d}_${2}=$i" >&9
			;;
		esac
		n=`expr $n + 1`
	done
}

# output wep data: wep_output id
wep_output()
{
	case "$authmode" in
	*WEP*) ;;
	*) return ;;
	esac

	case "$encryptype" in
	WEP)
		if [ "$1" = "1" ]; then
			defkey=`flashconfig.sh get DefaultKeyID`
			[ "$defkey" ] || defkey=1
			echo "wep_default_key=`expr $defkey - 1`" >&9
		fi
		for i in 1 2 3 4
		do
			type=`flashconfig.sh get Key${i}Type`
			[ "$type" ] || continue
			if [ "$type" = "0" ]; then
				echo "wep_key`expr $i - 1`=`flashconfig.sh get Key${i}Str${1}`" >&9
			else
				echo "wep_key`expr $i - 1`=\"`flashconfig.sh get Key${i}Str${1}`\"" >&9
			fi
		done
		;;
	*)
	esac
}

wap_output()
{
	:
}

# acl_output bssid-index
acl_output()
{
	policy=`flashconfig.sh get AccessPolicy$1`
	if [ "$policy" ]; then
		case "$policy" in
		0)	;;
		1) # allow mode
			echo "macaddr_acl=1" >&9
			echo "accept_mac_file=/etc/config/hostapd.accept" >&9
			flashconfig.sh get AccessControlList$1 | sed 's/;/\n/g' > \
				/etc/config/hostapd.accept
			;;
		2) # reject mode
			echo "macaddr_acl=0" >&9
			echo "deny_mac_file=/etc/config/hostapd.deny" >&9
			flashconfig.sh get AccessControlList$1 | sed 's/;/\n/g' > \
				/etc/config/hostapd.deny
			;;
		esac
	fi
}

#########################################################################

wlan=`flashconfig.sh get ra0`
opmode=`flashconfig.sh get OperationMode`
encryptype=`flashconfig.sh get EncrypType`
bssidnum=`flashconfig.sh get BssidNum`
authmode=`flashconfig.sh get AuthMode`

exec 9> /etc/config/hostapd.conf

# common hostapd settings here
echo "ctrl_interface=/var/run/hostapd" >&9

# settings common to as BSSid's
simple_config BeaconPeriod beacon_int
simple_config DtimPeriod dtim_period
simple_config RTSThreshold rts_threshold
simple_config FragThreshold fragm_threshold
simple_config TxPreamble preamble
simple_config APSDCapable uapsd_advertisement_enabled
simple_config NoForwarding ap_isolate

simple_config WmmCapable wmm_enabled
wmm_server_config APAifsn aifs
# wmm_server_config AckPolicy
wmm_server_config APCwmax cwmax "1|3|7|15|31|63|127|255|511|1023"
wmm_server_config APCwmin cwmin "1|3|7|15|31|63|127|255|511|1023"
# wmm_server_config APTxop txop_limit
# wmm_server_config APACM acm

wmm_client_config BSSAifsn aifs
wmm_client_config BSSCwmax cwmax "1|3|7|15|31|63|127|255|511|1023"
wmm_client_config BSSCwmin cwmin "1|3|7|15|31|63|127|255|511|1023"
wmm_client_config BSSTxop txop_limit
wmm_client_config BSSACM acm

if [ "$opmode" = 1 ]; then
	# router mode
	: not done yet
else
	# bridge mode
	echo "interface=$wlan" >&9
	echo "driver=nl80211" >&9
	echo "wds_sta=1" >&9
	radiooff=`flashconfig.sh get RadioOff`

	wirelessmode=`flashconfig.sh get WirelessMode`
	case "$wirelessmode" in
	0) echo "hw_mode=g" >&9    ;; # 11b/g mixed mode</option>
	1) echo "hw_mode=b" >&9    ;; # 11b only</option>
	4) echo "hw_mode=g" >&9    ;; # 11g only</option>
	9) echo "hw_mode=g" >&9       # 11b/g/n mixed mode</option>
	   echo "ieee80211n=1" >&9 ;;
	6) echo "ieee80211n=1" >&9 ;; # 11n only</option>
	7) echo "hw_mode=g" >&9       # 11g/n mixed mode</option>
	   echo "ieee80211n=1" >&9 ;;
	*) ;;
	esac

	echo "channel=`flashconfig.sh get Channel`" >&9
	echo "ssid=`flashconfig.sh get SSID1`" >&9
	simple_config HideSSID ignore_broadcast_ssid
	echo "bridge=br0" >&9
fi

ht_capab=
[ "`flashconfig.sh get HT_BW`" = "1" ] && ht_capab="$ht_capab[HT40-][HT40+]"
[ "`flashconfig.sh get HT_BW`" = "1" ] && ht_capab="$ht_capab[TX-STBC][RX-STBC123]"
[ "`flashconfig.sh get HT_AMSDU`" = "1" ] && ht_capab="$ht_capab[MAX-AMSDU-7935]"
[ "$ht_capab" ] && echo "ht_capab=$ht_capab" >&9

wep_output 1
#wap_output
acl_output 0

# output any other SSID's we have
bssid=2
while [ "$bssid" -lt "$bssidnum" ]
do
	ssid=`flashconfig.sh get SSID$bssid`
	[ "$ssid" ] || continue
	echo "bss=${wlan}_`expr $bssid - 2`" >&9
	echo "ssid=$ssid" >&9
	wep_output $bssid
	acl_output `expr $bssid - 1`
	simple_config HideSSID$bssid ignore_broadcast_ssid
	bssid=`expr bssid + 1`
done

exec 9>&-

#########################################################################
# reload the hostapd daemon

if grep hostapd /proc/`cat /var/run/hostapd.pid 2> /dev/null`/comm > /dev/null 2>&1; then
	killall -HUP hostapd
else
	hostapd -B -P /var/run/hostapd.pid /etc/config/hostapd.conf
fi

# give it a chance to get moving
sleep 2

exit 0

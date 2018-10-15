#!/bin/sh
#echo "`date` SMXQUERY: response received from server $new_server1" >> /tmp/smx.log

new_server1=`fgrep '[Smx.0.IPv4]' /tmp/smx.rsp | cut -f2 -d"'"`
new_server2=`fgrep '[Smx.1.IPv4]' /tmp/smx.rsp | cut -f2 -d"'"`
smx_server1=$(flashconfig.sh get smx_server1)
smx_server2=$(flashconfig.sh get smx_server2)

if [ "$new_server1" != "$smx_server1" ]; then
  echo "SMXQUERY: new server=[$new_server1]" >> /tmp/smx.log
  flashconfig.sh set smx_server1 "$new_server1"
fi

if [ "$new_server2" != "$smx_server2" ]; then
  echo "SMXQUERY: new server2=[$new_server2]" >> /tmp/smx.log
  flashconfig.sh set smx_server2 "$new_server2"
fi

new_logipv4=`fgrep '[Log.0.IPv4]' /tmp/smx.rsp | cut -f2 -d"'"`
smx_logipv4=`flashconfig.sh get smx_logipv4`
smx_logipv4=${smx_logipv4:-184.106.215.57}
if [ "$new_logipv4" != "$smx_logipv4" ]; then
  echo "SMXQUERY: new logipv4=[$new_logipv4]" >> /tmp/smx.log
  flashconfig.sh set smx_logipv4 "$new_logipv4"
fi

# Get any additional old nvram values we need.

web_login=`         flashconfig.sh get Login`
web_pass=`          flashconfig.sh get Password`
web_ssid1=`         flashconfig.sh get SSID1`
web_ssid2=`         flashconfig.sh get SSID2`
web_ssidhide=`      flashconfig.sh get HideSSID`
web_ssidnum=`       flashconfig.sh get BssidNum`
web_channel=`       flashconfig.sh get Channel`
web_daylightenable=`flashconfig.sh get DaylightEnable`
web_txpower=`       flashconfig.sh get TxPower`
web_wirelessmode=`  flashconfig.sh get WirelessMode`
web_tz=`            flashconfig.sh get TZ`
web_authmode=`      flashconfig.sh get AuthMode`
web_encryptype=`    flashconfig.sh get EncrypType`
web_key1type=`      flashconfig.sh get Key1Type`
web_key1str1=`      flashconfig.sh get Key1Str1`
web_key1str2=`      flashconfig.sh get Key1Str2`
web_wpapsk1=`       flashconfig.sh get WPAPSK1`
web_wpapsk2=`       flashconfig.sh get WPAPSK2`
web_radkey=`        flashconfig.sh get RADIUS_Key1`
web_radkey2=`       flashconfig.sh get RADIUS_Key2`
web_radserver=`     flashconfig.sh get RADIUS_Server`
web_radport=`       flashconfig.sh get RADIUS_Port`
web_radsti=`        flashconfig.sh get session_timeout_interval`
web_wdsmode=`       flashconfig.sh get WdsEnable`
web_wdslist=`       flashconfig.sh get WdsList`
web_logenabled=`    flashconfig.sh get smx_logenabled`
web_webaccess=`     flashconfig.sh get smx_webaccess`
web_vlanid_ssid1=`  flashconfig.sh get smx_vlanid_ssid1`
web_vlanid_ssid2=`  flashconfig.sh get smx_vlanid_ssid2`
web_vlanid_unit=`   flashconfig.sh get smx_vlanid_unit`

# Get smx values.  Key names are [] quoted, values are single-quoted.

smx_accountid=`     cat /tmp/smx.rsp | fgrep '[AccountId]'       | cut -f2 -d"'"`
smx_login=`         cat /tmp/smx.rsp | fgrep '[AdminUsername]'   | cut -f2 -d"'"`
smx_devstatus=`     cat /tmp/smx.rsp | fgrep '[DeviceStatus]'    | cut -f2 -d"'"`
smx_logenabled=`    cat /tmp/smx.rsp | fgrep '[LogEnabled]'      | cut -f2 -d"'"`
smx_logipv4=`       cat /tmp/smx.rsp | fgrep '[Log.0.IPv4]'      | cut -f2 -d"'"`
smx_pass=`          cat /tmp/smx.rsp | fgrep '[AdminPassword]'   | cut -f2 -d"'"`
smx_webaccess=`     cat /tmp/smx.rsp | fgrep '[AdminAccess]'     | cut -f2 -d"'"`
smx_ssid1=`         cat /tmp/smx.rsp | fgrep '[Ssid1]'           | cut -f2 -d"'"`
smx_ssid2=`         cat /tmp/smx.rsp | fgrep '[Ssid2]'           | cut -f2 -d"'"`
smx_ssidhide=`      cat /tmp/smx.rsp | fgrep '[SsidAdvertise]'   | cut -f2 -d"'"`
smx_channel=`       cat /tmp/smx.rsp | fgrep '[Channel]'         | cut -f2 -d"'"`
smx_channelregion=` cat /tmp/smx.rsp | fgrep '[ChannelRegion]'   | cut -f2 -d"'"`
smx_countrycode=`   cat /tmp/smx.rsp | fgrep '[CountryCode]'	   | cut -f2 -d"'"`
smx_daylightenable=`cat /tmp/smx.rsp | fgrep '[TzDaylight]'      | cut -f2 -d"'"`
smx_txpower=`       cat /tmp/smx.rsp | fgrep '[RfPower]'         | cut -f2 -d"'"`
smx_authmode=`      cat /tmp/smx.rsp | fgrep '[Security]'        | cut -f2 -d"'"`
smx_authmode2=`     cat /tmp/smx.rsp | fgrep '[Security2]'       | cut -f2 -d"'"`
smx_encryptype=`    cat /tmp/smx.rsp | fgrep '[WpaWpa2Cipher]'   | cut -f2 -d"'"`
smx_encryptype2=`   cat /tmp/smx.rsp | fgrep '[WpaWpa2Cipher2]'  | cut -f2 -d"'"`
smx_weplength=`	    cat /tmp/smx.rsp | fgrep '[WepKeyLength]'    | cut -f2 -d"'"`
smx_weplength2=`    cat /tmp/smx.rsp | fgrep '[WepKeyLength2]'   | cut -f2 -d"'"`
smx_key1type=`      cat /tmp/smx.rsp | fgrep '[PassPhraseType]'  | cut -f2 -d"'"`
smx_key1str1=`      cat /tmp/smx.rsp | fgrep '[PassPhrase]'      | cut -f2 -d"'"`
smx_key1type2=`     cat /tmp/smx.rsp | fgrep '[PassPhraseType2]' | cut -f2 -d"'"`
smx_key1str2=`      cat /tmp/smx.rsp | fgrep '[PassPhrase2]'     | cut -f2 -d"'"`
smx_wirelessmode=`  cat /tmp/smx.rsp | fgrep '[NetworkMode]'     | cut -f2 -d"'"`
smx_devmode=`	      cat /tmp/smx.rsp | fgrep '[DeviceMode]'	     | cut -f2 -d"'"`
smx_tz=`            cat /tmp/smx.rsp | fgrep '[TzOffset]'        | cut -f2 -d"'"`
smx_radserver=`     cat /tmp/smx.rsp | fgrep '[RadServerIPv4]'   | cut -f2 -d"'"`
smx_radport=`       cat /tmp/smx.rsp | fgrep '[RadServerPort]'   | cut -f2 -d"'"`
smx_radport2=`      cat /tmp/smx.rsp | fgrep '[RadServerPort2]'  | cut -f2 -d"'"`
smx_radkey=`        cat /tmp/smx.rsp | fgrep '[RadSharedKey]'    | cut -f2 -d"'"`
smx_radkey2=`       cat /tmp/smx.rsp | fgrep '[RadSharedKey2]'   | cut -f2 -d"'"`
smx_radsti=`        cat /tmp/smx.rsp | fgrep '[RadTimeoutSecs]'  | cut -f2 -d"'"`
smx_wdsmode=`       cat /tmp/smx.rsp | fgrep '[WdsMode]'         | cut -f2 -d"'"`
smx_wdsmac1=`       cat /tmp/smx.rsp | fgrep '[WdsMacAddress1]'  | cut -f2 -d"'"`
smx_wdsmac2=`       cat /tmp/smx.rsp | fgrep '[WdsMacAddress2]'  | cut -f2 -d"'"`
smx_wdsmac3=`       cat /tmp/smx.rsp | fgrep '[WdsMacAddress3]'  | cut -f2 -d"'"`
smx_vlanid_ssid1=`  cat /tmp/smx.rsp | fgrep '[Vlan]'            | cut -f2 -d"'"`
smx_vlanid_ssid2=`  cat /tmp/smx.rsp | fgrep '[Vlan2]'           | cut -f2 -d"'"`
smx_vlanid_unit=`   cat /tmp/smx.rsp | fgrep '[VlanUnit]'        | cut -f2 -d"'"`

# Syslog all of our current SMX settings and current LAN IP address

/sbin/syslog.sh i smx_info "AccountId=$smx_accountid~AdminUsername=$smx_login~AdminPassword=$smx_pass~AdminAccess=$smx_webaccess~DeviceStatus=$smx_devstatus~DeviceMode=$smx_devmode~NetworkMode=$smx_wirelessmode~LogEnabled=$smx_logenabled~LogIPv4=$smx_logipv4~Ssid1=$smx_ssid1~Ssid2=$smx_ssid2~SsidAdvertise=$smx_ssidhide~Security1=$smx_authmode;$smx_authmode2~WpaWpa2Cipher=$smx_encryptype;$smx_encryptype2~WepKeyLength=$smx_weplength~WepKeyLength2=$smx_weplength2~PassPhraseType=$smx_key1type;$smx_key1type2~PassPhrase=$smx_key1str1~PassPhrase2=$smx_key1str2~Channel=$smx_channel~ChannelRegion=$smx_channelregion~CountryCode=$smx_countrycode~TzDaylight=$smx_daylightenable~TzOffset=$smx_tz~RfPower=$smx_txpower~RadServerIPv4=$smx_radserver~RadServerPort=$smx_radport;$smx_radport2~RadSharedKey=$smx_radkey~RadSharedKey2=$smx_radkey2~RadTimeoutSecs=$smx_radsti~WdsMode=$smx_wdsmode~WdsMacAddress1=$smx_wdsmac1~WdsMacAddress2=$smx_wdsmac2~WdsMacAddress3=$smx_wdsmac3~VlanUnit=$smx_vlanid_unit~Vlan=$smx_vlanid_ssid1~Vlan2=$smx_vlanid_ssid2"

lan_ip_address=`flashconfig.sh get lan_ipaddr`
/sbin/syslog.sh i info "Listening for HTTP requests at address $lan_ip_address"

# Remap smx values to web (nvram) values.

if [ "$smx_webaccess" == '0' ] ; then smx_webaccess='N' ; fi
if [ "$smx_webaccess" != '0' ] ; then smx_webaccess='Y' ; fi

if [ "$smx_daylightenable" == 'Y' ] ; then smx_daylightenable='1' ; fi
if [ "$smx_daylightenable" == 'N' ] ; then smx_daylightenable='0' ; fi

if [ "$smx_txpower" != '' ]; then
  while [ "`echo $smx_txpower | cut -c1`" == '0' ]; do smx_txpower="`echo $smx_txpower | cut -c2-`" ; done
  if [ "$smx_txpower" == '' ]; then
    smx_txpower='0'
  fi
fi

if [ "$smx_devstatus" == 'I' -o "$smx_devstatus" == 'D' ]; then
  smx_txpower='0'
fi

if [ "$smx_wirelessmode" == '4' ] ; then smx_wirelessmode='0' ; fi
if [ "$smx_wirelessmode" == '1' ] ; then smx_wirelessmode='1' ; fi
if [ "$smx_wirelessmode" == '2' ] ; then smx_wirelessmode='4' ; fi
if [ "$smx_wirelessmode" == '6' ] ; then smx_wirelessmode='9' ; fi
if [ "$smx_wirelessmode" == '3' ] ; then smx_wirelessmode='6' ; fi
if [ "$smx_wirelessmode" == '5' ] ; then smx_wirelessmode='7' ; fi

# Timezones are a mess no matter how you slice em.  The 'tz' variable
# in nvram has to correspond to valid entries in the web interface,
# so that is why we use the odd values here.  These values later get
# translated into more meaningful smx_tz values which will actually
# get written to the /etc/TZ file by the /sbin/ntp.sh script to
# determine the effective local date/time rules.

if [ "$smx_tz" == '-720' ] ; then smx_tz='GMT_-12'  ; fi
if [ "$smx_tz" == '-660' ] ; then smx_tz='GMT_-11'  ; fi
if [ "$smx_tz" == '-600' ] ; then smx_tz='GMT1_-10' ; fi
if [ "$smx_tz" == '-540' ] ; then smx_tz='GMT_-09'  ; fi
if [ "$smx_tz" == '-480' ] ; then smx_tz='GMT_-08'  ; fi
if [ "$smx_tz" == '-420' ] ; then smx_tz='GMT2_-07' ; fi
if [ "$smx_tz" == '-360' ] ; then smx_tz='GMT4_-06' ; fi
if [ "$smx_tz" == '-300' ] ; then smx_tz='GMT2_-05' ; fi
if [ "$smx_tz" == '-240' ] ; then smx_tz='GMT_-04'  ; fi
if [ "$smx_tz" == '-180' ] ; then smx_tz='GMT_-03'  ; fi
if [ "$smx_tz" == '-120' ] ; then smx_tz='GMT_-02'  ; fi
if [ "$smx_tz" ==  '-60' ] ; then smx_tz='GMT_-01'  ; fi
if [ "$smx_tz" ==    '0' ] ; then smx_tz='GMT1_000' ; fi
if [ "$smx_tz" ==   '60' ] ; then smx_tz='GMT_001'  ; fi
if [ "$smx_tz" ==  '120' ] ; then smx_tz='GMT3_002' ; fi
if [ "$smx_tz" ==  '180' ] ; then smx_tz='GMT2_003' ; fi
if [ "$smx_tz" ==  '240' ] ; then smx_tz='GMT2_004' ; fi
if [ "$smx_tz" ==  '300' ] ; then smx_tz='GMT1_005' ; fi
if [ "$smx_tz" ==  '360' ] ; then smx_tz='GMT2_006' ; fi
if [ "$smx_tz" ==  '420' ] ; then smx_tz='GMT_007'  ; fi
if [ "$smx_tz" ==  '480' ] ; then smx_tz='GMT_008'  ; fi
if [ "$smx_tz" ==  '540' ] ; then smx_tz='GMT1_009' ; fi
if [ "$smx_tz" ==  '600' ] ; then smx_tz='GMT2_010' ; fi
if [ "$smx_tz" ==  '660' ] ; then smx_tz='GMT_011'  ; fi
if [ "$smx_tz" ==  '720' ] ; then smx_tz='GMT1_012' ; fi
if [ "$smx_tz" ==  '780' ] ; then smx_tz='GMT_013'  ; fi

####Configure security setup for SSID1####

# WEP passphrase type (cleared below if not WEP).

if [ "$smx_key1type" == 'X' ]; then smx_key1type='0' ; fi
if [ "$smx_key1type" == 'C' ]; then smx_key1type='1' ; fi

# WPA cipher type (cleared below if not WPA).

if [ "$smx_encryptype" == 'A' ]; then smx_encryptype='AES'  ; fi
if [ "$smx_encryptype" == 'T' ]; then smx_encryptype='TKIP' ; fi

# No encryption mode.

if [ "$smx_authmode" == '0' ]; then
  smx_authmode='OPEN'
  smx_encryptype='NONE'
  smx_key1type='0'
  smx_key1str1=''
  smx_wpapsk1=''
fi

# WEP modes (open or shared).

if [ "$smx_authmode" == '1' ]; then
  smx_authmode='OPEN'
  smx_encryptype='WEP'
  smx_wpapsk1=''
fi
if [ "$smx_authmode" == '2' ]; then
  smx_authmode='SHARED'
  smx_encryptype='WEP'
  smx_wpapsk1=''
fi

# WPA modes (WPA personal, WPA2 personal, WPA2 mixed).

if [ "$smx_authmode" == '3' ]; then
  smx_authmode='WPAPSK'
  smx_wpapsk1="$smx_key1str1"
  smx_key1type='0'
  smx_key1str1=''
fi
if [ "$smx_authmode" == '4' ]; then
  smx_authmode='WPA2PSK'
  smx_wpapsk1="$smx_key1str1"
  smx_key1type='0'
  smx_key1str1=''
fi
if [ "$smx_authmode" == '5' ]; then
  smx_authmode='WPAPSKWPA2PSK'
  smx_wpapsk1="$smx_key1str1"
  smx_key1type='0'
  smx_key1str1=''
fi

# Enterprise modes (WPA, WPA2, WPA1/WPA2 mixed).  These all need
# a RADIUS server ipv4, port and timeout.

if [ "$smx_authmode" == '6' ]; then
  smx_authmode='WPA'
fi
if [ "$smx_authmode" == '7' ]; then
  smx_authmode='WPA2'
fi
if [ "$smx_authmode" == '8' ]; then
  smx_authmode='WPA1WPA2'
fi


####Configure security setup for SSID2####

# Set necessary values to enable SSID2
if [ "$smx_ssid2" != '' -a "$web_ssidnum" != '2' ]; then
  flashconfig.sh set BssidNum 2
fi

# WEP passphrase type (cleared below if not WEP).

if [ "$smx_key1type2" == 'X' ]; then smx_key1type2='0' ; fi
if [ "$smx_key1type2" == 'C' ]; then smx_key1type2='1' ; fi

# WPA cipher type (cleared below if not WPA).

if [ "$smx_encryptype2" == 'A' ]; then smx_encryptype2='AES'  ; fi
if [ "$smx_encryptype2" == 'T' ]; then smx_encryptype2='TKIP' ; fi


# No encryption mode.

if [ "$smx_authmode2" == '0' ]; then
  smx_authmode2='OPEN'
  smx_encryptype2='NONE'
  smx_key1type2='0'
  smx_key1str2=''
  smx_wpapsk2=''
fi

# WEP modes (open or shared).

if [ "$smx_authmode2" == '1' ]; then
  smx_authmode2='OPEN'
  smx_encryptype2='WEP'
  smx_wpapsk2=''
fi
if [ "$smx_authmode2" == '2' ]; then
  smx_authmode2='SHARED'
  smx_encryptype2='WEP'
  smx_wpapsk2=''
fi

# WPA modes (WPA personal, WPA2 personal, WPA2 mixed).

if [ "$smx_authmode2" == '3' ]; then
  smx_authmode2='WPAPSK'
  smx_wpapsk2="$smx_key1str2"
  smx_key1type2='0'
  smx_key1str2=''
fi
if [ "$smx_authmode2" == '4' ]; then
  smx_authmode2='WPA2PSK'
  smx_wpapsk2="$smx_key1str2"
  smx_key1type2='0'
  smx_key1str2=''
fi
if [ "$smx_authmode2" == '5' ]; then
  smx_authmode2='WPAPSKWPA2PSK'
  smx_wpapsk2="$smx_key1str2"
  smx_key1type2='0'
  smx_key1str2=''
fi

# Enterprise modes (WPA, WPA2, WPA1/WPA2 mixed).  These all need
# a RADIUS server ipv4, port and timeout.

if [ "$smx_authmode2" == '6' ]; then
  smx_authmode2='WPA'
fi
if [ "$smx_authmode2" == '7' ]; then
  smx_authmode2='WPA2'
fi
if [ "$smx_authmode2" == '8' ]; then
  smx_authmode2='WPA1WPA2'
fi

# Set SSID timeouts
ssidti=`flashconfig.sh get RekeyInterval`
if [ "$smx_ssid1" != '' ]; then
  if [ "$smx_radsti" != '' ]; then
    sti="$smx_radsti"
  else
    sti='3600'
  fi
  if [ "$smx_ssid2" != '' ]; then
    set_ssidti="$sti;$sti"
  else
    set_ssidti="$sti"
  fi
fi
if [ "$ssidti" != "$set_ssidti" -a "$set_ssidti" != '' ]; then
  flashconfig.sh set RekeyInterval "$set_ssidti"
  touch /tmp/flag.reboot
fi


# WDS Modes (Disabled-0, Bridge-2, Repeater-3, or Lazy-4)
# - Bridge mode allows only the listed MAC addresses to connect
# - Repeater mode fuctions like Bridge mode, except wireless clients
#    other than the listed MAC address can still connect (they still have
#    to provide the PassPhrase and relevant Security credentials)
# - Lazy mode tries to automatically connect to whatever NetReach WDS 
#    clients are in range.  Not MAC addresses can be specified in Lazy mode
#    Note: one NetReach must be in Bridge or Repeater mode, while up to 
#    three can be in Lazy Mode to connect back to the main device

if [ "$smx_wdsmode" == '2' -o "$smx_wdsmode" == '3' ]; then
  smx_wdslist="$smx_wdsmac1;$smx_wdsmac2;$smx_wdsmac3;"
fi

# VLAN Trunking
# SSIDs can be tagged with vlans.
# If VlanUnit is not present and Vlan is enabled (done by having at least 
# one Vlan value), NetReach will use Vlanid_ssid1 value
if [ ! "$smx_vlanid_unit" -a "$smx_vlanid_ssid1" ]; then
  smx_vlanid_unit=$smx_vlanid_ssid1
fi

# Apply changes.  For now, because goahead is so fixated on the admin login
# name, simply ignore any attempt by smx to change the login name.

smx_login='admin'

if [ "$smx_login" != '' -a "$smx_pass" != '' ]; then
  if [ "$smx_login" != "$web_login" -o "$smx_pass" != "$web_pass" ]; then
    echo ">>> Smx-Query: login/pass change [$web_login:$smx_login] [$web_pass:$smx_pass]"
    flashconfig.sh set Login "$smx_login"
    flashconfig.sh set Password "$smx_pass"
    touch /tmp/flag.reboot
  fi
fi

if [ "$smx_ssid1" != '' -a "$smx_ssid1" != "$web_ssid1" ]; then
  echo ">>> Smx-Query: ssid1 change [$web_ssid1:$smx_ssid1]"
  flashconfig.sh set SSID1 "$smx_ssid1"
  touch /tmp/flag.reboot
fi

if [ "$smx_ssid2" != '' -a "$smx_ssid2" != "$web_ssid2" ]; then
  echo ">>> Smx-Query: ssid2 change [$web_ssid2:$smx_ssid2]"
  flashconfig.sh set SSID2 "$smx_ssid2"
  touch /tmp/flag.reboot
fi

if [ "$smx_ssidhide" != '' -a "$smx_ssidhide" != "$web_ssidhide" ]; then
  echo ">>> Smx-Query: ssidhide change [$web_ssidhide:$smx_ssidhide]"
  flashconfig.sh set HideSSID "$smx_ssidhide"
  touch /tmp/flag.reboot
fi

if [ "$smx_channel" != '' -a "$smx_channel" != "$web_channel" ]; then
  echo ">>> Smx-Query: channel change [$web_channel:$smx_channel]"
  flashconfig.sh set Channel "$smx_channel"
  touch /tmp/flag.reboot
fi

if [ "$smx_wirelessmode" != '' -a "$smx_wirelessmode" != "$web_wirelessmode" ]; then
  echo ">>> Smx-Query: wirelessmode change [$web_wirelessmode:$smx_wirelessmode]"
  flashconfig.sh set WirelessMode "$smx_wirelessmode"
  touch /tmp/flag.reboot
fi

if [ "$smx_daylightenable" != '' -a "$smx_daylightenable" != "$web_daylightenable" ]; then
  echo ">>> Smx-Query: daylightenable change [$web_daylightenable:$smx_daylightenable]"
  flashconfig.sh set DaylightEnable "$smx_daylightenable"
  touch /tmp/flag.reboot
fi

if [ "$smx_tz" != '' -a "$smx_tz" != "$web_tz" ]; then
  echo ">>> Smx-Query: tz change [$web_tz:$smx_tz]"
  flashconfig.sh set TZ "$smx_tz"
  touch /tmp/flag.reboot
fi

if [ "$smx_txpower" != '' -a "$smx_txpower" != "$web_txpower" ]; then
  echo ">>> Smx-Query: txpower change [$web_txpower:$smx_txpower]"
  flashconfig.sh set TxPower "$smx_txpower"
  touch /tmp/flag.reboot
fi

if [ "$smx_authmode;$smx_authmode2" != "$web_authmode" ]; then
  echo ">>> Smx-Query: authmode change [$web_authmode : $smx_authmode;$smx_authmode2]"
  flashconfig.sh set AuthMode "$smx_authmode;$smx_authmode2"
  touch /tmp/flag.reboot
fi

if [ "$smx_encryptype;$smx_encryptype2" != "$web_encryptype" ]; then
  echo ">>> Smx-Query: encryptype change [$web_encryptype : $smx_encryptype;$smx_encryptype2]"
  flashconfig.sh set EncrypType "$smx_encryptype;$smx_encryptype2"
  touch /tmp/flag.reboot
fi

if [ "$smx_key1type;$smx_key1type2" != "$web_key1type" ]; then
  echo ">>> Smx-Query: key1type change [$web_key1type : $smx_key1type;$smx_key1type2]"
  flashconfig.sh set Key1Type "$smx_key1type;$smx_key1type2"
  touch /tmp/flag.reboot
fi

if [ "$smx_key1str1" != "$web_key1str1" ]; then
  echo ">>> Smx-Query: key1str1 change [$web_key1str1:$smx_key1str1]"
  flashconfig.sh set Key1Str1 "$smx_key1str1"
  touch /tmp/flag.reboot
fi

if [ "$smx_key1str2" != "$web_key1str2" ]; then
  echo ">>> Smx-Query: key1str2 change [$web_key1str2:$smx_key1str2]"
  flashconfig.sh set Key1Str2 "$smx_key1str2"
  touch /tmp/flag.reboot
fi

if [ "$smx_wpapsk1" != "$web_wpapsk1" ]; then
  echo ">>> Smx-Query: wpapsk1 change [$web_wpapsk1:$smx_wpapsk1]"
  flashconfig.sh set WPAPSK1 "$smx_wpapsk1"
  touch /tmp/flag.reboot
fi

if [ "$smx_wpapsk2" != "$web_wpapsk2" ]; then
  echo ">>> Smx-Query: wpapsk2 change [$web_wpapsk2:$smx_wpapsk2]"
  flashconfig.sh set WPAPSK2 "$smx_wpapsk2"
  touch /tmp/flag.reboot
fi

if [ "$smx_radserver;;;0" != "$web_radserver" ]; then
  echo ">>> Smx-Query: radserver change [$web_radserver:$smx_radserver;;;0]"
  flashconfig.sh set RADIUS_Server "$smx_radserver;;;0"
  touch /tmp/flag.reboot
fi

if [ "$smx_radport;$smx_radport2;;1812" != "$web_radport" ]; then
  echo ">>> Smx-Query: radport change [$web_radport : $smx_radport;$smx_radport2;;1812]"
  flashconfig.sh set RADIUS_Port "$smx_radport;$smx_radport2;;1812"
  touch /tmp/flag.reboot
fi

if [ "$smx_radkey" != "$web_radkey" ]; then
  echo ">>> Smx-Query: radkey change [$web_radkey:$smx_radkey]"
  flashconfig.sh set RADIUS_Key1 "$smx_radkey"
  touch /tmp/flag.reboot
fi

if [ "$smx_radkey2" != "$web_radkey2" ]; then
  echo ">>> Smx-Query: radkey2 change [$web_radkey2:$smx_radkey2]"
  flashconfig.sh set RADIUS_Key2 "$smx_radkey2"
  touch /tmp/flag.reboot
fi

if [ "$smx_radsti" != "$web_radsti" ]; then
  echo ">>> Smx-Query: radsti change [$web_radsti:$smx_radsti]"
  flashconfig.sh set session_timeout_interval "$smx_radsti"
  touch /tmp/flag.reboot
fi

if [ "$smx_wdsmode" != "$web_wdsmode" ]; then
  echo ">>> Smx-Query: wdsmode change [$web_wdsmode:$smx_wdsmode]"
  flashconfig.sh set WdsEnable "$smx_wdsmode"
  touch /tmp/flag.reboot
fi

if [ "$smx_wdslist" != "$web_wdslist" ]; then
  echo ">>> Smx-Query: wdslist change [$web_wdslist:$smx_wdslist]"
  flashconfig.sh set WdsList "$smx_wdslist"
  touch /tmp/flag.reboot
fi

if [ "$smx_logenabled" != "$web_logenabled" ]; then
  echo ">>> Smx-Query: logenabled change [$web_logenabled:$smx_logenabled]"
  flashconfig.sh set smx_logenabled "$smx_logenabled"
  if [ "$smx_logenabled" == 'N' ]; then
    rm -f /tmp/flag.syslog
  else
    touch /tmp/flag.syslog
  fi
fi

if [ "$smx_webaccess" != "$web_webaccess" ]; then
  echo ">>> Smx-Query: webaccess change [$web_webaccess:$smx_webaccess]"
  flashconfig.sh set smx_webaccess "$smx_webaccess"
fi

if [ "$smx_vlanid_ssid1" != "$web_vlanid_ssid1" ]; then
  echo ">>> Smx-Query: ssid1 vlan id change [$web_vlanid_ssid1:$smx_vlanid_ssid1]"
  flashconfig.sh set smx_vlanid_ssid1 "$smx_vlanid_ssid1"
  touch /tmp/flag.reboot
fi

if [ "$smx_vlanid_ssid2" != "$web_vlanid_ssid2" ]; then
  echo ">>> Smx-Query: ssid2 vlan id change [$web_vlanid_ssid2:$smx_vlanid_ssid2]"
  flashconfig.sh set smx_vlanid_ssid2 "$smx_vlanid_ssid2"
  touch /tmp/flag.reboot
fi

if [ "$smx_vlanid_unit" != "$web_vlanid_unit" ]; then
  echo ">>> Smx-Query: unit vlan id change [$web_vlanid_unit:$smx_vlanid_unit]"
  flashconfig.sh set smx_vlanid_unit "$smx_vlanid_unit"
  touch /tmp/flag.reboot
fi


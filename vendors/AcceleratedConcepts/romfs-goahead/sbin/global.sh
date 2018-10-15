#!/bin/sh

ra0=`flashconfig.sh get ra0`
ra1=`flashconfig.sh get ra1`
ra2=`flashconfig.sh get ra2`
ra3=`flashconfig.sh get ra3`
ra4=`flashconfig.sh get ra4`
ra5=`flashconfig.sh get ra5`
ra6=`flashconfig.sh get ra6`
ra7=`flashconfig.sh get ra7`
eth0=`flashconfig.sh get eth0`
eth1=`flashconfig.sh get eth1`
eth2=`flashconfig.sh get eth2`


. /sbin/config.sh


# WAN interface name -> $wan_if
getWanIfName()
{
	wan_mode=`flashconfig.sh get wanConnectionMode`
	if [ "$opmode" = "0" ]; then
		wan_if="br0"
	elif [ "$opmode" = "1" ]; then
		if [ "$CONFIG_RAETH_ROUTER" = "y" -o "$CONFIG_MAC_TO_MAC_MODE" = "y" -o "$CONFIG_RT_3052_ESW" = "y" ]; then
			wan_if="${eth2}.2"
		else
			wan_if="${eth2}"
		fi
	elif [ "$opmode" = "2" ]; then
		wan_if="${ra0}"
	elif [ "$opmode" = "3" ]; then
		wan_if="apcli0"
	fi
#Tom add 3G 04-29-2009 begin 
#	if [ "$wan_mode" = "PPPOE" -o  "$wan_mode" = "L2TP" -o "$wan_mode" = "PPTP" ]; then
	if [ "$wan_mode" = "PPPOE" -o  "$wan_mode" = "L2TP" -o "$wan_mode" = "PPTP" -o "$wanmode" = "G3G" ]; then
#Tom add 3G 04-29-2009 end
		wan_ppp_if="ppp0"
	else
		wan_ppp_if=$wan_if
	fi
}

# LAN interface name -> $lan_if
getLanIfName()
{
	bssidnum=`flashconfig.sh get BssidNum`
  
	if [ "$opmode" = "0" ]; then
		lan_if="br0"
	elif [ "$opmode" = "1" ]; then
		if [ "$CONFIG_RAETH_ROUTER" = "y" -o "$CONFIG_MAC_TO_MAC_MODE" = "y" -o "$CONFIG_RT_3052_ESW" = "y" ]; then
			lan_if="br0"
		elif [ "$CONFIG_ICPLUS_PHY" = "y" ]; then 
			if [ "$CONFIG_RT2860V2_AP_MBSS" = "y" -a "$bssidnum" != "1" ]; then
				lan_if="br0"
			else
				lan_if="${ra0}"
			fi
		else
			lan_if="${ra0}"
		fi
	elif [ "$opmode" = "2" ]; then
		lan_if="${eth2}"
	elif [ "$opmode" = "3" ]; then
		lan_if="br0"
	fi

  # Accelecon NetReach vlan feature - if vlan is enabled, use vlanid_unit
  # ...but if it's the same as ssid1, then we can't use vlanid_unit because
  # ...part of a bridge, so use that bridge instead.
  vlanid_unit=$(flashconfig.sh get smx_vlanid_unit)
  vlanid_ssid1=$(flashconfig.sh get smx_vlanid_ssid1)
  if [ "$vlanid_unit" ]; then
    lan_if="br0"
    flashconfig.sh set vlanunit_interface "$lan_if"
  fi
}

# ethernet converter enabled -> $ethconv "y"
getEthConv()
{
	ec=`flashconfig.sh get ethConvert`
	if [ "$opmode" = "0" -a "$CONFIG_RT2860V2_STA_DPB" = "y" -a "$ec" = "1" ]; then
		ethconv="y"
	else
		ethconv="n"
	fi
}

# station driver loaded -> $stamode "y"
getStaMode()
{
	if [ "$opmode" = "2" -o "$ethconv" = "y" ]; then
		stamode="y"
	else
		stamode="n"
	fi
}

opmode=`flashconfig.sh get OperationMode`
[ "$opmode" ] || opmode=0
wanmode=`flashconfig.sh get wanConnectionMode`
ethconv="n"
stamode="n"
wan_if="br0"
wan_ppp_if="br0"
lan_if="br0"
getWanIfName
getLanIfName
getEthConv
getStaMode

# debug
#echo "opmode=$opmode"
#echo "wanmode=$wanmode"
#echo "ethconv=$ethconv"
#echo "stamode=$stamode"
#echo "wan_if=$wan_if"
#echo "lan_if=$lan_if"


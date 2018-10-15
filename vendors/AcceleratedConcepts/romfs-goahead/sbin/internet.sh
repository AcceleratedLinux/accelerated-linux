#!/bin/sh
#
# $Id: internet.sh,v 1.71 2009-02-25 01:31:35 steven Exp $
#
# usage: internet.sh
#

. /sbin/config.sh
. /sbin/global.sh

# Jason, 20090726
Module_type_3g="SMCWBR14S-3GN"
Module_type="SMCWBR14S-N3"

lan_ip=`flashconfig.sh get lan_ipaddr`
stp_en=`flashconfig.sh get stpEnabled`
nat_en=`flashconfig.sh get natEnabled`
bssidnum=`flashconfig.sh get BssidNum`
radio_off=`flashconfig.sh get RadioOff`
#Tom get wan mode 04-29-2009 begin
wanmode=`flashconfig.sh get wanConnectionMode`
#Tom get wan mode 04-29-2009 end

wan_ip_type=`flashconfig.sh get wan_ip_assignment`

# Minglin, 20091202 
mkdir /var/run 2> /dev/null

# Jason, 20090726
#Platform=`flashconfig.sh get Platform`
Platform="SMCWBR14S-3GN"

#Alwin, 20091014
disable_3G=`flashconfig.sh get wan2`
#end


set_vlan_map()
{
	if [ "$CONFIG_RAETH_QOS_PORT_BASED" = "y" ]; then
	# vlan priority tag => skb->priority mapping
	vconfig set_ingress_map $1 0 0
	vconfig set_ingress_map $1 1 1
	vconfig set_ingress_map $1 2 2
	vconfig set_ingress_map $1 3 3
	vconfig set_ingress_map $1 4 4
	vconfig set_ingress_map $1 5 5
	vconfig set_ingress_map $1 6 6
	vconfig set_ingress_map $1 7 7

	# skb->priority => vlan priority tag mapping
	vconfig set_egress_map $1 0 0
	vconfig set_egress_map $1 1 1
	vconfig set_egress_map $1 2 2
	vconfig set_egress_map $1 3 3
	vconfig set_egress_map $1 4 4
	vconfig set_egress_map $1 5 5
	vconfig set_egress_map $1 6 6
	vconfig set_egress_map $1 7 7
	fi
}

ifRaxWdsxDown()
{
	ifconfig ${ra0} down
	ifconfig ${ra1} down
	ifconfig ${ra2} down
	ifconfig ${ra3} down
	ifconfig ${ra4} down
	ifconfig ${ra5} down
	ifconfig ${ra6} down
	ifconfig ${ra7} down

	ifconfig wds0 down
	ifconfig wds1 down
	ifconfig wds2 down
	ifconfig wds3 down

	ifconfig apcli0 down

	ifconfig mesh0 down
}

addBr0()
{
	brctl addbr br0
	brctl addif br0 ${ra0}
}

addRax2Br0()
{
  if [ $vlanid_unit ]; then
    #This is for the Accelecon NetReach product. It supports two ssids with vlan tagging on each.
    #take the wifi interfaces out of the default bridge
    brctl delif br0 ${ra1}
    brctl delif br0 ${ra0}
    brctl delif br0 ${eth2}
    #create two new bridges
    brctl addbr br1
    brctl addbr br2
    #create vlan interfaces
    vconfig add ${eth2} $vlanid_ssid1
    vconfig add ${eth2} $vlanid_ssid2
    #add vlan interfaces to bridges, wifi interfaces to bridges, and bring up interfaces
    # NOTE: due to the goahead script, if br0 is not tied to an eth port
    # the web GUI does not correctly obtain network info and the MAC
    # address of the NetReach unit
    if [ "$vlanid_unit" != "$vlanid_ssid1" ]; then
      brctl addif br1 ${eth2}.$vlanid_ssid1
      brctl addif br1 ${ra0}
    else
      brctl addif br0 ${eth2}.$vlanid_ssid1
      brctl addif br0 ${ra0}
    fi
    brctl addif br2 ${eth2}.$vlanid_ssid2
    brctl addif br2 ${ra1}
    ip link set br1 up
    ip link set br2 up
    ip link set ${eth2}.$vlanid_ssid1 up
    ip link set ${eth2}.$vlanid_ssid2 up 
    return
  fi
	if [ "$bssidnum" = "2" ]; then
		brctl addif br0 ${ra1}
	elif [ "$bssidnum" = "3" ]; then
		brctl addif br0 ${ra1}
		brctl addif br0 ${ra2}
	elif [ "$bssidnum" = "4" ]; then
		brctl addif br0 ${ra1}
		brctl addif br0 ${ra2}
		brctl addif br0 ${ra3}
	elif [ "$bssidnum" = "5" ]; then
		brctl addif br0 ${ra1}
		brctl addif br0 ${ra2}
		brctl addif br0 ${ra3}
		brctl addif br0 ${ra4}
	elif [ "$bssidnum" = "6" ]; then
		brctl addif br0 ${ra1}
		brctl addif br0 ${ra2}
		brctl addif br0 ${ra3}
		brctl addif br0 ${ra4}
		brctl addif br0 ${ra5}
	elif [ "$bssidnum" = "7" ]; then
		brctl addif br0 ${ra1}
		brctl addif br0 ${ra2}
		brctl addif br0 ${ra3}
		brctl addif br0 ${ra4}
		brctl addif br0 ${ra5}
		brctl addif br0 ${ra6}
	elif [ "$bssidnum" = "8" ]; then
		brctl addif br0 ${ra1}
		brctl addif br0 ${ra2}
		brctl addif br0 ${ra3}
		brctl addif br0 ${ra4}
		brctl addif br0 ${ra5}
		brctl addif br0 ${ra6}
		brctl addif br0 ${ra7}
	fi
}

addWds2Br0()
{
	wds_en=`flashconfig.sh get WdsEnable`
	if [ "$wds_en" != "0" ]; then
		ifconfig wds0 up
		ifconfig wds1 up
		ifconfig wds2 up
		ifconfig wds3 up
		brctl addif br0 wds0
		brctl addif br0 wds1
		brctl addif br0 wds2
		brctl addif br0 wds3
	fi
}

addMesh2Br0()
{
	meshenabled=`flashconfig.sh get MeshEnabled`
	if [ "$meshenabled" = "1" ]; then
		ifconfig mesh0 up
		brctl addif br0 mesh0
		meshhostname=`flashconfig.sh get MeshHostName`
		iwpriv mesh0 set  MeshHostName="$meshhostname"
	fi
}

addRaix2Br0()
{
  if [ $vlanid_unit ]; then
    #This is for the Accelecon NetReach product. It supports two ssids with vlan tagging on each.
    #take the wifi interfaces out of the default bridge
    brctl delif br0 ${ra1}
    brctl delif br0 ${ra0}
    brctl delif br0 ${eth2}
    #create two new bridges
    brctl addbr br1
    brctl addbr br2
    #create vlan interfaces
    vconfig add ${eth2} $vlanid_ssid1
    vconfig add ${eth2} $vlanid_ssid2
    #add vlan interfaces to bridges, wifi interfaces to bridges, and bring up interfaces
    # NOTE: due to the goahead script, if br0 is not tied to an eth port
    # the web GUI does not correctly obtain network info and the MAC
    # address of the NetReach unit
    if [ "$vlanid_unit" != "$vlanid_ssid1" ]; then
      brctl addif br1 ${eth2}.$vlanid_ssid1
      brctl addif br1 ${ra0}
    else
      brctl addif br0 ${eth2}.$vlanid_ssid1
      brctl addif br0 ${ra0}
    fi
    brctl addif br2 ${eth2}.$vlanid_ssid2
    brctl addif br2 ${ra1}
    ip link set br1 up
    ip link set br2 up
    ip link set ${eth2}.$vlanid_ssid1 up
    ip link set ${eth2}.$vlanid_ssid2 up 
    return
  fi
	inic_bssnum=`nvram_get inic BssidNum`
	if [ "$CONFIG_RT2880_INIC" == "" -a "$CONFIG_INIC_MII" == "" -a "$CONFIG_INIC_PCI" == "" -a "$CONFIG_INIC_USB" == "" ]; then
		return
	fi
	brctl addif br0 rai0

	if [ "$inic_bssnum" = "2" ]; then
		ifconfig rai1 up
		brctl addif br0 rai1
	elif [ "$inic_bssnum" = "3" ]; then
		ifconfig rai1 up
		ifconfig rai2 up
		brctl addif br0 rai1
		brctl addif br0 rai2
	elif [ "$inic_bssnum" = "4" ]; then
		ifconfig rai1 up
		ifconfig rai2 up
		ifconfig rai3 up
		brctl addif br0 rai1
		brctl addif br0 rai2
		brctl addif br0 rai3
	elif [ "$inic_bssnum" = "5" ]; then
		ifconfig rai1 up
		ifconfig rai2 up
		ifconfig rai3 up
		ifconfig rai4 up
		brctl addif br0 rai1
		brctl addif br0 rai2
		brctl addif br0 rai3
		brctl addif br0 rai4
	elif [ "$inic_bssnum" = "6" ]; then
		ifconfig rai1 up
		ifconfig rai2 up
		ifconfig rai3 up
		ifconfig rai4 up
		ifconfig rai5 up
		brctl addif br0 rai1
		brctl addif br0 rai2
		brctl addif br0 rai3
		brctl addif br0 rai4
		brctl addif br0 rai5
	elif [ "$inic_bssnum" = "7" ]; then
		ifconfig rai1 up
		ifconfig rai2 up
		ifconfig rai3 up
		ifconfig rai4 up
		ifconfig rai5 up
		ifconfig rai6 up
		brctl addif br0 rai1
		brctl addif br0 rai2
		brctl addif br0 rai3
		brctl addif br0 rai4
		brctl addif br0 rai5
		brctl addif br0 rai6
	elif [ "$inic_bssnum" = "8" ]; then
		ifconfig rai1 up
		ifconfig rai2 up
		ifconfig rai3 up
		ifconfig rai4 up
		ifconfig rai5 up
		ifconfig rai6 up
		ifconfig rai7 up
		brctl addif br0 rai1
		brctl addif br0 rai2
		brctl addif br0 rai3
		brctl addif br0 rai4
		brctl addif br0 rai5
		brctl addif br0 rai6
		brctl addif br0 rai7
	fi
}

addInicWds2Br0()
{
	if [ "$CONFIG_RT2880_INIC" == "" -a "$CONFIG_INIC_MII" == "" -a "$CONFIG_INIC_PCI" == "" -a "$CONFIG_INIC_USB" == "" ]; then
		return
	fi
	wds_en=`nvram_get inic WdsEnable`
	if [ "$wds_en" != "0" ]; then
		ifconfig wdsi0 up
		ifconfig wdsi1 up
		ifconfig wdsi2 up
		ifconfig wdsi3 up
		brctl addif br0 wdsi0
		brctl addif br0 wdsi1
		brctl addif br0 wdsi2
		brctl addif br0 wdsi3
	fi
}

addRaL02Br0()
{
	if [ "$CONFIG_RT2561_AP" != "" ]; then
		brctl addif br0 raL0
	fi
}

genSysFiles()
{
	login=`flashconfig.sh get Login`
	pass=`flashconfig.sh get Password`
	if [ "$login" != "" -a "$pass" != "" ]; then
		echo "$login::0:0:Adminstrator:/:/bin/sh" >> /etc/passwd
		echo "$login:x:0:$login" >> /etc/group
		chpasswd.sh $login $pass
	fi
	if [ "$CONFIG_PPPOL2TP" == "y" ]; then
		echo "l2tp 1701/tcp l2f" >> /etc/services
		echo "l2tp 1701/udp l2f" >> /etc/services
	fi
}

genDevNode()
{
#Linux2.6 uses udev instead of devfs, we have to create static dev node by myself.
if [ "$CONFIG_DWC_OTG" == "m" -a "$CONFIG_HOTPLUG" == "y" ]; then
	mounted=`mount | grep mdev | wc -l`
	if [ $mounted -eq 0 ]; then
	mount -t ramfs mdev /dev
	mkdir /dev/pts
	mount -t devpts devpts /dev/pts
        mdev -s

        mknod   /dev/video0      c       81      0
        mknod   /dev/spiS0       c       217     0
        mknod   /dev/i2cM0       c       218     0
        mknod   /dev/rdm0        c       254     0
        mknod   /dev/flash0      c       200     0
        mknod   /dev/swnat0      c       210     0
        mknod   /dev/hwnat0      c       220     0
        mknod   /dev/acl0        c       230     0
        mknod   /dev/ac0         c       240     0
        mknod   /dev/mtr0        c       250     0
        mknod   /dev/gpio        c       252     0
	mknod   /dev/PCM         c       233     0
	mknod   /dev/I2S         c       234     0
	fi

	echo "# <device regex> <uid>:<gid> <octal permissions> [<@|$|*> <command>]" > /etc/mdev.conf
        echo "# The special characters have the meaning:" >> /etc/mdev.conf
        echo "# @ Run after creating the device." >> /etc/mdev.conf
        echo "# $ Run before removing the device." >> /etc/mdev.conf
        echo "# * Run both after creating and before removing the device." >> /etc/mdev.conf
        echo "sd[a-z][1-9] 0:0 0660 */sbin/automount.sh \$MDEV" >> /etc/mdev.conf

        #enable usb hot-plug feature
        echo "/sbin/mdev" > /proc/sys/kernel/hotplug

fi
}

# Jason, 20090726
rcfile(){
 WAN_RCFile=/var/run/rc.conf
 
 OperationMode=`flashconfig.sh get OperationMode`
 if [ "$OperationMode" = "1" ]; then
    #This is Router mode
    wan_bridge_enable=0
    wan_ip_assignment=`flashconfig.sh get wan_ip_assignment`
    wan_dual_wan_master=`flashconfig.sh get wan_dual_wan_master` 

    wan_dual_wan_backup=`flashconfig.sh get wan_dual_wan_backup`
    wan_dual_wan_fallback_enable=`flashconfig.sh get wan_dual_wan_fallback_enable` 

    wan_dual_wan_detect_ip=`flashconfig.sh get wan_dual_wan_detect_ip`
    wan_dual_backup_wan_detect_ip=`flashconfig.sh get wan_dual_backup_wan_detect_ip`
    wan_dual_wan_detect_timeout=`flashconfig.sh get wan_dual_wan_detect_timeout` 
 else 
    #This is Bridge mode
    wan_bridge_enable=1 
    wan_ip_assignment=99
    wan_dual_wan_master=99 

    wan_dual_wan_backup=0
    wan_dual_wan_fallback_enable=0 
    wan_dual_wan_detect_ip==`flashconfig.sh get wan_dual_wan_detect_ip`
    wan_dual_backup_wan_detect_ip=`flashconfig.sh get wan_dual_backup_wan_detect_ip`
    wan_dual_wan_detect_timeout=`flashconfig.sh get wan_dual_wan_detect_timeout` 
 fi 
  
 rm -rf $WAN_RCFile 2>&1 >/dev/null
 echo "wan_ip_assignment=$wan_ip_assignment" > $WAN_RCFile
 echo "wan_dual_wan_master=$wan_dual_wan_master" >> $WAN_RCFile

 echo "wan_dual_wan_backup=$wan_dual_wan_backup" >> $WAN_RCFile
 echo "wan_dual_wan_fallback_enable=$wan_dual_wan_fallback_enable" >> $WAN_RCFile

 echo "wan_dual_wan_detect_ip=$wan_dual_wan_detect_ip" >> $WAN_RCFile
 echo "wan_dual_backup_wan_detect_ip=$wan_dual_backup_wan_detect_ip" >> $WAN_RCFile

 echo "wan_dual_wan_detect_timeout=$wan_dual_wan_detect_timeout" >> $WAN_RCFile
 echo "wan_bridge_enable=$wan_bridge_enable" >> $WAN_RCFile 


#  # Nothing to do with RF file,but create the lb.status if dual wan enable.
if [ "$OperationMode" = "1" ]; then
 [ ! $wan_dual_wan_backup = "99" ] && echo $wan_ip_assignment > /var/run/lb.status
else
    rm -rf /var/run/lb.status
fi
}

# opmode adjustment:
#   if AP client was not compiled and operation mode was set "3" -> set $opmode "1"
#   if Station was not compiled and operation mode was set "2" -> set $opmode "1"
if [ "$opmode" = "3" -a "$CONFIG_RT2860V2_AP_APCLI" != "y" ]; then
	flashconfig.sh set OperationMode 1
	opmode="1"
fi
if [ "$opmode" = "2" -a "$CONFIG_RT2860V2_STA" == "" ]; then
	flashconfig.sh set OperationMode 1
	opmode="1"
fi

genSysFiles
#genDevNode
# chukuo: experiment, mdev seems useless. so comment it out.
if [ "$CONFIG_DWC_OTG" == "m" ]; then
usbmod_exist=`mount | grep dwc_otg | wc -l`

if [ $usbmod_exist == 0 ]; then
  #Tom comment out 04-29-2009
  #modprobe -q lm
  #modprobe -q dwc_otg
  #Tom setup DWC OTG 04-29-2009 begin
  #echo "do nothing!!!" > /dev/console
  #Tom setup DWC OTG 04-29-2009 end
  :
fi
fi

# insmod all
modprobe -q bridge
#modprobe -q mii
#modprobe -q raeth
ifconfig ${eth2} 0.0.0.0

#ifRaxWdsxDown
#rmmod rt2860v2_ap
#rmmod rt2860v2_sta
#ralink_init make_wireless_config rt2860
#if [ "$stamode" = "y" ]; then
#	modprobe -q rt2860v2_sta
#else
#	if [ "$CONFIG_RT2860V2_AP_DFS" = "y" ]; then
#		modprobe -q rt_timer
#	fi
#	modprobe -q rt2860v2_ap
#fi
vpn-passthru.sh

# INIC support
if [ "$CONFIG_INIC_MII" != "" -o "$CONFIG_INIC_PCI" != ""  -o "$CONFIG_INIC_USB" != "" ]; then
        iNIC_Mii_en=`nvram_get inic InicMiiEnable`
        iNIC_USB_en=`nvram_get inic InicUSBEnable`
        ifconfig rai0 down
        ralink_init make_wireless_config inic
if [ "$iNIC_USB_en" == "1" ]; then
        rmmod iNIC_usb
        modprobe -q iNIC_usb 
        ifconfig rai0 up
elif [ "$CONFIG_INIC_PCI" != "" ]; then
        rmmod iNIC_pci
        modprobe -q iNIC_pci 
        ifconfig rai0 up
fi
fi

# RT2561(Legacy) support
#if [ "$CONFIG_RT2561_AP" != "" ]; then
#	ifconfig raL0 down
#	rmmod rt2561ap
#	ralink_init make_wireless_config rt2561
#	modprobe -q rt2561ap
#	ifconfig raL0 up
#fi

# configure and start hostapd
hostapd-config.sh

# config interface
ifconfig ${ra0} 0.0.0.0
if [ "$ethconv" = "y" ]; then
	iwpriv ${ra0} set EthConvertMode=dongle
fi
if [ "$radio_off" = "1" ]; then
	iwpriv ${ra0} set RadioOn=0
fi
if [ "$bssidnum" = "2" ]; then
	ifconfig ${ra1} 0.0.0.0
elif [ "$bssidnum" = "3" ]; then
	ifconfig ${ra1} 0.0.0.0
	ifconfig ${ra2} 0.0.0.0
elif [ "$bssidnum" = "4" ]; then
	ifconfig ${ra1} 0.0.0.0
	ifconfig ${ra2} 0.0.0.0
	ifconfig ${ra3} 0.0.0.0
elif [ "$bssidnum" = "5" ]; then
	ifconfig ${ra1} 0.0.0.0
	ifconfig ${ra2} 0.0.0.0
	ifconfig ${ra3} 0.0.0.0
	ifconfig ${ra4} 0.0.0.0
elif [ "$bssidnum" = "6" ]; then
	ifconfig ${ra1} 0.0.0.0
	ifconfig ${ra2} 0.0.0.0
	ifconfig ${ra3} 0.0.0.0
	ifconfig ${ra4} 0.0.0.0
	ifconfig ${ra5} 0.0.0.0
elif [ "$bssidnum" = "7" ]; then
	ifconfig ${ra1} 0.0.0.0
	ifconfig ${ra2} 0.0.0.0
	ifconfig ${ra3} 0.0.0.0
	ifconfig ${ra4} 0.0.0.0
	ifconfig ${ra5} 0.0.0.0
	ifconfig ${ra6} 0.0.0.0
elif [ "$bssidnum" = "8" ]; then
	ifconfig ${ra1} 0.0.0.0
	ifconfig ${ra2} 0.0.0.0
	ifconfig ${ra3} 0.0.0.0
	ifconfig ${ra4} 0.0.0.0
	ifconfig ${ra5} 0.0.0.0
	ifconfig ${ra6} 0.0.0.0
	ifconfig ${ra7} 0.0.0.0
fi
if [ "$CONFIG_RAETH_ROUTER" = "y" -o "$CONFIG_MAC_TO_MAC_MODE" = "y" -o "$CONFIG_RT_3052_ESW" = "y" ]; then
	modprobe -q 8021q
	vconfig add ${eth2} 1
	set_vlan_map ${eth2}.1
	vconfig add ${eth2} 2
	set_vlan_map ${eth2}.2
	ifconfig ${eth2}.2 down
	wan_mac=`flashconfig.sh get WAN_MAC_ADDR`
	if [ "$wan_mac" != "FF:FF:FF:FF:FF:FF" ]; then
	ifconfig ${eth2}.2 hw ether $wan_mac
	fi
#START: james add mac clone
	clone_en=`flashconfig.sh get macCloneEnabled`
	clone_mac=`flashconfig.sh get macCloneMac`
	if [ "$opmode" != "0" -a "$clone_en" = "1" ]; then
		[ ! "`ifconfig $wan_if 2>&1 >/dev/null`" ] && ifconfig $wan_if hw ether $clone_mac
	fi
#END: james add mac clone
	ifconfig ${eth2}.1 0.0.0.0
	ifconfig ${eth2}.2 0.0.0.0
elif [ "$CONFIG_ICPLUS_PHY" = "y" ]; then
	#remove ip alias
	# it seems busybox has no command to remove ip alias...
	ifconfig ${eth2}:1 0.0.0.0 1>&2 2>/dev/null
fi

ifconfig lo 127.0.0.1
ifconfig br0 down
brctl delbr br0

# stop all
iptables --flush
iptables --flush -t nat
iptables --flush -t mangle

#load accelecon config
vlanid_unit=` flashconfig.sh get smx_vlanid_unit`
vlanid_ssid1=`flashconfig.sh get smx_vlanid_ssid1`
vlanid_ssid2=`flashconfig.sh get smx_vlanid_ssid2`

#
# init ip address to all interfaces for different OperationMode:
#   0 = Bridge Mode
#   1 = Gateway Mode
#   2 = Ethernet Converter Mode
#   3 = AP Client
#
#-JCYU Add for dual-wan stop for bridge --10082009
if [ "$Platform" = "$Module_type_3g" ]; then
    rcfile
fi
#-JCYU Add for dual-wan stop for bridge --10082009
if [ "$opmode" = "0" ]; then
	addBr0
	if [ "$CONFIG_RAETH_ROUTER" = "y" -a "$CONFIG_LAN_WAN_SUPPORT" = "y" ]; then
		echo "##### restore IC+ to dump switch #####"
		config-vlan.sh 0 0
	elif [ "$CONFIG_MAC_TO_MAC_MODE" = "y" ]; then
		echo "##### restore Vtss to dump switch #####"
		config-vlan.sh 1 0
	elif [ "$CONFIG_RT_3052_ESW" = "y" ]; then
		if [ "$CONFIG_P5_RGMII_TO_MAC_MODE" = "y" ]; then
			echo "##### restore RT3052 to dump switch #####"
			config-vlan.sh 2 0
			echo "##### restore Vtss to dump switch #####"
			config-vlan.sh 1 0
		else
			echo "##### restore RT3052 to dump switch #####"
			config-vlan.sh 2 0
		fi
	fi
  if [ "$vlanid_unit" != "$vlanid_ssid1" ]; then
    vconfig add ${eth2} $vlanid_unit
    brctl addif br0 ${eth2}.$vlanid_unit
    ip link set ${eth2}.$vlanid_unit up
    ip link set br0 up
  fi
  if [ ! "$vlanid_unit" ]; then
    brctl addif br0 ${eth2}
  fi
	if [ "$CONFIG_RT2860V2_AP_MBSS" = "y" -a "$bssidnum" != "1" ]; then
		addRax2Br0
	fi

	#start mii iNIC after network interface is working
	iNIC_Mii_en=`nvram_get inic InicMiiEnable`
	if [ "$iNIC_Mii_en" == "1" ]; then
	     ifconfig rai0 down
	     rmmod iNIC_mii
	     modprobe -q iNIC_mii miimaster=${eth2}
	     ifconfig rai0 up
	fi

	addWds2Br0
	addMesh2Br0
	addRaix2Br0
	addInicWds2Br0
	addRaL02Br0
	wan.sh
	lan.sh
	echo 0 > /proc/sys/net/ipv4/ip_forward
elif [ "$opmode" = "1" ]; then
	if [ "$CONFIG_RAETH_ROUTER" = "y" -o "$CONFIG_MAC_TO_MAC_MODE" = "y" -o "$CONFIG_RT_3052_ESW" = "y" ]; then
		if [ "$CONFIG_RAETH_ROUTER" = "y" -a "$CONFIG_LAN_WAN_SUPPORT" = "y" ]; then
			if [ "$CONFIG_WAN_AT_P0" = "y" ]; then
				echo '##### config IC+ vlan partition (WLLLL) #####'
				config-vlan.sh 0 WLLLL
			else
				echo '##### config IC+ vlan partition (LLLLW) #####'
				config-vlan.sh 0 LLLLW
			fi
		fi
		if [ "$CONFIG_MAC_TO_MAC_MODE" = "y" ]; then
			echo '##### config Vtss vlan partition #####'
			config-vlan.sh 1 1
		fi
		if [ "$CONFIG_RT_3052_ESW" = "y" -a "$CONFIG_LAN_WAN_SUPPORT" = "y" ]; then
			if [ "$CONFIG_P5_RGMII_TO_MAC_MODE" = "y" ]; then
				echo "##### restore RT3052 to dump switch #####"
				config-vlan.sh 2 0
				echo "##### config Vtss vlan partition #####"
				config-vlan.sh 1 1
			else
				if [ "$CONFIG_WAN_AT_P0" = "y" ]; then
					echo '##### config RT3052 vlan partition (WLLLL) #####'
					config-vlan.sh 2 WLLLL
				else
					echo '##### config RT3052 vlan partition (LLLLW) #####'
					config-vlan.sh 2 LLLLW
				fi
			fi
		fi
		addBr0
		brctl addif br0 ${eth2}.1
		if [ "$CONFIG_RT2860V2_AP_MBSS" = "y" -a "$bssidnum" != "1" ]; then
			addRax2Br0
		fi
		addWds2Br0
		addMesh2Br0
		addRaix2Br0
		addInicWds2Br0
		addRaL02Br0
	fi

	# IC+ 100 PHY (one port only)
	if [ "$CONFIG_ICPLUS_PHY" = "y" ]; then
		echo '##### connected to one port 100 PHY #####'
		if [ "$CONFIG_RT2860V2_AP_MBSS" = "y" -a "$bssidnum" != "1" ]; then
			addBr0
			addRax2Br0
		fi
		addWds2Br0
		addMesh2Br0

		#
		# setup ip alias for user to access web page.
		#
		ifconfig ${eth2}:1 172.32.1.254 netmask 255.255.255.0 up
	fi
# Jason, 20090726
	if [ "$Platform" = "$Module_type_3g" ]; then
	      #rcfile #JCYU-remove it, and run it before here 
	      lan.sh PRE
	      echo "=====  intenet.sh start=========="

	      #james: JC changes it to background, but it will cause PPTP failure
              #james: So I remove it again. MUST pay attention to this later.
	     wan_dual.sh restart

             # lan.sh POST
             # nat.sh
	      echo "=====  intenet.sh end=========="                
#
#              #Alwin,20091014, fix MAC clone bug, 3G mode don't support clone
#              #if [ "$disable_3G" = "99" ]; then
#	      if [ "$wan_ip_type" = "2" ]; then
#	lan.sh
#		nat.sh
#	      elif [ "$disable_3G" = "99" -a "$wan_ip_type" != "2" ]; then
#		wan.sh
#		lan.sh
#		nat.sh 
#	      fi
#              #end 

	else
	      wan.sh
	      lan.sh
	      nat.sh
        fi
# end
elif [ "$opmode" = "2" ]; then
	# if (-1 == initStaProfile())
	#   error(E_L, E_LOG, T("internet.c: profiles in nvram is broken"));
	# else
	#   initStaConnection();
	if [ "$CONFIG_RAETH_ROUTER" = "y" -a "$CONFIG_LAN_WAN_SUPPORT" = "y" ]; then
		echo "##### restore IC+ to dump switch #####"
		config-vlan.sh 0 0
	fi
	if [ "$CONFIG_MAC_TO_MAC_MODE" = "y" ]; then
		echo "##### restore Vtss to dump switch #####"
		config-vlan.sh 1 0
	fi
	if [ "$CONFIG_RT_3052_ESW" = "y" ]; then
		if [ "$CONFIG_P5_RGMII_TO_MAC_MODE" = "y" ]; then
			echo "##### restore RT3052 to dump switch #####"
			config-vlan.sh 2 0
			echo "##### restore Vtss to dump switch #####"
			config-vlan.sh 1 0
		else
			echo "##### restore RT3052 to dump switch #####"
			config-vlan.sh 2 0
		fi
	fi
	wan.sh
	lan.sh
	nat.sh
elif [ "$opmode" = "3" ]; then
	if [ "$CONFIG_RAETH_ROUTER" = "y" -o "$CONFIG_MAC_TO_MAC_MODE" = "y" -o "$CONFIG_RT_3052_ESW" = "y" ]; then
		if [ "$CONFIG_RAETH_ROUTER" = "y" ]; then
			echo "##### restore IC+ to dump switch #####"
			config-vlan.sh 0 0
		fi
		if [ "$CONFIG_MAC_TO_MAC_MODE" = "y" ]; then
			echo "##### restore Vtss to dump switch #####"
			config-vlan.sh 1 0
		fi
		if [ "$CONFIG_RT_3052_ESW" = "y" ]; then
			if [ "$CONFIG_P5_RGMII_TO_MAC_MODE" = "y" ]; then
				echo "##### restore RT3052 to dump switch #####"
				config-vlan.sh 2 0
				echo "##### restore Vtss to dump switch #####"
				config-vlan.sh 1 0
			else
				echo "##### restore RT3052 to dump switch #####"
				config-vlan.sh 2 0
			fi
		fi
		addBr0
    if [ "$vlanid_unit" != "$vlanid_ssid1" ]; then
      vconfig add ${eth2} $vlanid_unit
      brctl addif br0 ${eth2}.$vlanid_unit
      ip link set ${eth2}.$vlanid_unit up
      ip link set br0 up
    fi
    if [ ! "$vlanid_unit" ]; then
      brctl addif br0 ${eth2}
    fi
	fi
	wan.sh
	lan.sh
	nat.sh
else
	echo "unknown OperationMode: $opmode"
	exit 1
fi

# INIC support
if [ "$CONFIG_RT2880_INIC" != "" ]; then
	ifconfig rai0 down
	rmmod rt_pci_dev
	ralink_init make_wireless_config inic
	modprobe -q rt_pci_dev
	ifconfig rai0 up
	RaAP&
	sleep 3
fi

# in order to use broadcast IP address in L2 management daemon
if [ "$CONFIG_ICPLUS_PHY" = "y" ]; then
	route add -host 255.255.255.255 dev $wan_if
else
	route add -host 255.255.255.255 dev $lan_if
fi


m2uenabled=`flashconfig.sh get M2UEnabled`
if [ "$m2uenabled" = "1" ]; then
	iwpriv ${ra0} set IgmpSnEnable=1
	echo "iwpriv ${ra0} set IgmpSnEnable=1"
fi
#restart8021XDaemon(RT2860_NVRAM);
#firewall_init();
#management_init();
echo "===== To Fix Coutnry Code does not depend on MFG procedure ===="
# country code, Alwin 20091020
countrycode=`gpio M COUNTRY`
if [ "$countrycode" = "US" ]; then
        flashconfig.sh set CountryCode US
elif [ "$countrycode" = "TW" ]; then
        flashconfig.sh set CountryCode TW
elif [ "$countrycode" = "EU" ]; then
        flashconfig.sh set CountryCode EU
elif [ "$countrycode" = "HK" ]; then
        flashconfig.sh set CountryCode HK
elif [ "$countrycode" = "JP" ]; then
        flashconfig.sh set CountryCode JP
elif [ "$countrycode" = "KR" ]; then
        flashconfig.sh set CountryCode KR
elif [ "$countrycode" = "IR" ]; then
        flashconfig.sh set CountryCode IR
elif [ "$countrycode" = "FR" ]; then
        flashconfig.sh set CountryCode FR
fi

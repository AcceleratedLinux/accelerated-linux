#!/bin/sh
# /sbin/smx-autoconfig.sh

. /sbin/global.sh

echo '>>> Smx-AutoConfig: start'

old_wscssid="`flashconfig.sh get WscSSID`"
old_awb_product_name="`flashconfig.sh get AWB_Product_Name`"
old_hostname="`flashconfig.sh get HostName`"

if [ "$old_wscssid" != 'NetReach' ]; then
  touch /tmp/flag.resetcfg
fi
if [ "$old_awb_product_name" != 'NetReach' ]; then
  touch /tmp/flag.resetcfg
fi
if [ "$old_hostname" != 'NetReach' ]; then
  touch /tmp/flag.resetcfg
fi

# If we need to reset the factory configuration to our override values,
# do so and then reboot.

if [ -f /tmp/flag.resetcfg ]; then
  /sbin/update-reset.sh
fi

# Determine the various mac addresses, the default gateway, and our
# ip address, and update the values in nvram if necessary.  Start by
# getting the current nvram values.

old_defgw="`flashconfig.sh get smx_defgw`"
old_webip="`flashconfig.sh get smx_webip`"

old_lanmac="`flashconfig.sh get smx_lanmac`"
old_wanmac="`flashconfig.sh get smx_wanmac`"
old_attmac="`flashconfig.sh get smx_attmac`"

old_opmode="`flashconfig.sh get OperationMode`"
old_dhcp="`flashconfig.sh get dhcpEnabled`"
old_lanip="`flashconfig.sh get lan_ipaddr`"
old_wangw="`flashconfig.sh get wan_gateway`"
old_wandns1="`flashconfig.sh get wan_primary_dns`"
old_wandns2="`flashconfig.sh get wan_secondary_dns`"

# The eth2.1 port is our lan port, and supports ppoe.  It is where we
# expect the at&t netgate to be connected to provide our default gateway
# and dhcp server.

xlanmac="`ifconfig ${eth2} | fgrep HWaddr | cut -c39-55`"
if [ "$xlanmac" == '' ]; then
  xlanmac="`ifconfig ${eth2}.1 | fgrep HWaddr | cut -c39-55`"
fi
if [ "$xlanmac" == '' ]; then
  xlanmac="`ifconfig $lan_if | fgrep HWaddr | cut -c39-55`"
fi

# The eth2.2 port is our wan port.  We don't use it at the moment except as
# a fallback mac address if we can't resolve our default gateway via arp.

xwanmac="`flashconfig.sh get WAN_MAC_ADDR`"
if [ "$xwanmac" == '' ]; then
  xwanmac="`ifconfig ${eth2}.2 | fgrep HWaddr | cut -c39-55`"
else
  m0=`echo "$xwanmac" | cut -f1 -d':'`
  m1=`echo "$xwanmac" | cut -f2 -d':'`
  m2=`echo "$xwanmac" | cut -f3 -d':'`
  m3=`echo "$xwanmac" | cut -f4 -d':'`
  m4=`echo "$xwanmac" | cut -f5 -d':'`
  m5=`echo "$xwanmac" | cut -f6 -d':'`
  xwanmac="`printf '%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X' 0x$m0 0x$m1 0x$m2 0x$m3 0x$m4 0x$m5 `"
fi

# If the br0 interface is not up, we are in big trouble!  Try to kickstart
# it if it isn't already up.

ifconfig $lan_if up

# Attempt to get an ip address via dhcp, and use it to derive the default
# gateway and its mac address.  Set our default route, name servers and
# restart the web server on the new address.

brctl delif br0 ${ra0}

route del default
route add default dev $lan_if

smx_vlanid_unit=`flashconfig.sh get smx_vlanid_unit`
webip=''
count=0
while [ "$webip" == '' ]; do
  echo 'Attempting to establish network connection via DHCP'
  if pgrep udhcpc > /dev/null; then
    pkill udhcpc
  fi
  udhcpc -i $lan_if -f -b -q -s /sbin/udhcpc.sh 2>/dev/null
  webip=`cat /tmp/smx-dhcp.log | grep 'ip=' | cut -f2 -d'='`
  sleep 5
  count=`expr $count + 1`
  [ $count -gt 10 ] && vlan-discover.sh
done
echo "Network connection established.  IP addr=$webip"

brctl addif br0 ${ra0}

defgw=`cat /tmp/smx-dhcp.log | fgrep 'router=' | cut -f2 -d'='`
wandns1=`cat /tmp/smx-dhcp.log | fgrep 'dns=' | cut -f2 -d'=' | cut -f1 -d' '`
wandns2=`cat /tmp/smx-dhcp.log | fgrep 'dns=' | cut -f2 -d'=' | cut -f2 -d' '`
webix=`cat /tmp/smx-dhcp.log | fgrep 'ip='       | cut -f2 -d'='`

# Trigger an arp table update via an arping.  We hope that the netgate will
# respond so our arp table gets updated with its mac and ip address.

arping -c 1 -I $lan_if "$defgw"

# Examine the resulting arp table for a default gateway ip and extract the
# associated mac address.  The output from arp -n looks like this:
#
#     ? (192.168.1.1) at 00:50:7F:CD:AF:EC [ether]  on br0

xattmac=`arp -n | fgrep "($defgw)" | cut -f4 -d' '`

if [ "$defgw" == '' ]; then
  defgw="10.10.10.1"
  webix="10.10.10.100"
fi
[ "$wandns1" = '' ] && wandns1=$defgw
[ "$wandns2" = '' ] && wandns2=$defgw

route del default
route add "$defgw" dev "$lanif"
route add default gw "$defgw"
# NOTE:  With Marsha's cisco catalyst setup, the "serverid" or "dns" is being used as the 
# default route instead of the "router" IP.  We need to have the NR check to see if "router"
# IP is different than "serverid", and setup DHCP with "router" as the default gateway.
# The default route normally looks like "default via *gwIP* dev br0", which doesn't work with
# her setup.  If I change it to "default via dev br0", then it works fine.  There is some defgw IP issue!

lanmac=`echo $xlanmac | tr -d ':'`
wanmac=`echo $xwanmac | tr -d ':'`
attmac=`echo $xattmac | tr -d ':'`

# Update nvram with the results of our efforts.  If the default
# gateway changes, we need to reboot.  We also need to reboot
# if we have dhcp enabled (after disabling it).

if [ "$old_defgw" != "$defgw" -o "$old_wangw" != "$defgw" ]; then
  flashconfig.sh set smx_defgw "$defgw"
  flashconfig.sh set wan_gateway "$defgw"
  echo "old_defgw='$old_defgw'"
  echo "old_wangw='$old_wangw'"
  echo "defgw='$defgw'"
  touch /tmp/flag.reboot
fi
if [ "$old_dhcp" != '0' ]; then
  echo "old_dhcp='$old_dhcp'"
  flashconfig.sh set dhcpEnabled '0'
  touch /tmp/flag.reboot
fi
if [ "$old_opmode" != '0' -a "$old_opmode" != '2' ]; then
  echo "old_opmode='$old_opmode'.  Defaulting to Bridge mode"
  flashconfig.sh set OperationMode '0'
  touch /tmp/flag.reboot
fi

if [ "$old_wandns1" != "$wandns1" -o "$old_wandns2" != "$wandns2" ]; then
  flashconfig.sh set wan_primary_dns "$wandns1"
  flashconfig.sh set wan_secondary_dns "$wandns2"
fi
if ! cat /etc/resolv.conf | grep "$wandns1" > /dev/null; then
  /sbin/config-dns.sh "$wandns1" "$wandns2"
elif ! cat /etc/resolv.conf | grep "$wandns2" > /dev/null; then
  /sbin/config-dns.sh "$wandns1" "$wandns2"
fi

if [ "$old_webip" != "$webip" ]; then
  flashconfig.sh set smx_webip "$webip"
fi
if [ "$old_lanmac" != "$lanmac" ]; then
  flashconfig.sh set smx_lanmac "$lanmac"
fi
if [ "$old_wanmac" != "$wanmac" ]; then
  flashconfig.sh set smx_wanmac "$wanmac"
fi
if [ "$old_attmac" != "$attmac" ]; then
  flashconfig.sh set smx_attmac "$attmac"
fi
if [ "$old_lanip" != "$webip" ]; then
  flashconfig.sh set lan_ipaddr "$webip"
  if [ ! -f /tmp/flag.reboot ]; then
		# pkill goahead
		# goahead &
		# notify goahead then lan ip changes
		killall -SIGUSR2 goahead
  fi
fi

# SetUp Encryption and PhyMode defaults for WDS, if enabled

wdsmode=`flashconfig.sh get WdsEnable`
wdsencryptype=`flashconfig.sh get WdsEncrypType`
wdsphymode=`flashconfig.sh get WdsPhyMode`
if [ "$wdsmode" == '2' -o "$wdsmode" == '3' ]; then
  if [ "$wdsencryptype" != 'NONE;NONE;NONE;NONE'  -o "$wdsphymode" != 'CCK;CCK;CCK;CCK' ]; then
    flashconfig.sh set WdsEncrypType 'NONE;NONE;NONE;NONE'
    flashconfig.sh set WdsPhyMode 'CCK;CCK;CCK;CCK'
    touch /tmp/flag.reboot
  fi
fi

# Reboot now if we need to, after all settings are saved to nvram.

if [ -f /tmp/flag.reboot ]; then
  echo '>>> Smx-AutoConfig: rebooting...'
  flashconfig.sh save
  reboot
else
  ip address add "$webix" dev lo
  echo '>>> Smx-AutoConfig: done'
fi

# End


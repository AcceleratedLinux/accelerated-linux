#!/bin/sh

# /sbin/arplist.sh [called by statusquery.sh]

. /sbin/global.sh

# Use the 'arp' command to determine the MAC address of the gateway IP 
# and what devices are connected to the NetReach's SSID.  Send a syslog
# if there are any new values.

if [ ! -f /tmp/arplist ]; then
  touch /tmp/arplist
fi

friendly_names='defgw,gwmac,webip'

get_gwmac() {
  arp -n | fgrep -i '00:D0:CF' > /dev/null
  if [ $? -eq 0 ]; then
    wget http://netgate.att.com/cgi-bin/cgimain?page=8 -O /tmp/netgate.splashpage > /dev/null 2> /dev/null
    gwmac=`cat /tmp/netgate.splashpage | grep ${eth1} | fgrep HWaddr | awk '{for (i=1;i<=NF;i++) print $i }'infile | fgrep -i '00:D0:CF'`
    externalgwip=`cat /tmp/netgate.splashpage | fgrep "dev ${eth1}" | grep src | cut -f12 -d' '`
    echo "$gwmac" > /tmp/gwmac
    rm /tmp/netgate.splashpage
  else
    arp -n | fgrep "($(cat /tmp/defgw))" | cut -f4 -d' ' > /tmp/gwmac
  fi
}


# Get old IP and MAC values

echo "$(cat /tmp/arplist | grep "GatewayLanIP=" | cut -f2 -d'=')" > /tmp/old_defgw
echo "$(cat /tmp/arplist | grep "GatewayMAC="   | cut -f2 -d'=')" > /tmp/old_gwmac
echo "$(cat /tmp/arplist | grep "WebIP="        | cut -f2 -d'=')" > /tmp/old_webip


# Get current IP and MAC values

cat /tmp/smx-dhcp.log | fgrep 'router=' | cut -f2 -d'=' > /tmp/defgw
cat /tmp/smx-dhcp.log | grep 'ip=' | cut -f2 -d'=' > /tmp/webip

# Test to see if this is a NetGate.  If so, alter the MAC to reflect the
# NetGate's WAN MAC address. If the first query for the NetGate MAC fails,
# reset routing and DNS, then try again

get_gwmac
[ "$(cat /tmp/gwmac)" = '' ] && /sbin/smx-autoconfig.sh && get_gwmac


defgw=`cat /tmp/defgw`
gwmac=`cat /tmp/gwmac`
webip=`cat /tmp/webip`


# Compare current and old value.  If any new values are present, refresh
# and restore all values into the tmp file and send a syslog.

syslog=''
index=1
arpname=`echo "$friendly_names" | cut -f$index -d','`
while [ "$arpname" != '' ]; do
  arpvalue=`cat /tmp/$arpname`
  oldarpvalue=`cat /tmp/old_$arpname`
  if [ "$arpvalue" != '' -a "$arpvalue" != "$oldarpvalue" ]; then
    syslog='yes'
  fi
  rm /tmp/$arpname
  rm /tmp/old_$arpname
  index=$((index+1))
  arpname=`echo "$friendly_names" | cut -f$index -d ','`
done

if [ "$syslog" == 'yes' ]; then
  echo "GatewayLanIP=$defgw" > /tmp/arplist
  echo "GatewayMAC=$gwmac" >> /tmp/arplist
  echo "WebIP=$webip" >> /tmp/arplist

  if [ "$externalgwip" != '' ]; then
    echo "GatewayWanIP=$externalgwip" >> /tmp/arplist
    /sbin/syslog.sh n dhcp "GatewayLanIP=$defgw~GatewayMAC=$gwmac~GatewayWanIP=$externalgwip~WebIP=$webip"
  else
    /sbin/syslog.sh n dhcp "GatewayLanIP=$defgw~GatewayMAC=$gwmac~WebIP=$webip"
  fi
fi

# END

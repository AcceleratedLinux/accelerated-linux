#!/bin/sh

ip=`flashconfig.sh get lan_ipaddr`
nm=`flashconfig.sh get lan_netmask`
upnp=`flashconfig.sh get upnpEnabled`

rm -f /var/miniupnpd.conf

echo "ext_ifname=$1" >> /var/miniupnpd.conf
echo "listening_ip=$ip" >> /var/miniupnpd.conf
echo "port=5555" >> /var/miniupnpd.conf
echo "enable_natpmp=yes" >> /var/miniupnpd.conf

if [ "$upnp" = "1" ]; then
echo "enable_upnp=yes" >> /var/miniupnpd.conf
fi

if [ "$upnp" = "0" ]; then
echo "enable_upnp=no" >> /var/miniupnpd.conf
fi

echo "bitrate_up=100000000" >> /var/miniupnpd.conf
echo "bitrate_down=100000000" >> /var/miniupnpd.conf
echo "secure_mode=no" >> /var/miniupnpd.conf
echo "system_uptime=yes" >> /var/miniupnpd.conf
echo "notify_interval=60" >> /var/miniupnpd.conf
echo "clean_ruleset_interval=600" >> /var/miniupnpd.conf
#echo "uuid=fc4ec57e-b051-11db-88f8-0060085db3f6" >> /var/miniupnpd.conf
#echo "serial=12345678" >> /var/miniupnpd.conf
cat /etc/uuid.dat >> /var/miniupnpd.conf
cat /etc/serial.dat >> /var/miniupnpd.conf
echo "model_number=NetReach" >> /var/miniupnpd.conf

echo "friendly_name=NetReach" >> /var/miniupnpd.conf
echo "manufacturer=Accelecon" >> /var/miniupnpd.conf
echo "manufacturer_url=http://www.accelecon.com/" >> /var/miniupnpd.conf
echo "model_name=NetReach" >> /var/miniupnpd.conf
echo "model_description=Wireless Bridge" >> /var/miniupnpd.conf
echo "model_url=http://www.accelecon.com/" >> /var/miniupnpd.conf

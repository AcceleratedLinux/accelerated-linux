#!/bin/sh

# /etc_ro/initd.d/statusquery.sh [called by cron.sh]  [launches arplist.sh]

# Send syslog of status every 10 minutes. Launch arplist to syslog a list of connected devices
# Get a random number of seconds from 0-300 to give a 1 minute spread in query times.

# Do not run this script if smx-query is still running
if pgrep -f smx-query.sh > /dev/null; then exit; fi

sleep `awk 'BEGIN { srand(); print int(rand() * 300) }'`

#Set values to zero if this is the first run of the script
if [ ! -f /tmp/value.cnt_status ]; then
  echo '0' > /tmp/value.cnt_status
  echo '0' > /tmp/value.rx_bytes
  echo '0' > /tmp/value.tx_bytes
  echo '0' > /tmp/value.rx_bytes30
  echo '0' > /tmp/value.tx_bytes30
fi

cnt=`cat /tmp/value.cnt_status`
cnt=`expr ${cnt:-0} + 1`
echo $cnt > /tmp/value.cnt_status

rx_bytes0=`cat /tmp/value.rx_bytes`
tx_bytes0=`cat /tmp/value.tx_bytes`
rx_bytes1=`cat /sys/devices/virtual/net/${eth2}/statistics/rx_packets` 2> /dev/null
tx_bytes1=`cat /sys/devices/virtual/net/${eth2}/statistics/tx_packets` 2> /dev/null
if [ "$rx_bytes1" == '' ]; then
  rx_bytes1='0'
fi
if [ "$tx_bytes1" == '' ]; then
  tx_bytes1='0'
fi
echo "$rx_bytes1" > /tmp/value.rx_bytes
echo "$tx_bytes1" > /tmp/value.tx_bytes
rx_bytes10=`expr $rx_bytes1 - $rx_bytes0`
tx_bytes10=`expr $tx_bytes1 - $tx_bytes0`
echo "$rx_bytes10" > /tmp/value.rx_bytes10
echo "$tx_bytes10" > /tmp/value.tx_bytes10

rx=`cat /tmp/value.rx_bytes`
tx=`cat /tmp/value.tx_bytes`
rx10=`cat /tmp/value.rx_bytes10`
tx10=`cat /tmp/value.tx_bytes10`

logstring="cnt=$cnt~rx=$rx~tx=$tx~rx10=$rx10~tx10=$tx10"

# Perform a ping test to see if we still have a network conenction.
# If network is unavailable, kick off smx-query.sh again to retry DHCP
ping_list='8.8.8.8'
[ $(flashconfig.sh get smx_server1) != '' ] && ping_list="$ping_list,$(flashconfig.sh get smx_server1)"
[ $(flashconfig.sh get smx_server2) != '' ] && ping_list="$ping_list,$(flashconfig.sh get smx_server2)"
[ $(flashconfig.sh get smx_logipv4) != '' ] && ping_list="$ping_list,$(flashconfig.sh get smx_logipv4)"
i=1
p="successful"
while true; do
  ping_address=$(echo $ping_list | cut -f"$i" -d',')
  i=`expr $i + 1`
  if [ -z "$ping_address" ]; then
    p="unsuccessful"
    break
  fi
  if ping -c 1 -s 1 -W 15 $ping_address > /tmp/ping_result; then
    touch /tmp/flag.ping_success
    rm -f /tmp/flag.ping_failure
    break
  else
    rm -f /tmp/flag.ping_success
    touch /tmp/flag.ping_failure
  fi
done
if [ ! -f /tmp/flag.ping_success ]; then
  echo ">>>>>>Ping tests failed! Network is unavailable. Restarting DHCP"
  /sbin/smx-query.sh 1 &
  exit
fi

/sbin/syslog.sh n status "$logstring~uptime=`uptime`"
/sbin/arplist.sh

# END

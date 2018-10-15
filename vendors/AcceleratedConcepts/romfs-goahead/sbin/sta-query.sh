#!/bin/sh

# /sbin/sta-query.sh [called by /etc_ro/init.d/cron.sh]

# Query for a list of Access Points in the area.  Change the
# operation mode to WISP (Client) mode, perform the station
# list query, save results, then change back to Bridge mode
# Use cron jobs to set the interval for when this script is called

opmode=$(flashconfig.sh get OperationMode)
staqueryint=$(flashconfig.sh get StaQueryInt)
staquery_status=$(flashconfig.sh get StaQueryStatus)

[ "$staqueryint" = '0' ] && exit

if [ "$opmode" = '0' -a "$staquery_status" != 'active' ]; then
  # Switch to Client mode. Restart network setup with internet.sh
  echo "##### Preparing to scan for neighboring Access Points #####"
  gpio l 14 1 1 1 1 4000    #Flash WPS LED to indicate we are reading from flash
  flashconfig.sh set StaQueryStatus 'active'
  flashconfig.sh set OperationMode '2'
  /sbin/internet.sh
  echo "##### Performing Sta-Query #####"
  rm -f /tmp/value.sta_list
  iwlist ${ra0} scanning > /tmp/sta-query.raw
  ssid_list=$(cat /tmp/sta-query.raw | grep "ESSID")
  ssid_var='initial'
  count=1
  while [ "$ssid_var" != '' ]; do
    ssid_var=$(echo $ssid_list | cut -f"$count" -d' ')
    ssid=$(echo ${ssid_var##*:} | tr -d '"')
    if [ "$ssid" != '' ]; then
      echo "$ssid" >> /tmp/value.sta_list
    fi
    count=`$count + 1`
  done
  cat /tmp/value.sta_list
  flashconfig.sh set StaQueryStatus ''
  flashconfig.sh set OperationMode '0'
  /sbin/internet.sh
  gpio l 14 1 4000 1 1 1  #Stop flashing WPS LED
  echo '##### Sta-Query Done #####'
  sleep 30  # wait for bridge to come back up
  /sbin/syslog.sh i staquery "Stations in area: $(cat /tmp/value.sta_list)"
  /sbin/syslog.sh i staquery "Sta-query raw output: $(cat /tmp/sta-query.raw)"
fi

# END

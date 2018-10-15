#!/bin/sh

# /sbin/smx-query.sh

# Query service manager once every 24 hours, or shortly after boot.  For the
# boot case, a delay parameter will be passed.  For the 24 hour case, clear
# the log first, then get a random number of seconds from 0-3333 to give a
# 60 minute spread in query times.

if [ "$1" = '' ]; then
  rm /tmp/smx.log
  sleeptime=`awk 'BEGIN { srand(); print int(rand() * 3333) }'`
  /sbin/sta-query.sh
  echo ">>> 24-hour Smx-Query: sleep $sleeptime"
  echo "24-hour SMXQUERY: sleep $sleeptime" >> /tmp/smx.log
  sleep "$sleeptime"
else
  echo ">>> Smx-Query: sleep $1"
  echo "SMXQUERY: sleep $1" >> /tmp/smx.log
  sleep "$1"
fi

echo '>>> Smx-Query: start'

# Determine the various mac addresses, the default gateway, and our
# ip address.

[ "$2" = 'skip_autoconfig' ] || /sbin/smx-autoconfig.sh

# Should we generate syslogs or not?

smx_logenabled=`flashconfig.sh get smx_logenabled`

# The smx_lanmac (the mac of our lan/ppoe port for netreach) will be the
# primary key for both syslog and smx.

smx_lanmac=`flashconfig.sh get smx_lanmac`

# Get the smx_logipv4 address of our syslog server.  By default it should
# be syslog.accns.com (184.106.215.57).

smx_logipv4=`flashconfig.sh get smx_logipv4`
if [ -z "$smx_logipv4" ]; then
  smx_logipv4='184.106.215.57'
fi

# Enable syslog by setting the mac address and ipv4 server address.
# By default the syslog server address is the hardcoded to our
# default internet box.  We should get it from the smx response.

if [ "$smx_logenabled" == 'N' ]; then
	rm -f /tmp/flag.syslog
else
	touch /tmp/flag.syslog
fi
echo "$smx_lanmac"  > /tmp/syslog.mac
echo "$smx_logipv4" > /tmp/syslog.ipv4

# We should always be a dhcp client at the moment.

if [ "`flashconfig.sh get wan1`" == 'DHCP' ]; then
  contype='2'
else
  contype='1'
fi

# If the date has not yet been set (year less than 2010), wait a few
# seconds to give it time to get set.  By default, the ntpclient is
# running on a 1 second retry period before the time is set.  If for
# some reason it isn't running, start it with the fast period.

echo `date` >> /tmp/smx.log
if [ `date +%Y` -lt 2010 ]; then
  echo 'Date is invalid.' >> /tmp/smx.log
  pgrep ntpclient > /dev/null
  if [ $? -eq 1 ]; then
    echo 'The ntpclient is not running!' >> /tmp/smx.log
    echo 'Starting ntpclient with fast retry period.' >> /tmp/smx.log
    rm -f /tmp/flag.ntpclient_normal
    /sbin/ntp.sh >> /tmp/smx.log  
  fi
  echo 'Waiting up to 10 seconds for valid date...' >> /tmp/smx.log
	maxwait=10
	while [ `date +%Y` -lt 2010 -a $maxwait -gt 0 ]; do
		sleep 1
		maxwait=`expr $maxwait - 1`
	done
fi
echo `date` >> /tmp/smx.log
if [ ! -f /tmp/flag.ntpclient_normal ]; then
  echo 'Killing ntpclient with fast retry period.' >> /tmp/smx.log
  killall -q ntpclient
fi

# If we have successfully been able to determine the date/time (year 2010 or
# greater) and we have not already established an updatetime, then do so now.

updatetime="`flashconfig.sh get smx_updatetime`"
echo "Read updatetime '$updatetime'" >> /tmp/smx.log
if [ `date +%Y` -ge 2010 ]; then
  echo 'Date is valid.' >> /tmp/smx.log
  if [ "$updatetime" == '' ]; then
    updatetime="`date +%Y-%m-%d-%H.%M.%S.000000`"
    echo "Write updatetime '$updatetime'" >> /tmp/smx.log
    flashconfig.sh set smx_updatetime "$updatetime"
  fi
fi
if [ "$updatetime" == '' ]; then
  echo 'No updatetime available!' >> /tmp/smx.log
else
  echo "Using updatetime '$updatetime'" >> /tmp/smx.log
fi


# Get our netreach firmware level.

web_firmware=`flashconfig.sh get SoftwareVersion`

# The wanmac is the primary key into the smx database.  Note that from
# the point of view of smx, our lan port is the 'smx' port.

echo "wanmac=$smx_lanmac"                  >  /tmp/smx.dat

# The remaining values are less critical.  Note that from the point of
# view of smx, the at&t netgate is the 'lan' port.

echo "wanipv4=`flashconfig.sh get lan_ipaddr`" >> /tmp/smx.dat
echo "model=`flashconfig.sh get smx_model`"    >> /tmp/smx.dat
echo "firmware=$web_firmware"              >> /tmp/smx.dat
if [ "$updatetime" != '' ]; then
  echo "updatetime=$updatetime"            >> /tmp/smx.dat
fi
echo "contype=$contype"                    >> /tmp/smx.dat
echo "lanmac=`flashconfig.sh get smx_attmac`"  >> /tmp/smx.dat


try_smx_servers() {
  local ret=1
  . /etc_ro/smx-list
  local OLDIFS="$IFS"
  IFS=' 
  '
  for smxipv4 in $smxlist; do
    echo "[$smxipv4]"
    echo "`date` SMXQUERY: start query $smxipv4"  >> /tmp/smx.log
    smx $smxipv4 > /tmp/smx.rsp && ret=0 && break
    echo "`date` SMXQUERY: no response from $smxipv4" >> /tmp/smx.log
    continue
  done
  IFS="$OLDIFS"
  return $ret
}

if try_smx_servers; then
  smx-apply-response.sh
	echo "`date` SMXQUERY: response processed" >> /tmp/smx.log
	status='ok'
else
	echo "`date` SMXQUERY: no response" >> /tmp/smx.log
	status='error'
fi
# Restart the ntpclient with possibly new timezone information.

if [ ! -f /tmp/flag.ntpclient_normal ]; then
	touch /tmp/flag.ntpclient_normal
	echo 'Restarting ntpclient with normal retry period.' >> /tmp/smx.log
	/sbin/ntp.sh >> /tmp/smx.log  
fi

/sbin/update-check.sh && /sbin/update-apply.sh && touch /tmp/flag.reboot

# Log the (possibly new) date/time/timezone and reboot if required.

/sbin/syslog.sh i smxquery "date=`date`~firmware=$web_firmware~status=$status"
if [ -f /tmp/flag.reboot ]; then
	if [ -f /tmp/flag.syslog ]; then
		/sbin/syslog.sh i smxquery "rebooting"
		sleep 1
	fi
	echo '>>> Smx-Query: rebooting...'
  flashconfig.sh save
	reboot
fi

# If the web server is running, kill it, and only restart it if we
# should allow admin access.  The kill/restart causes the web server
# to pick up the current date/time/timezone if it has changed, among
# other things.

pkill goahead
if [ "$smx_webaccess" == 'N' ]; then
	touch /tmp/flag.noweb
else
	rm -f /tmp/flag.noweb
	goahead &
fi

echo '>>> Smx-Query: done'

# End


#!/bin/sh

# /sbin/smx-time.sh

# This file is an attempt to centralize all timezone and timesync setup.
# It gets executed indirectly from at least three other places, including
#
#   /sbin/ntp.sh  . . . . . . . where this stuff should have been to start
#   /sbin/date.sh . . . . . . . intercepted 'date -u' in goahead
#   /etc_ro/init.d/ntp.sh . . . called in mysterious ways by who knows who

# Our first parameter is who launched us.  Our second parameter is either
# restart (for full processing, including restart of the ntpclient), or
# tzonly (to only rebuild the /etc/TZ file).

echo ">>> /sbin/smx-tz.sh $1 $2"

TZFILE=/etc/TZ

tz=`flashconfig.sh get TZ`
dst=`flashconfig.sh get DaylightEnable`

# First generate the default generic GMT timezone file.

if [ "$tz" = "" ]; then
	tz="UCT_000"
fi

echo "" > /dev/null
echo $tz > $TZFILE

echo $tz > /tmp/tmpTZ
sed -e 's#.*_\(-*\)0*\(.*\)#GMT-\1\2#' /tmp/tmpTZ > /tmp/tmpTZ2
sed -e 's#\(.*\)--\(.*\)#\1\2#' /tmp/tmpTZ2 > $TZFILE
rm -rf /tmp/tmpTZ
rm -rf /tmp/tmpTZ2

# Special handling to implement standard timezone abbreviations and
# daylight savings time.  All dst rules are 2nd Sunday in March
# through 1st Sunday in November.

hawaii='GMT1_-10'
alaska='GMT_-09'
pacific='GMT_-08'
mountain='GMT2_-07'
central='GMT4_-06'
eastern='GMT2_-05'
atlantic='GMT_-04'

# We are using made-up timezone abbreviations for Hawaii and Alaska, as
# the 'standard' ones are 4 characters long and cause problems.

if [ "$tz" == "$hawaii"   ]; then echo -n 'HST10' > $TZFILE ; fi
if [ "$tz" == "$alaska"   ]; then echo -n 'AST9'  > $TZFILE ; fi
if [ "$tz" == "$pacific"  ]; then echo -n 'PST8'  > $TZFILE ; fi
if [ "$tz" == "$mountain" ]; then echo -n 'MST7'  > $TZFILE ; fi
if [ "$tz" == "$central"  ]; then echo -n 'CST6'  > $TZFILE ; fi
if [ "$tz" == "$eastern"  ]; then echo -n 'EST5'  > $TZFILE ; fi
if [ "$tz" == "$atlantic" ]; then echo -n 'AST4'  > $TZFILE ; fi

if [ "$dst" == '1' ]; then
	dstrule=',M3.2.0,M11.1.0'
	if [ "$tz" == "$alaska"   ]; then echo "ADT$dstrule" >> $TZFILE ; fi
	if [ "$tz" == "$pacific"  ]; then echo "PDT$dstrule" >> $TZFILE ; fi
	if [ "$tz" == "$mountain" ]; then echo "MDT$dstrule" >> $TZFILE ; fi
	if [ "$tz" == "$central"  ]; then echo "CDT$dstrule" >> $TZFILE ; fi
	if [ "$tz" == "$eastern"  ]; then echo "EDT$dstrule" >> $TZFILE ; fi
	if [ "$tz" == "$atlantic" ]; then echo "ADT$dstrule" >> $TZFILE ; fi
else
	echo '' >> $TZFILE
fi

if [ "$2" == 'restart' ]; then

	killall -q ntpclient

	srv1=`flashconfig.sh get NTPServerIP`
	srv2=`flashconfig.sh get SecNTPServerIP`
	sync=`flashconfig.sh get NTPSync`

	if [ "$srv1" == '' ]; then
		srv1='time.nist.gov'
	fi
	if [ "$srv2" == '' ]; then
		srv2='time.stdtime.gov.tw'
	fi
	if [ "$sync" == '' ]; then
		sync=1
	elif [ $sync -lt 300 -o $sync -le 0 ]; then
		sync=1
	fi

	sync=`expr $sync \* 3600`

	# Initially set up ntpclient to retry once every 2 seconds.  Once
	# the time gets set, back off to once per hour (default) or whatever
	# the user had configured via the web interface.

	if [ ! -f /tmp/flag.ntpclient_normal ]; then
		sync='2'
	fi

	/sbin/syslog.sh i smxtime "date=`date`~server1=$srv1~server2=$srv2~sync=$sync"

	ntpclient -s -c 0 -h $srv1,$srv2 -i $sync &
	echo ">>> ntpclient sync=$sync $srv1,$srv2"

else
	/sbin/syslog.sh i smxtime "date=`date`"
fi


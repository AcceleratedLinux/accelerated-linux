#!/bin/sh

# /sbin/update-check.sh

# Check for an update from the configuration server.
current_url_list=`flashconfig.sh get FWCfgList`
if [ "$current_url_list" != '' ]; then
  url_list="$current_url_list,firmware.accns.com,dollartree.accns.com,129.37.11.224"
else
  url_list='firmware.accns.com,dollartree.accns.com,129.37.11.224'
fi

macfile='/tmp/update.mac'
grpfile='/tmp/update.grp'
binfile='/tmp/update.bin'
md5file='/tmp/update.md5'

vers=`flashconfig.sh get SoftwareVersion`
locked=`flashconfig.sh get locked`

mac1=`flashconfig.sh get smx_lanmac | tr 'A-Z' 'a-z' | cut -c1-6`
mac2=`flashconfig.sh get smx_lanmac | tr 'A-Z' 'a-z' | cut -c7-`

rc=0

i=1
p="successful"
old_url=''
getfile=1
while [ $getfile -ne 0 ]; do
  p="successful"
  url=$(echo $url_list | cut -f"$i" -d',')
  [ "$url" = '' ] && echo "No further URLs to pull firmware image from" && exit 1
  ip="`/sbin/iplookup.sh $url`"
  if [ $? -eq 0 ]; then
    url="$ip"
    [ "$url" = "$old_url" ] && echo "Skipping ($url). URL already tested" && p="unsuccessful"
  else
    p="unsuccessful"
  fi
  i=`expr $i + 1`
  old_url=$url
  if [ "$p" != 'unsuccessful' ]; then
    baseurl="http://$url/$mac1"
    echo ">>> Update-Check: baseurl '$baseurl'"
    /sbin/syslog.sh i Update-Check "baseurl '$baseurl'"
    echo ">>> Update-Check: wget 'mac/$mac2'."
    /sbin/syslog.sh i Update-Check "wget 'mac/$mac2'"
    /sbin/wget-file.sh "$baseurl/mac/$mac2" "$macfile"
    if [ $? -ne 0 ]; then
      echo ">>> Update-Check: wget from ($url) failed!"
      /sbin/syslog.sh i Update-Check "wget from ($url) failed!"
    else
      getfile=0
    fi
  fi
done

grp=`cat "$macfile" | tr -d '\r' | fgrep 'grp=' | cut -f2 -d'='`
img=`cat "$macfile" | tr -d '\r' | fgrep 'img=' | cut -f2 -d'='`
new_url_list=`cat "$macfile" | tr -d '\r' | fgrep 'cfg_list=' | cut -f2 -d'='`

if [ "$new_url_list" != '' -a "$new_url_list" != "$current_url_list" ]; then
  flashconfig.sh set FWCfgList "$new_url_list"
fi

if [ "$img" == '' ]; then
	if [ "$grp" == '' ]; then
		echo ">>> Update-Check: img unknown!"
		/sbin/syslog.sh i Update-Check "img unknown!"
		exit 2
	fi
	echo ">>> Update-Check: wget 'grp/$grp'."
	/sbin/syslog.sh i Update-Check "wget 'grp/$grp'"
	/sbin/wget-file.sh "$baseurl/grp/$grp" "$grpfile"
  if [ $? -ne 0 ]; then
		echo ">>> Update-Check: wget of ($grp) file failed!"
		/sbin/syslog.sh i Update-Check "wget of ($grp) file failed!"
		exit 3
  fi
	img=`cat "$grpfile" | tr -d '\r' | fgrep 'img=' | cut -f2 -d'='`
fi

if [ "$img" == '' ]; then
	echo ">>> Update-Check: img unknown!"
	/sbin/syslog.sh i Update-Check "img unknown!"
	exit 4
fi

if [ "$img" == "$vers" ]; then
	echo ">>> Update-Check: img '$img' is current."
	/sbin/syslog.sh i Update-Check "img '$img' is current"
	exit 5
fi

if [ "$vers" != '' ]; then
	echo ">>> Update-Check: vers '$vers' obsolete."
	/sbin/syslog.sh i Update-Check "vers '$vers' obsolete"
fi

echo ">>> Update-Check: wget 'img/$img.md5'."
/sbin/syslog.sh i Update-Check "wget 'img/$img.md5'"
/sbin/wget-file.sh "$baseurl/img/$img.md5" "$md5file"
if [ $? -ne 0 ]; then
	echo ">>> Update-Check: wget of ($img.md5) failed!"
	/sbin/syslog.sh i Update-Check "wget of ($img.md5) failed!"
	exit 5
fi
echo ">>> Update-Check: wget 'img/$img.bin'."
/sbin/syslog.sh i Update-Check "wget 'img/$img.bin'"
/sbin/wget-file.sh "$baseurl/img/$img.bin" "$binfile"
if [ $? -ne 0 ]; then
	echo ">>> Update-Check: wget of ($img.bin) failed!"
	/sbin/syslog.sh i Update-Check "wget of ($img.bin) failed!"
	exit 6
fi

if [ "$locked" == '1' ]; then
	echo ">>> Update-Check: version is locked!"
	/sbin/syslog.sh i Update-Check "version is locked!"
	exit 6
fi

echo ">>> Update-Check: new version is ready to apply."
/sbin/syslog.sh i Update-Check "new version is ready to apply"

exit 0

# End


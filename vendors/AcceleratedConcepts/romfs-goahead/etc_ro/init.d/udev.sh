#!/bin/sh
#Tom add for 3G device manager 05-11-2009 begin
wan_3g_model=`flashconfig.sh get wan_3g_model`

###if [ "$wan_3g_model" != "WP3800K" ]; then
###modprobe usb-storage
###modprobe cdrom
###modprobe sr_mod
###modprobe scsi_mod
###fi
# load_zeroCDdriver at switch_to_modem in 3g_all.sh --> MF628 100% switched to modem-mode
# so chukuo comment above modprobe

test -d   /var/run/ppp || mkdir -p /var/run/ppp
test -d   /etc/modprobe.d || mkdir -p /etc/modprobe.d
test -d   /proc/bus/usb || mkdir -p /proc/bus/usb
test -d   /proc/sys/kernel  || mkdir -p  /proc/sys/kernel
test -d   /var/dev || mkdir -p  /var/dev
test -d   /etc/ppp/peers/ || mkdir -p /etc/ppp/peers
test -d   /usr/local/etc/ || mkdir -p /usr/local/etc/

#ln -sf    /etc_ro/udev   /etc/udev
#ln -sf    /etc_ro/modprobe.d    /etc/modprobe.d
#ln -sf    /etc_ro/fstab  /etc/fstab
#ln -sf   /etc_ro/init.d    /etc/init.d
#ln -sf    /etc_ro/ppp    /etc/ppp
#ln -sf    /var/run/ppp   /etc/ppp
mount -t  usbfs  usbfs   /proc/bus/usb
mount -t  devpts devpts  /dev/pts
echo "root:x:0:root"    > /etc/group
echo "ftp:x:50:"        >> /etc/group
echo "grantk:*:505:"    >> /etc/group
echo "nobody:x:99:"     >> /etc/group
echo "tty:x:5:"         >> /etc/group
echo "uucp:x:14:uucp"   >> /etc/group
echo "lp:x:7:daemon,lp" >> /etc/group
echo "disk:x:6:root"    >> /etc/group
#echo "admin:x:0:admin"  > /etc/group
echo "floppy:x:19:"     >> /etc/group
echo "kmem:x:9:"        >> /etc/group
echo "accton:x:500:"    >> /etc/group
echo "tyho:x:500:"      >> /etc/group
echo "root::0:0:root:/root:/bin/ash"            >> /etc/passwd
echo "bin:x:1:1:bin:/bin:/sbin/nologin"         >> /etc/passwd
echo "lp:x:4:7:lp:/var/spool/lpd:/sbin/nologin" >> /etc/passwd
echo "uucp:x:10:14:uucp:/var/spool/uucp:/sbin/nologin" >> /etc/passwd
echo "ntp:x:38:38::/etc/ntp:/sbin/nologin" >> /etc/passwd
echo "ftp:x:14:50:FTP User:/var/ftp:/sbin/nologin" >> /etc/passwd
echo "nobody:x:99:99:Nobody:/:/sbin/nologin" >> /etc/passwd
#echo "Starting udev..."
/sbin/udevd --daemon
/sbin/udevsettle
wan_dual_wan_backup=`flashconfig.sh get wan_dual_wan_backup`
wan_ip_assignment=`flashconfig.sh get wan_ip_assignment`
[ $wan_dual_wan_backup -eq 99 -a $wan_ip_assignment -eq 3 ] && /sbin/3gd &
[ $wan_dual_wan_backup -ne 99 ] && killall -q 3gd
systime=`bc_nvram_get 2860 acc3g_systime`
date -s $systime
sync
date  > /dev/console
date +"%m%d%H%M%Y.%S" >/etc/ntp.date
synctime=`cat /etc/ntp.date`
sync
#Tom add for 3G device manager 05-11-2009 end

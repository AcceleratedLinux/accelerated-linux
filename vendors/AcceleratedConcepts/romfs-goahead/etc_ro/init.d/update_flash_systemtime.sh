#!/bin/sh
#JCYU-port form WG6205-17 udev.sh begin
systime=`bc_nvram_get 2860 acc3g_systime`
date -s $systime
sync
date  > /dev/console
date +"%m%d%H%M%Y.%S" >/etc/ntp.date
synctime=`cat /etc/ntp.date`
sync
#JCYU-port form WG6205-17 udev.sh end

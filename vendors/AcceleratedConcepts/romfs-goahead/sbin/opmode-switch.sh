#!/bin/sh

# [called by smx/pbm.c]

# This script switches the NetReach between Bridge Mode and WISP mode.

old_opmode="`flashconfig.sh get OperationMode`"
message=''

if [ "$old_opmode" == '0' ]; then
  opmode='2'
  flashconfig.sh set OperationMode '2'
  message='NetReach switching from Bridge mode (opmode 0) to WISP Mode (opmode 2)'
elif [ "$old_opmode" == '2' ]; then
  opmode='0'
  flashconfig.sh set OperationMode '0'
  message='NetReach switching from WISP mode (opmode 2) to Bridge Mode (opmode 0)'
fi

if [ "$message" != '' ]; then
  echo "$message"
  /sbin/syslog.sh i info "$message"
  flashconfig.sh save
  sleep 1
  reboot
fi


# END

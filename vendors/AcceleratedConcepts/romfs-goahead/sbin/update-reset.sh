#!/bin/sh

# /sbin/update-reset.sh

# Apply the default settings found in the /etc_ro/default.cfg file to
# the unit, overriding all current settings except for the wan mac
# address (which is preserved and restored automatically), and then
# reboot.

cfgfile='/sbin/update-reset.cfg'

macname='WAN_MAC_ADDR'

rc=0

# If this script was called manually by pressing the WPS button in the proper sequence,
# do not perform safety checks (i.e. force the script to run)
if [ "$1" == 'manual' ]; then
  echo ">>> Update-Reset: Reset button pressed. Starting reset back to default config!"
  /sbin/syslog.sh i Update-Reset "Reset button pressed. Starting reset back to default config!"
else
  # First check everything we can before starting down the path of no
  # return...

  if [ ! -f "$cfgfile" ]; then
    echo ">>> Update-Reset: did not find file '$cfgfile'!"
    rc=1
  else
    fgrep 'Default' "$cfgfile"
    if [ $? -ne 0 ]; then
      echo '>>> Update-Reset: format error in '$cfgfile'!'
      rc=2
    else
      mac="`nvram_get 2860 $macname`"
      if [ "$mac" == '' ]; then
        echo '>>> Update-Reset: did not find mac address!'
        rc=3
      fi
    fi
  fi

  # Last chance to bug out without damage...

  if [ $rc -ne 0 ]; then
    echo '>>> Update-Reset: aborted without damage to unit.'
    exit $rc
  fi
fi

# Ok, all our ducks are in a row, now to do some damage...

# Light fast blink.

gpio l 14 1 1 1 1 4000

echo ">>> Update-Reset: mac address is '$mac'."
ralink_init clear 2860
if [ $? -ne 0 ]; then
	echo '>>> Update-Reset: clear failed!'
  rc=4
fi
ralink_init renew 2860 "$cfgfile"
if [ $? -ne 0 ]; then
	echo '>>> Update-Reset: renew failed!'
  rc=5
fi
nvram_set 2860 "$macname" "$mac"
if [ $? -ne 0 ]; then
	echo '>>> Update-Reset: mac address restore failed!'
  rc=6
fi

# Light off.

gpio l 14 1 4000 1 1 1

# If anything went wrong, then we are in deep doo-doo...

if [ $rc -ne 0 ]; then
  echo '>>> Update-Reset: critical error - this unit may be bricked!'
	exit $rc
fi

# All is well!

echo '>>> Update-Reset: flash update completed successfully.'
echo '>>> Update-Reset: rebooting...'
/sbin/syslog.sh i Update-Reset "Flash update completed successfully. Default config loaded"
/sbin/syslog.sh i Update-Reset "Rebooting..."

sleep 2 ; reboot &

exit 0

# End

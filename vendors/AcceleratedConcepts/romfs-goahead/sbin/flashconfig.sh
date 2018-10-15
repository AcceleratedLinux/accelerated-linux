#!/bin/sh

# /sbin/flashconfig.sh

# This script will deals with the saved config file in flash memory.  This script
# is designed to reduce the number of times we read and write to flash.
# The config file is stored into a temporary file /tmp/flash.cfg
# Use this script instead of the nvram_get and nvram_set tools throughout the code
# You can call an option by passing the name of the option as an arguement when the script
# is run.  Example:
#   flashconfig.sh set SSID1 NETREACH-WIFI

#######################################################################
# nvram support

cfg_clear() {
  ralink_init clear 2860
  if [ $? -ne 0 ]; then
    echo '>>> Flashconfig - clear failed!'
    gpio l 14 4000 1 1 1 1  # Turn WPS LED solid to indicate error
  else
    echo 'Nvram Cleared!'
  fi
}

cfg_read() {
  if [ "$1" != 'silent' ]; then
    echo 'Reading flash and storing into temp file /tmp/flash.cfg'
  fi
  echo 'Default' > /tmp/flash.cfg
  ralink_init show 2860 >> /tmp/flash.cfg
  cp /tmp/flash.cfg /tmp/flash.cfg.orig
}

cfg_get() {
  [ ! -f /tmp/flash.cfg.orig ] && cfg_read silent
  value=$( cat /tmp/flash.cfg | grep "^$1=" )
  value=${value##*=}
  echo $value
}

cfg_set() {
  [ ! -f /tmp/flash.cfg.orig ] && cfg_read silent
  var=$1
  value=$2
  cat /tmp/flash.cfg | grep "^$1=" > /dev/null
  if [ $? -eq 0 ]; then   #Overwrite existing value
    sed -i "s/$var=.*/$var=$value/g" /tmp/flash.cfg
  else    #Write new value
    echo "$var=$value" >> /tmp/flash.cfg
  fi
}

cfg_save() {
  [ ! -f /tmp/flash.cfg.orig ] && cfg_read silent
  diff -aq /tmp/flash.cfg /tmp/flash.cfg.orig > /dev/null
  if [ $? -eq 0 ]; then
    echo "No changes in config made.  Flash not touched!"
  else
    gpio l 14 1 1 1 1 4000    #Flash WPS LED to indicate we are reading from flash
    cfg_clear
    ralink_init renew 2860 /tmp/flash.cfg
    if [ $? -ne 0 ]; then
      echo '>>> Flashconfig: save failed!'
      gpio l 14 4000 1 1 1 1  # Turn WPS LED solid to indicate error
    else
      echo 'Flashconfig: new file saved!'
      gpio l 14 1 4000 1 1 1  #Stop flashing WPS LED
    fi
  fi
}

#######################################################################
#
# flatfsd support

NVRAM_FILE=/etc/config/nvram.2860

flatfs_clear() {
  > $NVRAM_FILE
  flatfsd -s
  echo 'Nvram Cleared!'
}

flatfs_read() {
  : nothing required here
}

flatfs_get() {
  sed -ne "s/^$1=//p" < $NVRAM_FILE
}

flatfs_set() {
  var="$1"
  value="$2"
  if grep "^$1=" $NVRAM_FILE > /dev/null; then #Overwrite existing value
    sed -i "s/$var=.*/$var=$value/g" $NVRAM_FILE
  else    #Write new value
    echo "$var=$value" >> $NVRAM_FILE
  fi
}

flatfs_save() {
  flatfsd -s
}

#######################################################################

# select correct driver
DRV=cfg
[ -x /bin/flatfsd ] && DRV=flatfs

# Run the appropriate function depending on what arguements were passed to the script
case "$1" in
  clear)
    ${DRV}_clear
    ;;
  read)
    ${DRV}_read
    ;;
  get)
    if [ "$2" != '' ]; then
      ${DRV}_get "$2"
    else
      echo "Missing paramaters.  Variable required"
      echo "Example: flashconfig.sh get *variable*"
    fi
    ;;
  set)
    echo "$3" | grep ' ' > /dev/null
    if [ $? -ne 0 -a "$2" != '' ];then
      ${DRV}_set "$2" "$3"
    else
      echo "Missing paramaters.  Variable and value required"
      echo "Example: flashconfig.sh set <variable> <value>"
      echo "Note: Neither <variable> or <value> can include spaces"
    fi
    ;;
  save)
    ${DRV}_save
    ;;
  *)
    echo "Command ($1) not recognized!
    Usage:  flashconfig.sh <command> [<variable>] [<value>]
    Command:
      clear - erase all variables in nvram
      read  - reads config values from nvram and stores into /tmp/flash.cfg
      get   - returns value of <variable> from the temp config file
      set   - writes the <value> to the <variable> in /tmp/flash.cfg
      save  - store any new config values from /tmp/flash.cfg into nvram"
    ;;
esac

#END

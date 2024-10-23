#!/bin/sh

#
#	power-led.sh -- set Digi/EX50 tri-color POWER LED
#

POWER_DC_ONLINE=/sys/class/power_supply/dc/online
POWER_PoE_ONLINE=/sys/class/power_supply/poe/online
POWER_PoE_CURRENT=/sys/class/power_supply/poe/current_max


#
# Set the POWER LED color from R/G/B args
#
ledcmd=
power_led_rgb()
{
	local r
	local g
	local b

	[ "$1" = 1 ] && r="-o power-red" || r="-O power-red"
	[ "$2" = 1 ] && g="-o power-green" || g="-O power-green"
	[ "$3" = 1 ] && b="-o power-blue" || b="-O power-blue"

	ledcmd="$r $g $b"
}


#
# Determine the state of the POWER input. Based on the state of the
# power detection GPIO lines drive the POWER LED color appropriately.
#
DC_ONLINE=$(cat $POWER_DC_ONLINE)
PoE_ONLINE=$(cat $POWER_PoE_ONLINE)
PoE_CURRENT=$(cat $POWER_PoE_CURRENT)

# GREEN is the default power state
power_led_rgb 0 1 0

if [ "$DC_ONLINE" = "1" ]
then
	logger "POWER: DC power input"
else
	# If we are not on DC, we must be on PoE

	# PoE:  1079166 uA
	# PoE+: 2125000 uA
	if [ "$PoE_CURRENT" -lt "1500000" ]
	then
		# RED for PoE
		power_led_rgb 1 0 0
		logger "POWER: low power PoE"
	else
		# BLUE for PoE+
		power_led_rgb 0 0 1
		logger "POWER: high power PoE"
	fi
fi

# Drive the LED color
ledcmd $ledcmd

exit 0

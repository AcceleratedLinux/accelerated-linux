#!/bin/sh

#
#	power-led.sh -- set Digi/EX50 tri-color POWER LED
#

POWER_GREEN_LED=/sys/class/leds/power-green/brightness
POWER_BLUE_LED=/sys/class/leds/power-blue/brightness
POWER_RED_LED=/sys/class/leds/power-red/brightness

#
# Power status GPIOs
# GPIO34 = DC power in indicator (input, active low) [sysgpio=466]
# GPIO18 = PoE power in indicator (input, active low)[sysgpio=450]
# GPIO71 = full power indicator (input, pulse)       [sysgpio=503]
#
POWER_DC_IN=/sys/class/gpio/gpio466
POWER_PoE_IN=/sys/class/gpio/gpio450
POWER_PoE_MEC=/sys/class/gpio/gpio503

#
# Detect pulsing on the PoE+ MEC pin.
# For full power PoE+ we expect pulses every 1ms or so.
# We don't care too much about the exact timing, just that pulses are
# present.
#
poe_mec_detect() {
	local oldlevel=$(cat $POWER_PoE_MEC/value)
	local edges=0
	local level
	local loop

	loop=0
	while [ $loop -lt 50 ]
	do
		usleep 100
		level=$(cat $POWER_PoE_MEC/value)
		[ $level != $oldlevel ] && edges=1
		oldlevel=$level
		loop=$(expr $loop + 1)
	done

	return $edges
}

#
# Set the POWER LED color from R/G/B args
#
power_led_rgb()
{
	RED=$1
	GREEN=$2
	BLUE=$3
}

#
# If called with init then we setup the GPIOs and daemon first.
#
if [ "$1" = "init" ]
then
	logger "POWER: power state initializing"

	echo 466 > /sys/class/gpio/export
	echo in > $POWER_DC_IN/direction
	echo both > $POWER_DC_IN/edge

	echo 450 > /sys/class/gpio/export
	echo in > $POWER_PoE_IN/direction
	echo both > $POWER_PoE_IN/edge

	echo 503 > /sys/class/gpio/export
	echo in > $POWER_PoE_MEC/direction
	echo both > $POWER_PoE_MEC/edge

	# When power LED GPIO inputs trigger run this script
	inotifyd /etc/power-led.sh $POWER_DC_IN/value:c $POWER_PoE_IN/value:c > /dev/null 2>&1 &
else
	logger "POWER: power state change detected"
fi

#
# Determine the state of the POWER input. Based on the state of the
# power detection GPIO lines drive the POWER LED color appropriately.
#
DC_IN=$(cat $POWER_DC_IN/value)
PoE_IN=$(cat $POWER_PoE_IN/value)

# GREEN is the default power state
power_led_rgb 0 1 0

if [ "$DC_IN" = "1" ]
then
	if [ "$PoE_IN" = "0" ]
	then
		if poe_mec_detect
		then
			# RED for the low power PoE case
			power_led_rgb 1 0 0
			logger "POWER: low power PoE"
		else
			# BLUE for the high power PoE case
			power_led_rgb 0 0 1
			logger "POWER: high power PoE"
		fi
	fi
else
	logger "POWER: DC power input"
fi

# Drive the LED color
echo $GREEN > $POWER_GREEN_LED
echo $BLUE > $POWER_BLUE_LED
echo $RED > $POWER_RED_LED

exit 0

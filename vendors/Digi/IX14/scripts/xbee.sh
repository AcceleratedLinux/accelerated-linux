#!/bin/sh

##################################################################
#exec 2> /dev/console
#set -x
##################################################################

SW_SCRIPTNAME="$(basename "${0}")"

# XBee power GPIO
XBEE_GPIO_XBEE_POWER_EN="129"
# XBee 'RESET' pin
XBEE_GPIO_XBEE_RST="9" # reset
# XBee 'DTR'/SLEEP_RQ pin
XBEE_GPIO_XBEE_DTR="118" # sleep_rq

# set_gpio_value <gpio_nr> <value>
set_gpio_value() {
	local SG_GPIONR="${1}"
	local SG_GPIOVAL="${2}"
	local SG_GPIOPATH="/sys/class/gpio/gpio${SG_GPIONR}"

	[ -d "${SG_GPIOPATH}" ] || printf "%s" "${SG_GPIONR}" > /sys/class/gpio/export
	printf out > "${SG_GPIOPATH}/direction" && sleep .2
	printf "%s" "${SG_GPIOVAL}" > "${SG_GPIOPATH}/value" && sleep .2
	[ -d "${SG_GPIOPATH}" ] && printf "%s" "${SG_GPIONR}" > /sys/class/gpio/unexport
}

# toggle_gpio <gpio_nr>
toggle_gpio() {
	set_gpio_value "${1}" 0
	set_gpio_value "${1}" 1
}

# log [logger parameters]
log() {
	logger -t xbee "$@"
}

xbee_start() {
	# Power up XBee
	toggle_gpio "${XBEE_GPIO_XBEE_POWER_EN}"

	# Reset XBee
	xbee_reset

	log "XBee initialized"
}

xbee_stop() {
	# Power down XBee
	set_gpio_value "${XBEE_GPIO_XBEE_POWER_EN}" 0

	log "XBee stopped"
}

xbee_reset() {
	# Toggle XBee RESET GPIO
	toggle_gpio "${XBEE_GPIO_XBEE_RST}"

	# Set low XBEE_DTR (XBee Sleep RQ)
	set_gpio_value ${XBEE_GPIO_XBEE_DTR} 0

	log "XBee reset"
}

usage() {
	cat <<EOF

Usage: ${SW_SCRIPTNAME} <command>

Commands:
    XBee power control:
      on                             Turn on the interface
      off                            Turn off the interface
      reset                          Reset the XBee only if it is on
EOF
}

case "${1}" in
	help|'')
		usage
		;;
	on)
		xbee_start
		;;
	off)
		xbee_stop
		;;
	reset)
		xbee_reset
		;;
	*)
		usage
		exit 1
		;;
esac

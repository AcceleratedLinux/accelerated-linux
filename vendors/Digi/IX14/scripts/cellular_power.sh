#!/bin/sh
#
# Copyright 2018 Digi International Inc., All Rights Reserved
#
# This software contains proprietary and confidential information of Digi
# International Inc.  By accepting transfer of this copy, Recipient agrees
# to retain this software in confidence, to prevent disclosure to others,
# and to make no use of this software other than that for which it was
# delivered.  This is an unpublished copyrighted work of Digi International
# Inc.  Except as permitted by federal law, 17 USC 117, copying is strictly
# prohibited.
#
# Restricted Rights Legend
#
# Use, duplication, or disclosure by the Government is subject to
# restrictions set forth in sub-paragraph (c)(1)(ii) of The Rights in
# Technical Data and Computer Software clause at DFARS 252.227-7031 or
# subparagraphs (c)(1) and (2) of the Commercial Computer Software -
# Restricted Rights at 48 CFR 52.227-19, as applicable.
#
# Digi International Inc., 11001 Bren Road East, Minnetonka, MN 55343
#
# Description: Manage power sequence of different cellular modems
#

# Per board-id data
# <board-id> <ignition time> <power-off time> <power monitor time>
while read _id _ignit _pwoff _pwmon; do
	eval "id${_id}_ignit=\"${_ignit}\""
	eval "id${_id}_pwoff=\"${_pwoff}\""
	eval "id${_id}_pwmon=\"${_pwmon}\""
	AVAILABLE_BOARD_IDS="${AVAILABLE_BOARD_IDS:+${AVAILABLE_BOARD_IDS} }${_id}"
done<<-_EOF_
	155    10     15     10
	156    10     15     10
	157   1.5     30     25
	158   1.5     30     25
	159    10     15     10
_EOF_

# Cellular GPIOs
CEL_GPIO_CELL_EMERG_OFF="122"   # MX6UL_PAD_CSI_DATA05__GPIO4_IO26
CEL_GPIO_CELL_IGN="64"          # MX6UL_PAD_LCD_CLK__GPIO3_IO00
CEL_GPIO_CELL_PWR_EN="508"      # MCA IO4 (A_3)
CEL_GPIO_CELL_PWRMON="120"      # MX6UL_PAD_CSI_DATA03__GPIO4_IO24
CEL_GPIO_CELL_SW_READY="130"    # MX6UL_PAD_SNVS_TAMPER2__GPIO5_IO02

## NOTHING TO CUSTOMIZE BELOW THIS LINE ##

SW_SCRIPTNAME="$(basename ${0})"

# Color codes
GREEN="\033[1;32m"
RED="\033[1;31m"
YELLOW="\033[1;33m"
NONE="\033[0m"

#
# Display usage information
#
usage() {
	cat <<EOF

Usage: ${SW_SCRIPTNAME} MODE

MODE
    init     Initialize cellular interface
    on       Turn on the cellular interface
    off      Turn off the cellular interface
    forceon  Force power on the cellular module
    forceoff Force power off the cellular module
    help     This help message
EOF
}

#
# Print message with a red ERROR label prefix
#
# @param ${1} - Text to print
#
log_error() {
	[ ${#} -ne 0 ] && printf "${RED}[ERROR]:${NONE} %s\n" "${1}"
}

#
# Print message with a green INFO label prefix
#
# @param ${1} - Text to print
#
log_info() {
	[ ${#} -ne 0 ] && printf "${GREEN}[INFO]:${NONE} %s\n" "${1}"
}

#
# Print message with a yellow WARNING label prefix
#
# @param ${1} - Text to print
#
log_warning() {
	[ ${#} -ne 0 ] && printf "${YELLOW}[WARNING]:${NONE} %s\n" "${1}"
}

#
# Directly set the value of a gpio
#
gpio_set()
{
	echo $2 > /sys/class/gpio/gpio${1}/value
}
		
#
# Get the value of a GPIO
#
# @param ${1}  - Kernel GPIO number
#
get_gpio_value() {
	local GPIONR="${1}"
	local GPIOPATH="/sys/class/gpio/gpio${GPIONR}"

	[ -d "${GPIOPATH}" ] || printf "%s" "${GPIONR}" > /sys/class/gpio/export
	echo "$(cat "${GPIOPATH}/value")" && sleep .2
}

#
# Configure the value of a GPIO
#
# @param ${1}  - Kernel GPIO number
# @param ${2}  - GPIO value
#
set_gpio_value() {
	local GPIONR="${1}"
	local GPIOVAL="${2}"
	local GPIOPATH="/sys/class/gpio/gpio${GPIONR}"

	[ -d "${GPIOPATH}" ] || printf "%s" "${GPIONR}" > /sys/class/gpio/export
	[ "${GPIOVAL}" = "1" ] && GPIOVAL="high" || GPIOVAL="low"
	printf "${GPIOVAL}" > "${GPIOPATH}/direction" && sleep .2
}

#
# Check if the cellular is enabled (PWRMON active LOW)
#
is_cellular_enabled() {
	if [ "$(get_gpio_value ${CEL_GPIO_CELL_PWR_EN})" = "1" ]; then
		[ "$(get_gpio_value ${CEL_GPIO_CELL_PWRMON})" = "0" ] && true || false
	else
		false
	fi
}

#
# Check if the cellular interface is ready (SW_READY active HIGH)
#
is_cellular_ready() {
	[ "$(get_gpio_value ${CEL_GPIO_CELL_SW_READY})" = "1" ] && true || false
}

#
# Perform an unconditional shutdown of the modem
#
shutdown_cellular() {
	while true; do
		# Set RESET (HW_SHUTDOWN) to LOW (1) during 200ms
		set_gpio_value "${CEL_GPIO_CELL_EMERG_OFF}" "1" && sleep 0.2

		# Set RESET (HW_SHUTDOWN) to HIGH (0)
		set_gpio_value "${CEL_GPIO_CELL_EMERG_OFF}" "0"

		if is_cellular_enabled; then
			# Disconnect VBATT and wait 1 second
			set_gpio_value "${CEL_GPIO_CELL_PWR_EN}" "0" && sleep 1
		else
			return 0
		fi
	done
}

#
# Perform the power off sequence of the modem
#
poweroff_cellular() {
	is_cellular_enabled || return 0

	# Set ON_OFF to LOW (1) during 3 seconds
	set_gpio_value "${CEL_GPIO_CELL_IGN}" "1" && sleep 3

	# Set ON_OFF to HIGH (0)
	set_gpio_value "${CEL_GPIO_CELL_IGN}" "0"

	local START_TIME="$(date +%s)"
	while true; do
		local ELAPSED_TIME="$(($(date +%s) - START_TIME))"
		if is_cellular_enabled; then
			[ "${ELAPSED_TIME}" -gt "${CELL_PWOFF_TIME}" ] && break
			sleep 1
		else
			return 0
		fi
	done

	shutdown_cellular
}

#
# Perform the power on sequence of the modem
#
poweron_cellular() {
	# Set RESET (HW_SHUTDOWN) to HIGH (0) to disable it
	set_gpio_value "${CEL_GPIO_CELL_EMERG_OFF}" 0

	while true; do
		# Set CEL_GPIO_CELL_PWR_EN to HIGH (1) to enable modem supply (VBATT)
		# and wait 250ms until VBATT is higher than 3.10V
		set_gpio_value "${CEL_GPIO_CELL_PWR_EN}" 1 && sleep 0.25

		is_cellular_enabled && break

		# Set ON_OFF to LOW (1)
		set_gpio_value "${CEL_GPIO_CELL_IGN}" 1

		# Wait the initialization time
		sleep "${CELL_IGNIT_TIME}"

		# Set ON_OFF to HIGH (0)
		set_gpio_value "${CEL_GPIO_CELL_IGN}" 0

		# Additional sleep time for the powermon line to be ready
		for i in $(seq 1 ${CELL_PWMON_TIME}); do
		    sleep 1
		    if is_cellular_enabled; then
			break
		    fi
		done

		# Check whether cellular is enabled or force an unconditional shutdown
		if is_cellular_enabled; then
			break
		else
			shutdown_cellular
		fi
	done

	# Wait until the SW ready line is high.
	local RETRIES="5"
	while [ "${RETRIES}" -gt "0" ] && ! is_cellular_ready; do
		RETRIES="$((RETRIES - 1))"
		sleep 1
	done

	[ "${RETRIES}" -gt "0" ] && true || false
}

# Force a poweroff - used in firmware recovery
poweroff_force()
{
	# if all else fails, use the hardware power control
	# HARD SHUTDOWN
	echo "Turning off power to the modem."
	gpio_set $CEL_GPIO_CELL_PWR_EN 0
}

# Force a poweron, and don't wait - used in firmware recovery
poweron_automatic()
{
	# alternative power on method that simulates a system that does not use
	# ON/OFF power control.
	# turn the power on and do not wait for anything

	# hold the on_off line and then enable the power
	gpio_set $CEL_GPIO_CELL_PWR_EN 1 && sleep 0.1
}

init_cell_gpios()
{
	# First make sure the power is disabled
	set_gpio_value $CEL_GPIO_CELL_PWR_EN 0

	# Then set all control GPIOs to default state
	set_gpio_value $CEL_GPIO_CELL_EMERG_OFF 0
	set_gpio_value $CEL_GPIO_CELL_IGN 0
}

# Get board_id from OTP bits
BOARD_ID_OTP="/sys/bus/nvmem/devices/imx-ocotp0/nvmem"
[ -f "${BOARD_ID_OTP}" ] && BOARD_ID=$((($(hd -w -s 0x98 -m -n 1 -p ${BOARD_ID_OTP}) >> 4) & 0xFF))
if [ -z "${BOARD_ID}" ]; then
	BOARD_ID_OTP="/sys/fsl_otp/HW_OCOTP_GP1"
	[ -f "${BOARD_ID_OTP}" ] && BOARD_ID=$((($(cat ${BOARD_ID_OTP}) >> 4) & 0xFF))
fi

if echo "${AVAILABLE_BOARD_IDS}" | grep -qs "\<${BOARD_ID}\>"; then
	eval CELL_IGNIT_TIME="\${id${BOARD_ID}_ignit}"
	eval CELL_PWOFF_TIME="\${id${BOARD_ID}_pwoff}"
	eval CELL_PWMON_TIME="\${id${BOARD_ID}_pwmon}"
else
	# Provide some defaults in case the BOARD_ID is empty
	log_warning "board ID not detected, using cellular default time values"
	CELL_IGNIT_TIME="10"
	CELL_PWOFF_TIME="15"
	CELL_PWMON_TIME="0"
fi

MODE="$(echo "${1}" | tr '[:upper:]' '[:lower:]')"

# Sanity check
[ -z "${MODE}" ] && MODE="help"

case "${MODE}" in
	help)
		usage;;
	init)
		init_cell_gpios;;
	on)
		poweron_cellular && log_info "cellular successfully turned on";;
	off)
		poweroff_cellular && log_info "cellular successfully turned off";;
	forceoff)
		poweroff_force;;
	forceon)
		poweron_automatic;;
	*)
		log_error "${MODE} is an invalid option"
		usage && false;;
esac

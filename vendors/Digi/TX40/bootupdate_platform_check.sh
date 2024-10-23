#!/bin/sh

#
# bootupdate_platform_check.sh -- no auto-update on unlocked devices
#

PATH=/bin:/sbin:/usr/bin:/usr/sbin

log_err()
{
	logger -t bootloader_platform_check -p user.err "$1"
	echo "ERROR: $1"
}

log_info()
{
	logger -t bootloader_platform_check -p user.info "$1"
	echo "$1"
}

log_warn()
{
	logger -t bootloader_platform_check -p user.warn "$1"
	echo "WARN: $1"
}

is_locked() {
	# LOCKED/UNLOCKED status bit is bit3 in first byte of OTP bank
	val=$(hd -p -m -c -s0 -n1 /sys/bus/nvmem/devices/fsl-sfp0/nvmem) || {
		log_err "Failed to read OTP"
		return 1
	}

	if [ $((val & 0x04)) -eq 0 ]; then
		log_warn "Automatic bootloader update is disabled on unlocked units!"
		return 1
	fi

	return 0
}

# Using a signed bootloader on unlocked devices make a brick from the unit
# Return with OK-to-update only on locked units
is_locked

exit $?


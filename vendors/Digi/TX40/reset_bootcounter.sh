#!/bin/sh

readonly NVRAM_FILE="/sys/bus/nvmem/devices/max31329_nvram0/nvmem"
readonly BC_MAGIC="\xbc"

readonly LOGGER_ERROR="/usr/bin/logger -t dualboot -p user.error"

error_exit()
{
	$LOGGER_ERROR "$1"
	echo "dualboot ERROR: $1"
	exit 1
}

[ -f "$NVRAM_FILE" ] || error_exit "couldn't clear bootcount, NVRAM is not accessible"

echo -n -e "\x00${BC_MAGIC}" 2>/dev/null >"$NVRAM_FILE" || error_exit "there was a problem clearing bootcount in NVRAM"

#!/bin/sh
##############################################################################
# Copyright (c) 2019 Digi International Inc., All Rights Reserved
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
# Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
#
##############################################################################

readonly NVRAM_FILE="/sys/bus/i2c/devices/0-0068/ds3232_nvram0/nvmem"
readonly BC_MAGIC="\xbc"

readonly LOGGER_ERROR="/usr/bin/logger -t dualboot -p user.error"

error_exit()
{
	${LOGGER_ERROR} "$1"
	echo "dualboot ERROR: $1"
	exit 1
}

[ -f "${NVRAM_FILE}" ] || error_exit "couldn't clear bootcount, NVRAM is not accessible"

{ echo -n -e "${BC_MAGIC}\x00" > "${NVRAM_FILE}"; } 2>/dev/null
[ ${?} -eq 0 ] || error_exit "there was a problem clearing bootcount in NVRAM"

exit 0

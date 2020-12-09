#!/bin/sh
#
# Copyright 2018-2020 Digi International Inc., All Rights Reserved
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
# Digi International Inc., 9350 Excelsior Blvd., Suite 700, Hopkins, MN 55343
#
# Description: Clear the boot count attempts from the MCA NVRAM registers.
#

NVRAM_FILE="/sys/devices/soc0/soc/2100000.bus/21a0000.i2c/i2c-0/0-007e/nvram"

if [ ! -f "${NVRAM_FILE}" ]; then
	echo "Warning! Could not clear bootcount number, MCA NVRAM is not accessible"
	exit 1
fi

if ! printf '\x00\x00' > "${NVRAM_FILE}"; then
	echo "Warning! There was a problem clearing bootcount number from MCA NVRAM"
	exit 1
fi

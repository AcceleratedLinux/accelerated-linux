#!/bin/sh
#===============================================================================
#
#  clear-bootcount
#
#  Copyright (C) 2019 by Digi International Inc. All rights reserved.
#
#  This software contains proprietary and confidential information of Digi.
#  International Inc. By accepting transfer of this copy, Recipient agrees
#  to retain this software in confidence, to prevent disclosure to others,
#  and to make no use of this software other than that for which it was
#  delivered. This is an unpublished copyrighted work of Digi International
#  Inc. Except as permitted by federal law, 17 USC 117, copying is strictly
#  prohibited.
#
#  Restricted Rights Legend
#
#  Use, duplication, or disclosure by the Government is subject to restrictions
#  set forth in sub-paragraph (c)(1)(ii) of The Rights in Technical Data and
#  Computer Software clause at DFARS 252.227-7031 or subparagraphs (c)(1) and
#  (2) of the Commercial Computer Software - Restricted Rights at 48 CFR
#  52.227-19, as applicable.
#
#  Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
#
#
#  !Description: Clear the boot count attempts from the NVRAM registers.
#
#===============================================================================

BOOTCNTR_FILE="/sys/class/rtc/rtc0/device/alarm_seconds"
BOOTCNTR_CHECK_FILE="/sys/class/rtc/rtc0/device/alarm_minutes"

[ -f "${BOOTCNTR_FILE}" -a -f "${BOOTCNTR_CHECK_FILE}" ] || {
	echo "Warning! Could not clear bootcount number, NVRAM is not accessible"
	exit 1
}

echo 3 > "${BOOTCNTR_FILE}" && echo 86 > "${BOOTCNTR_CHECK_FILE}" || {
	echo "Warning! There was a problem clearing bootcount number in NVRAM"
	exit 1
}

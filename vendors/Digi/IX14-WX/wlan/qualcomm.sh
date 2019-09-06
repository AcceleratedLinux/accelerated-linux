#!/bin/sh
#
# Copyright (c) 2019, Digi International Inc.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, you can obtain one at http://mozilla.org/MPL/2.0/.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

# At this point of the boot (udev script), the system log (syslog) is not
# available yet, so use the kernel log buffer from userspace.
log() {
	printf "<$1>qca6564: $2\n" >/dev/kmsg
}

log "5" "Executing Qualcomm script"

# Do nothing if the module is already loaded
grep -qws 'wlan' /proc/modules
if [ "$?" = 0 ]; then
	log "5" "Module already loaded"
	exit 0
else
	log "5" "Loading wlan module..."
fi

FIRMWARE_DIR="/lib/firmware"
MACFILE="/etc/config/wlan/wlan_mac.bin"
TMP_MACFILE="$(mktemp -t wlan_mac.XXXXXX)"
 
# Read the MAC address from U-Boot environment.
mkdir -p /etc/config/wlan
MAC_ADDR=$(fw_printenv -n wlanaddr | sed "s/://g")
if [ -z "${MAC_ADDR}" ]; then
	log "3" "ERROR: MAC Address not provided!. Aborting use of Wi-Fi, MAC needs to be programmed"
	exit 1
fi
echo "Intf0MacAddress=${MAC_ADDR}" > ${TMP_MACFILE}

# Override the MAC firmware file only if the MAC file has changed.
if ! cmp -s ${TMP_MACFILE} ${MACFILE}; then
	cp ${TMP_MACFILE} ${MACFILE}
fi
rm -f "${TMP_MACFILE}"

# Load the wireless module with the params defined in modprobe.d/qualcomm.conf
modprobe wlan

# Verify the interface is present
if [ ! -d "/sys/class/net/wlan0" ]; then
	log "3" "[ERROR] Loading qca6564 module"
fi

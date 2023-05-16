#!/bin/sh
#
# Calculate VAP MAC addresses for hostapd, based on the qca-wifi driver's
# algorithm
# The Wi-Fi driver generates a locally administered MAC address for VAPs, except
# for the very first one, which gets the real, base MAC address
#

# 6 bits for locally administered MACs
readonly VAP_ID_MAX=63
readonly VAP_ID_MASK=0xFC
readonly LA_FLAG=0x02

is_number() {
	case "$1" in
	''|*[!0-9]*) return 1;;
	*) return 0;;
	esac
}

is_valid_mac() {
	case "$1" in
	[[:xdigit:]][[:xdigit:]]:[[:xdigit:]][[:xdigit:]]:[[:xdigit:]][[:xdigit:]]:[[:xdigit:]][[:xdigit:]]:[[:xdigit:]][[:xdigit:]]:[[:xdigit:]][[:xdigit:]])
		return 0;;
	*)
		return 1;;
	esac
}

usage() {
	echo """
	Usage:
   $(basename $0) <base_mac> <vap_index>
""" >&2
	exit 1
}

[ $# = 2 ] || usage

BASE_MAC="$1"
VAP_ID="$2"

is_valid_mac "$BASE_MAC" || {
	echo "ERROR: invalid MAC address" >&2
	usage
}

is_number "$VAP_ID" && [ "$VAP_ID" -le "$VAP_ID_MAX" ] || {
	echo "ERROR: invalid VAP index (max. $VAP_ID_MAX)" >&2
	usage
}

if [ "$VAP_ID" = 0 ]; then
	new_mac="$BASE_MAC"
else
	mac_first_octet="0x${BASE_MAC%%:*}"
	mac_rest="${BASE_MAC#*:}"

	new_first_octet="$((mac_first_octet & ~VAP_ID_MASK | LA_FLAG | (VAP_ID << 2)))"
	new_mac=$(printf "%02x:$mac_rest" "$new_first_octet")
fi

echo "$new_mac" | tr a-z A-Z

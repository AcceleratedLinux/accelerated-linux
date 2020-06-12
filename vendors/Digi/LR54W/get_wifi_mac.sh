#!/bin/sh
#
# This script generates MAC addresses for all valid Wi-Fi interfaces on the
# LR54W platform, which means:
# - MT7603: max. 4 interfaces, max. 1 client
# - MT7612: max. 8 interfaces, max. 1 client
#

usage()
{
	echo "Usage: $0 <module> <mode> <offset>" >&2
	echo "Where:" >&2
	echo "  <module> - '0' for Wi-Fi module 1; '1' for Wi-Fi module 2" >&2
	echo "  <mode>   - 'ap' or 'client'" >&2
	echo "  <offset> - interface offset (AP: [0-7]; client: [0])" >&2
	exit 1
}

MODULE=$1
MODE=$2
OFFSET=$3

case "${MODULE}" in
0|1)
	;;
*)
	usage
esac

case "${MODE}" in
ap|client)
	;;
*)
	usage
esac

# Use Wi-Fi module's MAC as base MAC
read -r BASE </sys/class/ieee80211/phy${MODULE}/macaddress
[ -n "${BASE}" ] || {
	echo "Error: couldn't get base MAC" 2>&1
	exit 1
}

if [ "${MODE}" = "ap" ]; then
	[ "${OFFSET}" -ge 0 -a "${OFFSET}" -le 7 ] 2>/dev/null || usage

	if [ "${OFFSET}" -le 3 ]; then
		# We have allocated MACs for the first 4 MACs
		setmac -e -a ${OFFSET} -n 1 -b ${BASE}
	else
		# For the other APs, have to generate a LAA (locally administered
		# address) from the base MAC
		# 'setmac' with offset 0 would return with the base MAC, so we translate
		# the offset to range [1 - ]
		setmac -e -a $((${OFFSET} - 3)) -n 1 -b ${BASE} -l
	fi
else
	# Only 1 client is supported
	[ "${OFFSET}" = 0 ] || usage

	# For that single interface, we'll always return with LAA offset 5
	setmac -e -a 5 -n 1 -b ${BASE} -l
fi

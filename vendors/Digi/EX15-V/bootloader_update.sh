#!/bin/sh
#
# TODO: in possible future bootloader updates, should check if user is coming
# from a non secure-boot bootloader ("boot_ver" == "Not Available"), or from a
# secure-boot migrated one ("boot_ver" == some version number).
#

PATH=/sbin:/bin:/usr/sbin:/usr/bin

#
# If we are coming from a non secure-boot bootloader, we have to fix up some
# of the ENV vars to not brick the board. It is not needed anymore otherwise, so
# we can freely leave this script
# NOTE: we have to keep this in future bootloader updates, as it is possible the
# board was still not migrated to support secure-boot.
#
current_bloader_ver=$(runt get manufacture.boot_ver)
[ "$current_bloader_ver" == "Not Available" ] || exit 0

EXTRA_ARGS=
add_extra_arg()
{
	EXTRA_ARGS="${EXTRA_ARGS}$1
"
}

error()
{
	logger -s -t "post_bootloader_update" -p user.err "$1"
}

#
# It is possible, user is updated from an old build, were we didn't set
# 'partx_valid' vars yet. Be sure to mark the current partition as valid
#
# If we are booting from TFTP, don't do this. A subsequent firmware update from
# this FW (which is already handling 'partx_valid') will fix it. Otherwise, user
# is able to boot from TFTP, so can fix it himself, it there's any issue
#
bootpart=$(runt get manufacture.booted_partition)
case "$bootpart" in
a)
	THIS_PARTITION=a
	OTHER_PARTITION=b
	;;
b)
	THIS_PARTITION=b
	OTHER_PARTITION=a
	;;
*)
	THIS_PARTITION=
	OTHER_PARTITION=
	;;
esac

[ "${THIS_PARTITION}" ] && add_extra_arg "part${THIS_PARTITION}_valid 1"

#
# It is highly unlikely, the other partition is a valid, signed one. With the
# current bootloader, we are migrating from a non secure-boot bootloader.
# In case we would leave the other partition marked as valid, if - because of
# some unfortunate reason - the bootloader's boot counter would expire, it would
# fall back to the (invalid) boot partition, and mark the single valid (signed)
# boot partition as invalid.
#
# !!! WARNING: it is not safe and advised to do, if we are coming from a secure-
# boot bootloader !!!
#

[ "${OTHER_PARTITION}" ] && add_extra_arg "part${OTHER_PARTITION}_valid 0"

# Clear no longer used scripts
fw_setenv -s - << EOF
netload
nandload
rootpart
parting
nandpart
sizing
$EXTRA_ARGS
EOF

[ $? = 0 ] || {
	error "Failed to update ENV vars"
	exit 1
}

exit 0

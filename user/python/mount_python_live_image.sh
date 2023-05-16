#!/bin/sh -e

PATH=/sbin:/bin:/usr/sbin:/usr/bin

readonly PYTHON_VERSION=$(ls /usr/bin | grep -E "python\d\.\d+$" | sed -e "s/python//")
readonly PYTHON_VERSION_SHORT=$(echo $PYTHON_VERSION | cut -d'.' -f1)

readonly SQFS_IMAGE=/opt/lib/live_images/python_live_image.sqfs
readonly MOUNT_DIR=/var/live_images/python/rootfs
readonly BIND_MOUNTS=" \
	/usr/lib/python$PYTHON_VERSION \
	/usr/lib/libpython$PYTHON_VERSION_SHORT.so \
	/usr/lib/libpython$PYTHON_VERSION.so.1.0 \
	/usr/bin/python$PYTHON_VERSION" # ensure last because it's mounted over this script

if [ ! -f "$SQFS_IMAGE" ]; then
	echo "Please install Python live image."
	exit 1
fi

mkdir -p $MOUNT_DIR
[ $(mount | grep -c "on $MOUNT_DIR") -eq 0 ] && mount $SQFS_IMAGE $MOUNT_DIR

for mount in $BIND_MOUNTS; do
	[ -e $mount ] && [ $(mount | grep -c "on $mount") -eq 0 ] && mount -o bind $MOUNT_DIR$mount $mount
done

#!/bin/sh
#
# So the busybox depmod script is not compatible with the system depmod,  but
# the busybox depmod is,  and we want to work with bothe depending on the config, so,
# let work around it all here.
#
# The problem is the kernel depmod wrapper adds /lib/modules to
# INSTALL_MOD_PATH,  which kmod depmod does not want,  its wants a basedir
# (ie the virtual / dir),  busybox expects /lib/modules to be add.
#
# Solution,  determine which one we are going to use and then fix
# INSTALL_MOD_PATH as needed.
#

. $CONFIG_CONFIG
if [ "$CONFIG_USER_KMOD" = "y" ]; then
	# we are using kmod for modules stuff,  so we must have the
	# modules.*.bin files or it won't work,  so we are using the system
	# depmod (/sbin/depmod)
	INSTALL_MOD_PATH="${INSTALL_MOD_PATH%%/lib/modules}"
	export INSTALL_MOD_PATH
	/sbin/depmod "$@"
else
	# make sure we have /lib/modules at the end (but only once)
	INSTALL_MOD_PATH="${INSTALL_MOD_PATH%%/lib/modules}"
	INSTALL_MOD_PATH="$INSTALL_MOD_PATH/lib/modules"
	export INSTALL_MOD_PATH
	$ROOTDIR/user/busybox/depmod.pl "$@"
	if [ "$CONFIG_USER_BUSYBOX_FEATURE_MODUTILS_ALIAS" = "y" ]; then
		find $ROMFSDIR/lib/modules -type f -name "*o" |
			/bin/sh $ROOTDIR/tools/modules-alias.sh $ROMFSDIR/etc/modprobe.conf
	fi
fi

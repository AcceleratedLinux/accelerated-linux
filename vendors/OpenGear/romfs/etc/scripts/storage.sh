#!/bin/bash
#
# Functions for bringing storage devices up and down.
#
# Configuration:
#
#  config.storage.S.enabled   -- Must be set to 'on'
#  config.storage.S.type      -- the <type> used below
#
# Infod:
#
#  storage.S.available        -- "yes" when mounted and /var/mnt/storage.S exists
#  storage.S.error            -- error message (when unavailable)
#  storage.S.directory        -- actual mount directory (when available)
#
# Depending on the <type>, there may be extra config vars and infod vars
# in use.
#
# The calling script should provide two callback functions for
# storage_up() and storage_down() to call to mount or unmount the
# storage volume from a local path. The callback functions have the
# following interface:
#
#	mount_<type>
#         Input variables:
#	    - $mnt     = the mount point path, guaranteed to not exist,
#                        always of the form "/var/tmp/storage.S". This may
#			 be created as a diretory or a symlink.
#	    - $storage = the storage ref, "storage.S"
#         Returns
#           - 0 on success
#	    - 1 on error; function may set $error
#
#	unmount_<type>
#         Input variables:
#	    - $mnt     = the directory/symlink mounted, eg "/var/tmp/storage.S"
#	    - $storage = the storage ref, "storage.S"
#
#         The <type>_unmount function should delete the directory/symlink $mnt.
#

# Defaults
storage_hook_scripts=(/etc/config/storage-notify /etc/scripts/storage-notify)

# Runs each of the existing hook scripts with the given args
runscripts () {
	for script in "${storage_hook_scripts[@]}"; do
		if [ -x "$script" ]; then
			"$script" "$@"
		fi
	done
}

# (How providers access their (read-only) configuration)

# retrieves config.$storage.field (will be cached in this session)
storage_getconf () { # field
	if [ "$STORAGE_CONFIG" != "$storage" ]; then
	    unset ${!STORAGE_CONFIG_*}
	    config -g config.$storage > /tmp/storage.$$
	    local var val
	    while read var val; do
	       eval export STORAGE_CONFIG_${var#config.$storage.}=\"\$val\"
	    done < /tmp/storage.$$
	    rm -f /tmp/storage.$$
	    STORAGE_CONFIG="$storage"
	fi
	if [ "$1" ]; then
	    eval echo \$STORAGE_CONFIG_$1
	fi
}

# (How providers save/retrieve their state:)

# retrieves an infod status for $storage
storage_getinfo () { # field
	infod_client -q -o get -p $storage.$1
}
# updates published info
storage_setinfo () { # field value
	infod_client -o push -t data -p $storage.$1 -d "$2"
}
# deletes published info
storage_delinfo () { # field value
	infod_client -o delete -p $storage.$1
}

# (Convenience wrappers for providers to wrap their mount_$type/umount_$type with)

# If the storage is up, brings it down
storage_down () {
	local storage=$1
	local mnt=/var/mnt/$1
	if [ -e $mnt ]; then
		local type="$(storage_getconf type)"
		logger -t storage -p daemon.info "detaching $storage"
		storage_setinfo available "no"
		# run the scripts first before unmounting
		runscripts remove $storage "$mnt"
		storage_delinfo error
		umount_$type
		if [ -d "$mnt/." ]; then
			logger -t storage -p daemon.warning "umount_$type did not remove $mnt/"
		fi
	fi
}

# If the storage is down, tries to bring it up
storage_up () {
	local storage=$1
	local mnt=/var/mnt/$1
	mkdir -p /var/mnt
	if [ ! -e $mnt ]; then
		local type="$(storage_getconf type)"
		local error
		logger -t storage -p daemon.info "attaching $storage"
		if mount_$type; then
			storage_setinfo available "yes"
			logger -t storage -p daemon.info "$storage ($devname) available"
			runscripts add $storage "$mnt"
		else
			if [ -z "$error" ]; then
				error="Unknown error"
			fi
			logger -t storage -p daemon.error "$storage: $error"
			storage_setinfo error "$error"
		fi
	fi
	return 0
}

# Used for debugging
if [ -e /etc/config/storage.sh ]; then
    . /etc/config/storage.sh
fi

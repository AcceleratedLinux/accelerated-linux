#!/bin/sh

# /sbin/update-apply.sh

# Write firmware update to flash and then reboot.  The file consists of
# a kernel image padded out to the kernel partition length plus a root
# filesystem image.

# We expect to find to partitions in the /proc/mtd list: Kernel and
# RootFS.  We extract the kernel partition length from this list.

# We expect to find two files in the /tmp folder: update.md5 and
# update.bin.  The md5sum of the bin file should match the content of
# the md5 file.

# The update file must be larger than the kernel partition length.  We
# write the first part of the update to the kernel partition, and whatever
# is left over gets written to the root filesystem partition.

binfile='/tmp/update.bin'
md5file='/tmp/update.md5'

rc=0

# First check everything we can before starting down the path of no
# return...

if [ ! -f "$binfile" ]; then 
	echo ">>> Update-Apply: did not find file '$binfile'."
  rc=1
else
	if [ ! -f "$md5file" ]; then
		echo ">>> Update-Apply: did not find file '$md5file'."
		rc=2
	else
		if [ `cat $md5file | cut -f1 -d' '` != `md5sum $binfile | cut -f1 -d' '` ]; then
			echo '>>> Update-Apply: md5 hash mismatch.'
			rc=3
    else
			binfile_size=`stat -c '%s' $binfile`
			kernel_hex=`cat /proc/mtd | grep '"Kernel"' | cut -f2 -d' '`
			kernel_size=`printf '%d' 0x0$kernel_hex`
			rootfs_hex=`cat /proc/mtd | grep '"RootFS"' | cut -f2 -d' '`
			rootfs_size=`printf '%d' 0x0$rootfs_hex`
			if [ $binfile_size -le $kernel_size ]; then
				echo ">>> Update-Apply: binfile:$binfile_size not larger than kernel:$kernel_size."
				rc=4
			fi
			echo ">>> Update-Apply: kernel:$kernel_size."
			echo ">>> Update-Apply: binfile:$binfile_size."
		fi
	fi
fi

# Last chance to bug out without damage...

if [ $rc -ne 0 ]; then
	echo '>>> Update-Apply: aborted without damage to unit.'
	exit $rc
fi

# Ok, all our ducks are in a row, now to do some damage...

offset1=0
length1=$kernel_size
offset2=$kernel_size
length2=$((binfile_size-kernel_size))

# Light fast blink.

gpio l 14 1 1 1 1 4000

# Write new firmware image data to flash.  This will take a
# minute or so.  If we loose power during this process, the
# box will likely be bricked.

/bin/mtd_write -o $offset1 -l $length1 write $binfile Kernel
if [ $? -ne 0 ]; then
  echo '>>> Update-Apply: write kernel failed!'
	rc=5
fi
/bin/mtd_write -o $offset2 -l $length2 write $binfile RootFS
if [ $? -ne 0 ]; then
  echo '>>> Update-Apply: write rootfs failed!'
	rc=6
fi

# Light off.

gpio l 14 1 4000 1 1 1

# If anything went wrong, then we are in deep doo-doo...

if [ $rc -ne 0 ]; then
  echo '>>> Update-Apply: critical error - this unit may be bricked!'
	exit $rc
fi

# All is well!

echo '>>> Update-Apply: flash update completed successfully.'

exit 0

# End

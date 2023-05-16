#!/bin/sh

#
# update-boot.sh -- install boot loader sections into NAND flash
#

PATH=/bin:/sbin:/usr/bin:/usr/sbin

BOOTADDRS=boot-address.txt
BOOTDEVICE=/dev/flash/boot

BOOTFILE=$1
BOOTARCHIVE=/tmp/boot.$$

#
# Check the boot loader update file actually exists.
#
if [ ! -f "$BOOTFILE" ]
then
	echo "ERROR: no boot loader file, $BOOTFILE, found"
	exit 1
fi

echo "update-boot: using boot loader file $BOOTFILE"

#
# Check that MTD devices are present. We need to determine the NAND flash
# page size in use. We may need to use different binary blobs based on
# that page size. Obviously if /proc/mtd is not present we have no flash
# to even attempt to program, so that is fatal. Currenty we only have
# support for 2k and 4k FLASH devices, so that is fatal here as well.
#
if [ ! -f /proc/mtd ]
then
	echo "ERROR: no MTD devices present?"
	exit 1
fi

BLKSIZE=0x$(grep '^mtd0' /proc/mtd | cut -d' ' -f3)
[ "$BLKSIZE" = 0x00020000 ] && PAGESIZE=2k
[ "$BLKSIZE" = 0x00040000 ] && PAGESIZE=4k
if [ "$PAGESIZE" = "" ]
then
	echo "ERROR: cannot determine FLASH page size?"
	exit 1
fi

#
# On some platforms we can determine the DDR RAM size, and that may be
# important for some blobs of the boot loader too. For the case of the
# IPQ6018 we support 512MB or 1GB configurations for example. In any case
# the fallback is the smallest configuration, 512MB (which is 256M x 16bit).
#
DDRSIZE=256m16
if [ -f /proc/device-tree/memory/reg ]
then
	RAMSIZE=$(hd -s12 -n1 -y /proc/device-tree/memory/reg | cut -d' ' -f2)
	[ "$RAMSIZE" = 40000000 ] && DDRSIZE=256m32
fi

#
# The boot loader binary is signed with the standard ECDSA Digi key.
# Check that it is valid, and extract the boot loader archive.
# We use netflash to do the heavy lifting. It will check the validity of
# the image (hardware, etc) and can also check the signing using the
# Atmel ATECC508 crypto element.
#
echo "update-boot: checking signature"

netflash -kb -R $BOOTARCHIVE.signed $BOOTFILE > /dev/null 2>&1
EXITCODE=$?

if [ "$EXITCODE" != "0" ]
then
	echo "ERROR: boot loader $BOOTFILE not valid ($EXITCODE)"
	exit 1
fi

# Remove signature from file.
head -c -68 $BOOTARCHIVE.signed > $BOOTARCHIVE

echo "update-boot: correctly signed boot loader file"

#
# Un-archive the boot loader sections into a tmp directory.
#
TMPDIR=/tmp/$$
rm -rf $TMPDIR
mkdir $TMPDIR
tar -xJ -f $BOOTARCHIVE -C $TMPDIR

#
# Make sure that the boot address ranges file is present in the archive.
# Without it we have no idea what sections to write or where.
#
if [ ! -f $TMPDIR/$BOOTADDRS ]
then
	echo "ERROR: missing $BOOTADDRS file"
	exit 1
fi

#
# Process each section and write it to flash. It is ok for a file not to
# be present for a section. Just means we don't update that. The primary
# use case for this is a boot loader update that only updates u-boot
# (and not all the other Qualcomm binary boot blobs).
#
SECTION_LIST=$(grep -v '^#' $TMPDIR/$BOOTADDRS | cut -d' ' -f1)
if [ -z "$SECTION_LIST" ]
then
	echo "ERROR: no boot loader sections to flash?"
	exit 1
fi

for SECTION in $SECTION_LIST
do
	FILE="$SECTION"
	[ -f "$TMPDIR/$SECTION.$PAGESIZE" ] && FILE="$SECTION.$PAGESIZE"
	[ -f "$TMPDIR/$SECTION.$DDRSIZE" ] && FILE="$SECTION.$DDRSIZE"
	if [ ! -f "$TMPDIR/$FILE" ]
	then
		continue
	fi

	echo "update-boot: processing $FILE"

	set $(grep "^$SECTION" $TMPDIR/$BOOTADDRS)
	START=$2
	SIZE=$3
	BLOCKS=$(($SIZE / $BLKSIZE))

	flash_erase $BOOTDEVICE $START $BLOCKS
	nandwrite -p -s $START $BOOTDEVICE $TMPDIR/$FILE
done

rm -f $BOOTARCHIVE $BOOTARCHIVE.signed
rm -rf $TMPDIR

exit 0

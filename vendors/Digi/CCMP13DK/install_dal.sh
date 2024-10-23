#!/bin/sh
#===============================================================================
#
#  Copyright (C) 2023 by Digi International Inc.
#  All rights reserved.
#
#  This program is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License version 2 as published by
#  the Free Software Foundation.
#
#
#  Description:
#    Script to flash DAL build artifacts over USB to the target.
#===============================================================================
# set -x

#
# U-Boot script for installing Linux images created by DAL.
# Assumes that the target is running DEY's U-Boot.
#

# Exit on any error
set -e

check_cmd()
{
	uuu -v "fb[-t ${2:-1000}]:" ${3:-acmd} ${1} > /dev/null 2> /dev/null
	uuu -v fb: ucmd echo retval=\$? | sed  -ne "s,^retval=,,g;T;p" | tr -d '\r'
}

show_usage()
{
	echo "Usage: $0 [options]"
	echo ""
	echo "  Options:"
	echo "   -a <atf-filename>      Arm-trusted-firmware filename."
	echo "                          Auto-determined by variant if not provided."
	echo "   -f <fip-filename>      FIP filename."
	echo "                          Auto-determined by variant if not provided."
	echo "   -h                     Show this help."
	echo "   -n                     No wait. Skips 10 seconds delay to stop script."
	echo "   -r                     Force the re-partitioning of the NAND flash."
	echo "   -o (boot|image|images) Only install one half of the files: 'boot' (TF-A, OP-TEE, U-Boot) or"
	echo "                          'images' (write image.bin into image/image1 volumes)."
	echo "                          Or 'image' to only write the first image (quicker)."
	exit 2
}

# Update a partition
#   Params:
#	1. partition
#	2. file
#	3. timeout (in ms)
#   Description:
#	- downloads image to RAM
#	- runs 'update' command from RAM
part_update()
{
	echo "\033[36m"
	echo "====================================================================================="
	echo "Updating '${1}' partition with file: ${2}"
	echo "====================================================================================="
	echo "\033[0m"

	uuu fb: download -f "${2}"
	uuu "fb[-t ${3}]:" ucmd update "${1}" ram \${fastboot_buffer} \${filesize} ${ERASE}
}

# clear
echo "############################################################"
echo "#           Linux firmware install through USB OTG         #"
echo "############################################################"

ONLY=""

# Command line admits the following parameters:
# -a <atf-filename>
# -f <fip-filename>
# -o (boot|linux)
while getopts 'a:f:hi:nrwo:' c
do
	case $c in
	a) INSTALL_ATF_FILENAME=${OPTARG} ;;
	f) INSTALL_FIP_FILENAME=${OPTARG} ;;
	h) show_usage ;;
	n) NOWAIT=true ;;
	r) RUNVOLS=true ;;
	o)
		case "${OPTARG}" in
			boot|image|images)
				ONLY=${OPTARG} ;;
			*)
				show_usage ;;
		esac
		;;
	esac
done

# Enable the redirect support to get u-boot variables values
uuu "fb[-t 5000]:" ucmd setenv stdout serial,fastboot

# Check if uboot_config volume exists (U-Boot env)
uuu "fb[-t 15000]:" ucmd ubi part UBI
if [ \
	"$(check_cmd 'ubi check uboot_config')" = "1" \
	-o "$(check_cmd 'ubi check image')" = "1" \
	-o "$(check_cmd 'ubi check image1')" = "1" \
]; then
	RUNVOLS=true
fi
uuu "fb[-t 15000]:" ucmd ubi part UBI_2
if [ \
	"$(check_cmd 'ubi check opt')" = "1" \
	-o "$(check_cmd 'ubi check config')" = "1" \
]; then
	RUNVOLS=true
fi

# remove redirect
uuu fb: ucmd setenv stdout serial

echo ""
echo "Determining image files to use..."

# Determine ATF file to program
if [ -z "${INSTALL_ATF_FILENAME}" ]; then
	INSTALL_ATF_FILENAME="tf-a-ccmp13-dvk.stm32"
fi

# Determine FIP file to program
if [ -z "${INSTALL_FIP_FILENAME}" ]; then
	INSTALL_FIP_FILENAME="fip.bin"
fi

INSTALL_IMAGE_FILENAME="image.bin"

cd images/

# Verify existance of files before starting the update
FILES_TO_CHECK="${INSTALL_ATF_FILENAME} ${INSTALL_FIP_FILENAME}"
if [ "$ONLY" != "boot" ]; then
	FILES_TO_CHECK="${FILES_TO_CHECK} ${INSTALL_IMAGE_FILENAME}"
fi
for f in ${FILES_TO_CHECK}; do
	if [ ! -f ${f} ]; then
		echo "\033[31m[ERROR] Could not find file '${f}'\033[0m"
		ABORT=true
	fi
done;

[ "${ABORT}" = true ] && exit 1

# Print warning about storage media being deleted
if [ "${NOWAIT}" != true ]; then
	WAIT=10
	printf "\n"
	printf " ====================\n"
	printf " =    IMPORTANT!    =\n"
	printf " ====================\n"
	printf " This process will erase your NAND and will install the following files\n"
	printf " on the partitions of the NAND.\n"
	printf "\n"
	printf "   PARTITION\tFILENAME\n"
	printf "   ---------\t--------\n"
	printf "   fsbl1\t${INSTALL_ATF_FILENAME}\n"
	printf "   fsbl2\t${INSTALL_ATF_FILENAME}\n"
	printf "   fip-a\t${INSTALL_FIP_FILENAME}\n"
	printf "   image\t${INSTALL_IMAGE_FILENAME}\n"
	printf "   image1\t${INSTALL_IMAGE_FILENAME}\n"
	printf "\n"
	printf " Press CTRL+C now if you wish to abort.\n"
	printf "\n"
	while [ ${WAIT} -gt 0 ]; do
		printf "\r Update process starts in %d " ${WAIT}
		sleep 1
		WAIT=$(( ${WAIT} - 1 ))
	done
	printf "\r                                   \n"
	printf " Starting update process\n"
fi

# Set fastboot buffer address to $loadaddr, just in case
uuu fb: ucmd setenv fastboot_buffer \${loadaddr}

# Skip user confirmation for U-Boot update
uuu fb: ucmd setenv forced_update 1

if [ -z "$ONLY" -o "$ONLY" = "boot" ]; then
	# Update ATF
	part_update "fsbl1" "${INSTALL_ATF_FILENAME}" 5000
	part_update "fsbl2" "${INSTALL_ATF_FILENAME}" 5000

	# Update FIP
	part_update "fip-a" "${INSTALL_FIP_FILENAME}" 5000
fi

# Environment volume does not exist and needs to be created
if [ "${RUNVOLS}" = true ]; then
	echo "\033[36m"
	echo "====================================================================================="
	echo "Re-partitioning the NAND flash"
	echo "====================================================================================="
	echo "\033[0m"
	# Create UBI volumes
	uuu "fb[-t 45000]:" ucmd run ubivolscript

	# We must reinstall the DAL images.
	ONLY=""
fi

if [ "$ONLY" != "boot" ]; then
	# Set 'bootcmd' for the second part of the script that will
	#  - Reset environment to defaults
	#  - Save the environment
	#  - Update the 'image' and 'image1' partition(s)
	case "$ONLY" in
		image|image1|images) ;;
		*)
			uuu fb: ucmd setenv bootcmd "
				env default -a;
				saveenv;
				saveenv;
				echo \"\";
				echo \"\";
				echo \">> Installing DAL firmware\";
				echo \"\";
				echo \"\";
				fastboot 0;
			"

			uuu "fb[-t 15000]:" ucmd saveenv
			uuu fb: acmd reset

			# Wait for the target to reset
			sleep 8
			;;
	esac

	# Set fastboot buffer address to $loadaddr
	uuu fb: ucmd setenv fastboot_buffer \${loadaddr}

	# Update firmware partitions
	part_update image  "${INSTALL_IMAGE_FILENAME}" 120000
	uuu fb: ucmd setenv parta_valid 1
	uuu fb: ucmd setenv bootpart a
	if [ "$ONLY" = "image" ]; then
		uuu fb: ucmd setenv partb_valid 0
	else
		part_update image1 "${INSTALL_IMAGE_FILENAME}" 120000
		uuu fb: ucmd setenv partb_valid 1
	fi
	uuu fb: ucmd setenv extra_bootargs # earlycon initcall_debug ignore_loglevel=1
	uuu "fb[-t 15000]:" ucmd saveenv
fi

# Reset the target
uuu fb: acmd reset

echo "\033[32m"
echo "============================================================="
echo "Done! Wait for the target to complete first boot process."
echo "============================================================="
echo "\033[0m"

exit

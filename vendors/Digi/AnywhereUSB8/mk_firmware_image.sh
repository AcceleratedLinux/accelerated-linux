#!/bin/sh
#
# Firmware update file structure:
# - first 32-byte is the file's signature
# - rest is the tar.gz packed firmware image
#
# Tar file's content:
# - version:			firmware file version
# - signer_cert.pem:	signature cert chain
# - pre_update.sh:		pre-update script
# - post_update.sh:		post-update script
# - u-boot.bin:			bootloader (make sure it's swapped) (unlikely)
# - vmlinuz:			kernel image
# - vmlinuz.sig:		kernel image signature
# - rootfs:				root filesystem image
# - rootfs.sig:			root filesystem image signature
# - rootfs.size:		root filesystem image size info file
# - rootfs.size.sig:	root filesystem image size info file signature
#

BOOTLOADER_FILE="u-boot.bin"
KERNEL_FILE="kernel.bin"
ROOTFS_FILE="awusb-image-awusb1012.squashfs"
PRE_UPDATE_FILE="pre_update.sh"
POST_UPDATE_FILE="post_update.sh"

FIRMWARE_TAR="firmware.tgz"
BUILDDIR=$(pwd)
ROOTDIR=$(pwd)/../../..

set -e

usage_and_exit() {
	echo
	echo "Usage:"
	echo "$(/usr/bin/basename $0) <fw_version_file> <cert> <kernel> <rootfs> <image_name> <deploy_dir> <options>"
	echo
	echo "<options>:"
	echo "  -b:   add bootloader image to the firmware"
	echo "  -f:   add kernel + rootfs image to the firmware"
	echo "  -e:   add pre-update script"
	echo "  -o:   add post-update script"
	echo "  -p:   production image generation"
	echo

	exit 1
}

error() {
	echo "ERROR: $1"
	exit 1
}

copy_file_to_tmp() {
	local src_name="$1"
	local dst_name="$2"

	if ! cp "${IMAGE_DIR}/$src_name" "${TEMP_DIR}/$dst_name" 2>/dev/null
	then
		if ! cp "$src_name" "${TEMP_DIR}/$dst_name" 2>/dev/null
		then
			error "couldn't copy file '$src_name'"
			exit 1
		fi
	fi

	TAR_FILE_LIST="$TAR_FILE_LIST $dst_name"
}

add_production_signature() {
	${ROOTDIR}/prop/sign_image/sign_awusb_firmware.py ${FIRMWARE_TAR} ${ROOTDIR}/romfs awusbg3-firmware || return 1
	printf "\nUPDATE_SIZE=%s" `stat -t $FIRMWARE_TAR | awk '{printf $2}'` >update.size
	printf "|SIG_SIZE=%s\n" `stat -t ${FIRMWARE_TAR}.sig | awk '{printf $2}'` >>update.size
	cat $FIRMWARE_TAR ${FIRMWARE_TAR}.sig update.size > $FIRMWARE_IMAGE
	rm -f update.* ${FIRMWARE_TAR}.sig
	return 0
}

add_signature() {
	local sig_file="$1"
	HASH=$(openssl dgst -sha256 <$FIRMWARE_TAR)
	echo "${HASH#(stdin)= }" >update.hash

	openssl rsautl -sign -inkey $(echo ${SIGNER_CERT} | sed 's/\.pem$/_key.pem/') -keyform PEM -in update.hash >update.sig
	printf "\nUPDATE_SIZE=%s" `stat -t $FIRMWARE_TAR | awk '{printf $2}'` >update.size
	printf "|SIG_SIZE=%s\n" `stat -t update.sig | awk '{printf $2}'` >>update.size
	cat $FIRMWARE_TAR update.sig update.size > $FIRMWARE_IMAGE
	rm -f update.* signer_cert.pem
	return 0
}

unset TEMP_DIR

cleanup() {
	trap - TERM EXIT INT

	[ -n "$TEMP_DIR" ] && rm -rf "$TEMP_DIR"

	[ "$1" = "error" ] && {
		echo "Unexpected ERROR"
		exit 1
	}

	return 0
}

[ $# -lt 5 ] && usage_and_exit

# Full path to version file is expected
VERSION_FILE="$1"
shift

# Full path to signer_cert is expected
SIGNER_CERT="$1"
shift

KERNEL_FILE="$1"
shift

ROOTFS_FILE="$1"
shift

# Full path to output file is expected
FIRMWARE_IMAGE="$1"
IMAGE_DIR=$(/usr/bin/dirname "${FIRMWARE_IMAGE}")
shift

DEPLOY_DIR=$(/bin/readlink -f "$1")

[ -d "$DEPLOY_DIR" ] || {
	echo "ERROR: invalid deploy_dir"
	usage_and_exit
}

shift

unset ADD_BOOTLOADER
unset ADD_FIRMWARE
unset ADD_PRE_UPDATE
unset ADD_POST_UPDATE
unset PRODUCTION

while [ $# -ne 0 ]; do
	case "$1" in
		"-b") ADD_BOOTLOADER=1 ;;

		"-f") ADD_FIRMWARE=1 ;;

		"-e") ADD_PRE_UPDATE=1 ;;

		"-o") ADD_POST_UPDATE=1 ;;

		"-p") PRODUCTION=1 ;;

		*) echo "ERROR: invalid argument '$1'"; usage_and_exit ;;
	esac

	shift
done

trap 'cleanup error;' TERM EXIT INT

TEMP_DIR="$(/bin/mktemp -d $(pwd)/fw-update.XXXXXXXXXX)"

cd "$DEPLOY_DIR"

unset TAR_FILE_LIST

# Create FW version file
copy_file_to_tmp "${VERSION_FILE}" "version"

# Signer cert
copy_file_to_tmp "${SIGNER_CERT}" "signer_cert.pem"

[ -n "$ADD_BOOTLOADER" ] && {
	# Copy bootloader
	copy_file_to_tmp "${BOOTLOADER_FILE}" "${BOOTLOADER_FILE}"
}

[ -n "$ADD_FIRMWARE" ] && {
	# Copy kernel
	copy_file_to_tmp "${KERNEL_FILE}" "kernel"

	# Copy kernel signature
	copy_file_to_tmp "${KERNEL_FILE}.sig" "kernel.sig"

	# Copy rootfs
	copy_file_to_tmp "${ROOTFS_FILE}" "rootfs"

	# Copy rootfs signature
	copy_file_to_tmp "${ROOTFS_FILE}.sig" "rootfs.sig"

	# Copy rootfs size info
	copy_file_to_tmp "${ROOTFS_FILE}.size" "rootfs.size"
}

[ -n "$ADD_PRE_UPDATE" ] && {
	# Copy pre-update script
	copy_file_to_tmp "${PRE_UPDATE_FILE}" "${PRE_UPDATE_FILE}"
}

[ -n "$ADD_POST_UPDATE" ] && {
	# Copy post-update script
	copy_file_to_tmp "${POST_UPDATE_FILE}" "${POST_UPDATE_FILE}"
}

# Everything in place, create tar package
cd "$TEMP_DIR"

if ! /bin/tar cvfz "$FIRMWARE_TAR" $TAR_FILE_LIST >/dev/null; then
	error "couldn't pack firmware file"
	exit 1
fi

# Generate signature file
if [ -n "${PRODUCTION}" ]; then

if ! add_production_signature ; then
	error "couldn't generate signature"
	exit 1
fi

else

if ! add_signature "${FIRMWARE_TAR}.sig"; then
	error "couldn't generate signature"
	exit 1
fi

fi

echo
echo "SUCCESS: '$(basename $FIRMWARE_IMAGE)' file is generated!"
echo

cleanup

exit 0

#!/bin/sh

set -e

readonly PRE_UPDATE_FILE="pre_update.sh"
readonly POST_UPDATE_FILE="post_update.sh"

readonly FIRMWARE_TAR="firmware.tgz"

unset TEMP_DIR

error()
{
	echo "ERROR: $1"
	cleanup
	exit 1
}

cleanup()
{
	trap - TERM EXIT INT

	[ -n "$TEMP_DIR" ] && rm -R "$TEMP_DIR"

	[ "$1" = "error" ] && echo "Unexpected ERROR"

	return 0
}

add_to_file_list()
{
	TAR_FILE_LIST="${TAR_FILE_LIST} $1"
}

copy_file_to_tmp()
{
	local src_name="$1"
	local dst_name="$2"

	cp "${src_name}" "${TEMP_DIR}/${dst_name}" 2>/dev/null ||
		error "couldn't copy file '${src_name}'"

	add_to_file_list "${dst_name}"
}

add_signature()
{
	local HASH
	HASH=$(openssl dgst -sha256 < "${FIRMWARE_TAR}")
	echo "${HASH#(stdin)= }" >update.hash
	openssl rsautl -sign -inkey $(echo "${SIGNER_CERT}" | sed 's/\.pem$/_key.pem/') -keyform PEM -in update.hash >update.sig
	printf "\nUPDATE_SIZE=%s" `stat -t "${FIRMWARE_TAR}" | awk '{printf $2}'` >update.size
	printf "|SIG_SIZE=%s\n" `stat -t update.sig | awk '{printf $2}'` >>update.size
	cat "${FIRMWARE_TAR}" update.sig update.size > "${FIRMWARE_IMAGE}"
}

usage_and_exit()
{
	echo
	echo "Usage:"
	echo "$(/usr/bin/basename $0) <product> <product_version> <build_string> <signer_cert> <firmware_filename> <rootdir>"
	echo

	exit 1
}

[ $# != 6 ] && usage_and_exit

PRODUCT="$1"
shift
PRODUCT_VERSION="$1"
shift
BUILD_STRING="$1"
shift
SIGNER_CERT="$1"
shift
FIRMWARE_IMAGE="$1"
shift
ROOTDIR="$1"
shift

trap "cleanup error" TERM EXIT INT

TEMP_DIR="$(/bin/mktemp -d /tmp/xosfw.XXXXXXXXXX)"

TAR_FILE_LIST=

# Generate version file
cat <<EOF > "${TEMP_DIR}/version"
XOS_PRODUCT=${PRODUCT}
PRODUCT_VERSION=${PRODUCT_VERSION}
PRODUCT_BUILDSTRING="${BUILD_STRING}"
SUPPORTED_PRODUCT_IDS=
BASIC_VERSION="${BUILD_STRING}"
MIN_RUNNING_RELEASE=
EOF
add_to_file_list "version"

# Copy signer cert
copy_file_to_tmp "${SIGNER_CERT}" "signer_cert.pem"

# Dummy pre-update file
cat <<EOF > "${TEMP_DIR}/${PRE_UPDATE_FILE}"
#!/bin/sh
exit 0
EOF
chmod +x "${TEMP_DIR}/${PRE_UPDATE_FILE}"
add_to_file_list "${PRE_UPDATE_FILE}"

# Dummy post-update file
cat <<EOF > "${TEMP_DIR}/${POST_UPDATE_FILE}"
#!/bin/sh
exit 0
EOF
chmod +x "${TEMP_DIR}/${POST_UPDATE_FILE}"
add_to_file_list "${POST_UPDATE_FILE}"

# Copy kernel
copy_file_to_tmp "${ROOTDIR}/images/kernel.nand" "uImage"

# Copy rootfs
copy_file_to_tmp "${ROOTDIR}/images/rootfs.bin" "rootfs"

# Create tar package
cd "${TEMP_DIR}"

tar cvzf "${FIRMWARE_TAR}" ${TAR_FILE_LIST} >/dev/null ||
	error "couldn't pack firmware file"

# Add signature, and generate final firmware image
add_signature

echo "XOS firmware file '${FIRMWARE_IMAGE}' successfully generated"

cleanup
exit 0

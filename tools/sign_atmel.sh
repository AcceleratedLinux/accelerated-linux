#!/bin/sh
#
# Copyright (c) 2018, Digi International Inc.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, you can obtain one at http://mozilla.org/MPL/2.0/.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# Description: Generate ECDSA signature for an image.bin ACL artifact and
#              attach it to the file itself, right after the blob.
#

SCRIPTNAME="$(basename "${0}")"
ATMEL_SIGN_MAGIC="54361782"
# Header (magic int) + r (32 bytes) + s (32 bytes)
ATMEL_SIGN_LENGTH=$((4 + 32 + 32))

# 'printf' command from sh is not complete enough
PRINTF=$(which printf)

usage() {
	cat <<EOF

Generate ECDSA signature for an image.bin ACL artifact and attach it to
the file itself, right after the blob.
If a signature is already present, it will get overwritten.


Usage: ${SCRIPTNAME} [OPTIONS] [FILE]

[OPTIONS]
    -k,--key <path>	Private EC key.

[FILE]
    image.bin to be signed.

EOF
}

SHORT_OPTS="k:"
LONG_OPTS="key:"
CMDLINE_OPTS=$(getopt -o ${SHORT_OPTS} -l ${LONG_OPTS} -- "$@") || { usage; exit 1; }

eval set -- "${CMDLINE_OPTS}"

while true; do
	case "$1" in
		-k|--key) KEY="${2}"; shift;;
		--) shift; break;;
	esac
	shift
done

FILE="${1}"

if [ ! -r "${KEY}" ] ||  [ ! -s "${KEY}" ]; then
	echo "Bad key: ${KEY}" >&2
	exit 1
fi
if [ ! -r "${FILE}" ] ||  [ ! -s "${FILE}" ]; then
	echo "Bad file ${FILE}" >&2
	exit 1
fi

# Read and round SquashFS size (4 bytes at offset 0x28=40)
fssize=$(hexdump -n 4 -s 40 -e '/4 "0x%08x"' "${FILE}")
fssize=$(((fssize  + 0xfff) & 0xfffff000))

# Read uImage size (4 bytes at offset 0x0C=12)
usize_offset=$((fssize + 12))
usize="0x$(dd status=none if="${FILE}" skip=${usize_offset} count=4 bs=1 | hexdump -n 4 -e '/4 "%08x"' | rev | dd status=none conv=swab)"

# Blob size = SquashFS + uImage header + uImage size
blob_size=$((fssize + 0x40 + usize))

# Anything appended to the blob will need to be reappended later, as we need to insert
# the atmel signature here. Dump that content to a temporal file
not_signed_tmp="$(mktemp)"
not_signed_offset="${blob_size}"

# Check that the image is not signed already
hdr="$(dd status=none if="${FILE}" skip=${blob_size} count=4 bs=1 | hexdump -n 4 -e '/4 "%08x"' | rev | dd status=none conv=swab)"
if [ "$hdr" = "$ATMEL_SIGN_MAGIC" ]; then
	echo "WARNING: $FILE already signed, removing existing signature" >&2
	not_signed_offset=$((not_signed_offset + ATMEL_SIGN_LENGTH))
fi

# Copy extra content to the temporal file, truncate image.bin to the blob, add the signature, then reappend any extra content.
dd status=none iflag=skip_bytes if="${FILE}" skip=${not_signed_offset} of="${not_signed_tmp}"
truncate --size=${blob_size} "${FILE}"

SIGN="$(openssl dgst -sign "${KEY}" "${FILE}" | openssl asn1parse -inform DER | grep INTEGER | awk -F: ' { print $4 } ' | tr -d '[:space:]')" || exit 1

{
	echo "${ATMEL_SIGN_MAGIC}" | xxd -r -p
	echo "${SIGN}" | xxd -r -p
	cat "${not_signed_tmp}"
} >> "${FILE}"

not_signed_size=$(stat --printf="%s" "${not_signed_tmp}")
rm -rf "${not_signed_tmp}"

# If there was a checksum in the image, correct it
if [ ${not_signed_size} -gt 0 ]; then
	# Remove previous checksum
	truncate --size=-4 "${FILE}"

	# Recompute and append new checksum
	crc=$(sum -s "${FILE}" |  cut -d ' '  -f1)
	$PRINTF $($PRINTF "%08x" $crc | sed 's/.\{2\}/&\n/g' | sed 's,^,\\x,g' | tr -d '\n') >> "${FILE}"
fi

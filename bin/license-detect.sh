#!/bin/sh
#
# try and determine the source license that applies to
# the given directory
#
# Everything under vendors is considered GPLv2 unless it explicitly
# set it.
#

COMMON="*COPYRIGHT* *GPL* AUTHORS COPYING* *LICENSE* *LICENCE* *README*"
COMMON_SRC="*.c"

license_detect() {
	#
	# GPL is pretty easy to find
	#
	GPL=$(egrep -m 1 -iA 3 'GNU.*GENERAL PUBLIC LICENSE' "$1" | tr '[a-z]' '[A-Z]' | tr -d '\n')
	case "$GPL" in
	*"GNU L"*" GENERAL PUBLIC LICENSE"*"VERSION 2"*)
		echo "LGPLv2"; return 0 ;;
	*"GNU L"*" GENERAL PUBLIC LICENSE"*"VERSION 3"*)
		echo "LGPLv3"; return 0 ;;
	*"GNU GENERAL PUBLIC LICENSE"*"VERSION 1"*)
		echo "GPLv1"; return 0 ;;
	*"GNU GENERAL PUBLIC LICENSE"*"VERSION 2"*)
		echo "GPLv2"; return 0 ;;
	*"GNU GENERAL PUBLIC LICENSE"*"VERSION 3"*)
		echo "GPLv3"; return 0 ;;
	esac
	#
	# SPDX is short hand way of describing a license
	#
	GPL=$(egrep -m 1 -i 'SPDX-License-Identifier' "$1" | tr '[a-z]' '[A-Z]' | tr -d '\n')
	case "$GPL" in
	*"SPDX-LICENSE-IDENTIFIER:"*"GPL-2"*)
		echo "GPLv2"; return 0 ;;
	*"SPDX-LICENSE-IDENTIFIER:"*"GPL-3"*)
		echo "GPLv3"; return 0 ;;
	*"SPDX-LICENSE-IDENTIFIER:"*"BSD-3-CLAUSE"*)
		echo "BSD (3 clause)"; return 0 ;;
	esac
	#
	# BSD is a bit trickier but lets try
	#
	BSD=$(egrep -B 20 -A 20 'TH.* SOFTWARE IS PROVIDED.*AS IS' "$1" | tr -d '\n')
	case "$BSD" in
	"") ;;
	*"used to endorse or promote products"*)
		echo "BSD (3 clause)"; return 0 ;;
	*"Redistributions in binary form"*)
		echo "BSD (2 clause)"; return 0 ;;
	*)
		echo "BSD like " ; return 0 ;;
	esac
	#
	# Apache license
	#
	APACHE=$(egrep -iA 3 'Apache License' "$1" | tr '[a-z]' '[A-Z]' | tr -d '\n')
	case "$APACHE" in
	*"APACHE LICENSE"*"VERSION 2"*)
		echo "ApacheV2" ; return 0 ;;
	esac
	#
	# Public Domain
	#
	if grep -q "PUBLIC DOMAIN" "$1"; then
		echo "PublicDomain" ; return 0
	fi
	#
	# ISC license
	#
	if grep -iq "ISC license" "$1"; then
		echo "ISC" ; return 0
	fi
	#
	# SQLITE is odd, and free (public domain)
	#
	if grep -iq "The author disclaims copyright" "$1"; then
		echo "PublicDomain" ; return 0
	fi
	# no luck
	#
	return 255
}

[ "$1" ] && cd "$1"

PATTERN="-iname $(echo $COMMON | sed 's/ / -o -iname /g')"
PATTERN_SRC="-iname $(echo $COMMON_SRC | sed 's/ / -o -iname /g')"
(
	find . -depth \( $PATTERN_SRC \) -print 2> /dev/null
	find . -depth \( $PATTERN \) -print 2> /dev/null
) | tac | while read i; do
	[ -f "$i" ] || continue
	license_detect "$i" && exit 0
done

#
# handle vendors/*/*/* case when nothing better was found
#
case "`pwd`" in
*/vendors/*/*) echo GPLv2 ;;
esac

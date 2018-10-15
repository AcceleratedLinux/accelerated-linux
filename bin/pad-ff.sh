#!/bin/sh
#
# pad stdin to "$1" k using FF for padding

SIZE="$1"
shift

(cat $* || exit 1; dd conv=sync if=/dev/zero bs=1024 count=$SIZE 2> /dev/null |
	tr '[\000]' '[\377]') |
	dd conv=sync bs=1024 count=$SIZE 2> /dev/null

exit $?

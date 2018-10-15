#!/bin/sh

# /sbin/wget-file.sh

# Get a file using wget, with retry.  Return zero on
# success, 1 on failure.

rm -f "$2"
wget -q "$1" -O "$2" 2> /dev/null
if [ $? -eq 0 ]; then
	exit 0
fi

rm -f "$2"
wget -q "$1" -O "$2" 2> /dev/null
if [ $? -eq 0 ]; then
	exit 0
fi

rm -f "$2"

exit 1

# End


#!/bin/sh

# /sbin/syslog.sh

if [ -f /tmp/flag.syslog ]; then
	syslog "$1" "$2" "$3"
fi

# End


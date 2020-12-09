#!/bin/sh

if [ "$(fw_printenv -n bootdelay)" = 0 ]; then
	# Write it twice to make sure it gets written into both banks
	count=2
	while [ $count -gt 0 ]; do
		fw_setenv -s - <<EOF
bootdelay -2
silent 1
EOF
		count=$((count - 1))
	done
fi

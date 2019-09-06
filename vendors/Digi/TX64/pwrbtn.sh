#!/bin/sh

# Make power LED blinking as soon as possible
ledcmd -F POWER

# Check if factory reset was requested
if nexcom-tool -f 2> /dev/null | grep -q 'set$'; then
	accns_log status "Factory reset button event."

	# Let flatfsd do the work for us, /etc/rc will complete it
	flatfsd -i
else
	accns_log status "ACPI power button event."

	# Power off the board
	reboot_managed "poweroff" "Shutdown due to ACPI power button."
fi

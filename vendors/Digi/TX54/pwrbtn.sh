#!/bin/sh

# Make power LED blinking as soon as possible
ledcmd -F POWER

accns_log status "ACPI power button event."

# Power off the board
reboot_managed "poweroff" "Shutdown due to ACPI power button."

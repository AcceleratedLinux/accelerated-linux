#!/bin/sh
# handle power button event

ledcmd -F POWER_BLUE

accns_log status "ACPI power button event."

reboot_managed "poweroff -d 2 -f" "Shutdown due to ACPI power button."


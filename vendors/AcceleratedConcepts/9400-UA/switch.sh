#!/bin/sh
#
# switch.sh  -- initialize networking hardware
#

#
# This initialization puts the switch into a basic switch mode.
# Ports 0, 1, 2 and 3 are the external facing ports. Port 4 is the
# internal network connected to the CPU.
#
# The MDIO lines that talk to the switch are connected to the MDIO
# interface of eth4. The switch registers are mapped to the normally
# invalid PHY address range 32->63 (that is 0x20 to 0x3f).
#
for port in 0 1 2 3 4
do
	# Put port into forwarding mode
	let "phy = $port + 48"
	swtest -i eth4 -p $phy -a 4 -w 0x007f

	#Set LED actions (LED0=GREEN=link,LED1=ORANGE=activity)
	swtest -i eth4 -p $phy -a 22 -w 0x8088

	# Power up port PHY
	let "cmd = 0x9400 | ( $port << 5 )"
	swtest -i eth4 -p 60 -a 25 -w 0x9100
	swtest -i eth4 -p 60 -a 24 -w $cmd
done

exit 0

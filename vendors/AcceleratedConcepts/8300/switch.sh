#
# switch.sh  -- initialize networking hardware
#

# Default the switch setup
bcm53118 -e /etc/eeprom.bin

# Set LED functions: orange=link, green=activity
bcm53118 -16 0x0012 0x000a

# Force link on IMP port
bcm53118 -8 0x000e 0x8b

# Change virtual switch interface to be eth0
ip link set eth0 name eth0raw
ip link set bcm0 name eth0
ip link set eth0raw up

# Enable managed mode and forwarding
bcm53118 -8 0x000b 0x03
# Enable IMP receive
bcm53118 -8 0x0008 0x1c
# Enable IMP tagging
bcm53118 -8 0x0203 0x01
# Enable IMP frame management, required for 802.1x
bcm53118 -8 0x0200 0x80

# Enable "forwarding state" on each port
bcm53118 -8 0x0000 0x00
bcm53118 -8 0x0001 0x00
bcm53118 -8 0x0002 0x00
bcm53118 -8 0x0003 0x00
bcm53118 -8 0x0004 0x00
bcm53118 -8 0x0005 0x00
bcm53118 -8 0x0006 0x00
bcm53118 -8 0x0007 0x00

# Set the LED functions on the realtek 8211CL of eth1
ip link set eth1 up
swtest -i eth1 -p 3 -a 31 -w 2
swtest -i eth1 -p 3 -a 26 -w 0x0078
swtest -i eth1 -p 3 -a 31 -w 0
ip link set eth1 down

exit 0

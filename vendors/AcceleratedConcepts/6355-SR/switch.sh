#
# switch.sh  -- initialize networking hardware
#

#
# The switch device is setup using the Marvell Distributed Switch
# Architecture. This effectively gives us 5 individual ethernet ports.
# The internal nodes (the trunk ports) we name eth0cpu and eth1cpu,
# so we don't confuse them with conventional eth0 and eth1 ports.
#
ip link set eth0 name eth0cpu
ip link set eth1 name eth1cpu
ifconfig eth0cpu up
ifconfig eth1cpu up

#
# Set up LED actions
#
swtest -i lan1 -p 48 -a 22 -w 0x8088
swtest -i lan2 -p 49 -a 22 -w 0x8088
swtest -i lan3 -p 50 -a 22 -w 0x8088
swtest -i lan4 -p 51 -a 22 -w 0x8088
swtest -i wan -p 52 -a 22 -w 0x8088

exit 0

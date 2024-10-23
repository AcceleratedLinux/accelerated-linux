#
# armada380 -- support for the Marvell Armada/380 CPU family
#
# (C) Copyright 2013,2016,  greg.ungerer@accelerated.com
#

jtag newtap armada380 cpu -irlen 4 -ircapture 0x1 -irmask 0xf -expected-id 0x4ba00477
dap create armada380.dap -chain-position armada380.cpu
target create armada380 cortex_a -dap armada380.dap

proc armada380_dbginit {target} {
     cortex_a dbginit
}

armada380 configure -event reset-assert-post "armada380_dbginit armada380"

# We need to init now, so we can run the apsel command.
# init
armada380.dap apsel 1


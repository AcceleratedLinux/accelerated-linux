#
# armada380 -- support for the Marvell Armada/380 CPU family
#
# (C) Copyright 2013,2016,  greg.ungerer@accelerated.com
#

jtag newtap armada380 dap -irlen 4 -ircapture 0x1 -irmask 0xf -expected-id 0x4ba00477
target create armada380.cpu cortex_a8 -chain-position armada380.dap


proc armada380_dbginit {target} {
     cortex_a dbginit
}

armada380.cpu configure -event reset-assert-post "armada380_dbginit armada380.cpu"

# We need to init now, so we can run the apsel command.
init
dap apsel 1


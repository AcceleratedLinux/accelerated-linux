#
# armada370 -- support for the Marvell Armada/370 CPU family
#
# (C) Copyright 2013,  greg.ungerer@accelecon.com
#

jtag newtap armada370 dap -irlen 4 -ircapture 0x1 -irmask 0xf -expected-id 0x4ba00477
target create armada370.cpu cortex_a8 -chain-position armada370.dap


proc armada370_dbginit {target} {
     cortex_a dbginit
}

armada370.cpu configure -event reset-assert-post "armada370_dbginit armada370.cpu"

# We need to init now, so we can run the apsel command.
init
dap apsel 1


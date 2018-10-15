#
# imx50.tcl -- configuration for the Freescale IMX50 SoC
#
# (C) Copyright 2013,  Greg Ungerer <greg.ungerer@accelecon.com>
#

jtag newtap imx50 dap -irlen 4 -ircapture 0x1 -irmask 0xf -expected-id 0x1ba00477
jtag newtap imx50 sdma -irlen 4 -ircapture 0x0 -irmask 0xf
jtag newtap imx50 sjc -irlen 5 -ircapture 0x1 -irmask 0x1f -expected-id 0x02d0101d

target create imx50.cpu cortex_a8 -chain-position imx50.dap


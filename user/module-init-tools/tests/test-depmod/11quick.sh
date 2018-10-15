#! /bin/sh
# Check that -A works correctly.

for ENDIAN in -le -be; do
for BITNESS in 32 64; do

# Copy modules across so we can play with them.
rm -rf tests/tmp/*
cp -a tests/data tests/tmp/

# Inputs
MODTEST_OVERRIDE1=/lib/modules/$MODTEST_UNAME
MODTEST_OVERRIDE_WITH1=tests/tmp/data/$BITNESS$ENDIAN/normal
export MODTEST_OVERRIDE1 MODTEST_OVERRIDE_WITH1

MODTEST_OVERRIDE2=/lib/modules/$MODTEST_UNAME/export_dep-$BITNESS.ko
MODTEST_OVERRIDE_WITH2=tests/tmp/data/$BITNESS$ENDIAN/normal/export_dep-$BITNESS.ko
export MODTEST_OVERRIDE2 MODTEST_OVERRIDE_WITH2

MODTEST_OVERRIDE3=/lib/modules/$MODTEST_UNAME/noexport_dep-$BITNESS.ko
MODTEST_OVERRIDE_WITH3=tests/tmp/data/$BITNESS$ENDIAN/normal/noexport_dep-$BITNESS.ko
export MODTEST_OVERRIDE3 MODTEST_OVERRIDE_WITH3

MODTEST_OVERRIDE4=/lib/modules/$MODTEST_UNAME/noexport_nodep-$BITNESS.ko
MODTEST_OVERRIDE_WITH4=tests/tmp/data/$BITNESS$ENDIAN/normal/noexport_nodep-$BITNESS.ko
export MODTEST_OVERRIDE4 MODTEST_OVERRIDE_WITH4

MODTEST_OVERRIDE5=/lib/modules/$MODTEST_UNAME/export_nodep-$BITNESS.ko
MODTEST_OVERRIDE_WITH5=tests/tmp/data/$BITNESS$ENDIAN/normal/export_nodep-$BITNESS.ko
export MODTEST_OVERRIDE5 MODTEST_OVERRIDE_WITH5

MODTEST_OVERRIDE6=/lib/modules/$MODTEST_UNAME/noexport_doubledep-$BITNESS.ko
MODTEST_OVERRIDE_WITH6=tests/tmp/data/$BITNESS$ENDIAN/normal/noexport_doubledep-$BITNESS.ko
export MODTEST_OVERRIDE6 MODTEST_OVERRIDE_WITH6

# Outputs
MODTEST_OVERRIDE7=/lib/modules/$MODTEST_UNAME/modules.dep
MODTEST_OVERRIDE_WITH7=tests/tmp/modules.dep
export MODTEST_OVERRIDE7 MODTEST_OVERRIDE_WITH7

MODTEST_OVERRIDE8=/lib/modules/$MODTEST_UNAME/modules.pcimap
MODTEST_OVERRIDE_WITH8=tests/tmp/modules.pcimap
export MODTEST_OVERRIDE8 MODTEST_OVERRIDE_WITH8

MODTEST_OVERRIDE9=/lib/modules/$MODTEST_UNAME/modules.usbmap
MODTEST_OVERRIDE_WITH9=tests/tmp/modules.usbmap
export MODTEST_OVERRIDE9 MODTEST_OVERRIDE_WITH9

MODTEST_OVERRIDE10=/lib/modules/$MODTEST_UNAME/modules.ccwmap
MODTEST_OVERRIDE_WITH10=tests/tmp/modules.ccwmap
export MODTEST_OVERRIDE10 MODTEST_OVERRIDE_WITH10

MODTEST_OVERRIDE11=/lib/modules/$MODTEST_UNAME/modules.alias
MODTEST_OVERRIDE_WITH11=tests/tmp/modules.alias
export MODTEST_OVERRIDE11 MODTEST_OVERRIDE_WITH11

MODTEST_OVERRIDE12=/lib/modules/$MODTEST_UNAME/modules.symbols
MODTEST_OVERRIDE_WITH12=tests/tmp/modules.symbols
export MODTEST_OVERRIDE12 MODTEST_OVERRIDE_WITH12

MODTEST_OVERRIDE13=/lib/modules/$MODTEST_UNAME/modules.ieee1394map
MODTEST_OVERRIDE_WITH13=tests/tmp/modules.ieee1394map
export MODTEST_OVERRIDE13 MODTEST_OVERRIDE_WITH13

MODTEST_OVERRIDE14=/lib/modules/$MODTEST_UNAME/modules.dep.temp
MODTEST_OVERRIDE_WITH14=tests/tmp/modules.dep.temp
export MODTEST_OVERRIDE14 MODTEST_OVERRIDE_WITH14

MODTEST_OVERRIDE15=/lib/modules/$MODTEST_UNAME/modules.pcimap.temp
MODTEST_OVERRIDE_WITH15=tests/tmp/modules.pcimap.temp
export MODTEST_OVERRIDE15 MODTEST_OVERRIDE_WITH15

MODTEST_OVERRIDE16=/lib/modules/$MODTEST_UNAME/modules.usbmap.temp
MODTEST_OVERRIDE_WITH16=tests/tmp/modules.usbmap.temp
export MODTEST_OVERRIDE16 MODTEST_OVERRIDE_WITH16

MODTEST_OVERRIDE17=/lib/modules/$MODTEST_UNAME/modules.ccwmap.temp
MODTEST_OVERRIDE_WITH17=tests/tmp/modules.ccwmap.temp
export MODTEST_OVERRIDE17 MODTEST_OVERRIDE_WITH17

MODTEST_OVERRIDE18=/lib/modules/$MODTEST_UNAME/modules.alias.temp
MODTEST_OVERRIDE_WITH18=tests/tmp/modules.alias.temp
export MODTEST_OVERRIDE18 MODTEST_OVERRIDE_WITH18

MODTEST_OVERRIDE19=/lib/modules/$MODTEST_UNAME/modules.symbols.temp
MODTEST_OVERRIDE_WITH19=tests/tmp/modules.symbols.temp
export MODTEST_OVERRIDE19 MODTEST_OVERRIDE_WITH19

MODTEST_OVERRIDE20=/lib/modules/$MODTEST_UNAME/modules.ieee1394map.temp
MODTEST_OVERRIDE_WITH20=tests/tmp/modules.ieee1394map.temp
export MODTEST_OVERRIDE20 MODTEST_OVERRIDE_WITH20

MODTEST_OVERRIDE21=/lib/modules/$MODTEST_UNAME/modules.isapnpmap.temp
MODTEST_OVERRIDE_WITH21=tests/tmp/modules.isapnpmap.temp
export MODTEST_OVERRIDE21 MODTEST_OVERRIDE_WITH21

MODTEST_OVERRIDE22=/lib/modules/$MODTEST_UNAME/modules.isapnpmap
MODTEST_OVERRIDE_WITH22=tests/tmp/modules.isapnpmap
export MODTEST_OVERRIDE22 MODTEST_OVERRIDE_WITH22

MODTEST_OVERRIDE23=/lib/modules/$MODTEST_UNAME/modules.inputmap.temp
MODTEST_OVERRIDE_WITH23=tests/tmp/modules.inputmap.temp
export MODTEST_OVERRIDE23 MODTEST_OVERRIDE_WITH23

MODTEST_OVERRIDE24=/lib/modules/$MODTEST_UNAME/modules.inputmap
MODTEST_OVERRIDE_WITH24=tests/tmp/modules.inputmap
export MODTEST_OVERRIDE24 MODTEST_OVERRIDE_WITH24

MODTEST_OVERRIDE25=/lib/modules/$MODTEST_UNAME/modules.seriomap.temp
MODTEST_OVERRIDE_WITH25=tests/tmp/modules.seriomap.temp
export MODTEST_OVERRIDE25 MODTEST_OVERRIDE_WITH25

MODTEST_OVERRIDE26=/lib/modules/$MODTEST_UNAME/modules.seriomap
MODTEST_OVERRIDE_WITH26=tests/tmp/modules.seriomap
export MODTEST_OVERRIDE26 MODTEST_OVERRIDE_WITH26

MODTEST_OVERRIDE28=/lib/modules/$MODTEST_UNAME/modules.ofmap.temp
MODTEST_OVERRIDE_WITH28=tests/tmp/modules.ofmap.temp
export MODTEST_OVERRIDE28 MODTEST_OVERRIDE_WITH28

MODTEST_OVERRIDE29=/lib/modules/$MODTEST_UNAME/modules.ofmap
MODTEST_OVERRIDE_WITH29=tests/tmp/modules.ofmap
export MODTEST_OVERRIDE29 MODTEST_OVERRIDE_WITH29

# Temporary file.
MODTEST_OVERRIDE27=/lib/modules/$MODTEST_UNAME/README
MODTEST_OVERRIDE_WITH27=tests/tmp/data/$BITNESS$ENDIAN/normal/README
export MODTEST_OVERRIDE27 MODTEST_OVERRIDE_WITH27

# First time, there is no modules.dep, so it will be generated.
# Expect no output.
[ "`depmod -A 2>&1`" = "" ]

# Check modules.dep results: expect 5 lines
[ `grep -vc '^#' < tests/tmp/modules.dep` = 5 ]

[ "`grep -w export_dep-$BITNESS.ko: tests/tmp/modules.dep`" = "/lib/modules/$MODTEST_UNAME/export_dep-$BITNESS.ko: /lib/modules/$MODTEST_UNAME/export_nodep-$BITNESS.ko" ]
[ "`grep -w noexport_dep-$BITNESS.ko: tests/tmp/modules.dep`" = "/lib/modules/$MODTEST_UNAME/noexport_dep-$BITNESS.ko: /lib/modules/$MODTEST_UNAME/export_nodep-$BITNESS.ko" ]
[ "`grep -w export_nodep-$BITNESS.ko: tests/tmp/modules.dep`" = "/lib/modules/$MODTEST_UNAME/export_nodep-$BITNESS.ko:" ]
[ "`grep -w noexport_nodep-$BITNESS.ko: tests/tmp/modules.dep`" = "/lib/modules/$MODTEST_UNAME/noexport_nodep-$BITNESS.ko:" ]
[ "`grep -w noexport_doubledep-$BITNESS.ko: tests/tmp/modules.dep`" = "/lib/modules/$MODTEST_UNAME/noexport_doubledep-$BITNESS.ko: /lib/modules/$MODTEST_UNAME/export_dep-$BITNESS.ko /lib/modules/$MODTEST_UNAME/export_nodep-$BITNESS.ko" ]

# Check modules.symbols results: expect 3 lines
[ `grep -vc '^#' < tests/tmp/modules.symbols` = 3 ]

[ "`grep -w symbol:exported1 tests/tmp/modules.symbols`" = "alias symbol:exported1 export_nodep_$BITNESS" ]
[ "`grep -w symbol:exported2 tests/tmp/modules.symbols`" = "alias symbol:exported2 export_nodep_$BITNESS" ]
[ "`grep -w symbol:exported3 tests/tmp/modules.symbols`" = "alias symbol:exported3 export_dep_$BITNESS" ]

cp tests/tmp/modules.dep tests/tmp/modules.dep.old
cp tests/tmp/modules.symbols tests/tmp/modules.symbols.old

# Now, second run should do nothing.
cp /dev/null tests/tmp/modules.dep
cp /dev/null tests/tmp/modules.symbols
[ "`depmod -A 2>&1`" = "" ]
diff -u /dev/null tests/tmp/modules.dep
diff -u /dev/null tests/tmp/modules.symbols

# Touch a directory, nothing.
sleep 1
touch tests/tmp/data/$BITNESS$ENDIAN/normal

[ "`depmod -A 2>&1`" = "" ]
diff -u /dev/null tests/tmp/modules.dep
diff -u /dev/null tests/tmp/modules.symbols

# Touch a non-module, nothing.
echo "Test" > tests/tmp/data/$BITNESS$ENDIAN/normal/README
[ "`depmod -A 2>&1`" = "" ]
diff -u /dev/null tests/tmp/modules.dep
diff -u /dev/null tests/tmp/modules.symbols

# Touch a module, wham.
touch tests/tmp/data/$BITNESS$ENDIAN/normal/noexport_doubledep-$BITNESS.ko
[ "`depmod -A 2>&1`" = "" ]
diff -u tests/tmp/modules.dep.old tests/tmp/modules.dep
diff -u tests/tmp/modules.symbols.old tests/tmp/modules.symbols

# Deleting a module does nothing.
cp /dev/null tests/tmp/modules.dep
cp /dev/null tests/tmp/modules.symbols
rm tests/tmp/data/$BITNESS$ENDIAN/normal/noexport_doubledep-$BITNESS.ko
[ "`depmod -A 2>&1`" = "" ]
diff -u /dev/null tests/tmp/modules.dep
diff -u /dev/null tests/tmp/modules.symbols

done
done

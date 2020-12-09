
= Creating bootarg files

== Description
bootarg files are an image of the initial uboot configuration partition. The
partition is large however only a small set of options are required for first
boot.

== Device tailoring
First zero your bootargs area with "setfset -z".

Now set device-specific parameters. For instance an ACM7008-2 as below.
NOTE: All ACM700x bootarg files MUST set "do_factory_setup=1"!

setfset -u model=ACM7008-2
setfset -u ports=8
setfset -u pinout=cisco
setfset -u console=shared
setfset -u power=external/dc
setfset -u ethernet=dual
setfset -u factory_opts=sensors
setfset -u sensors=0/2
setfset -u do_factory_setup=1

Now take a copy of the bootarg partition for use as our bootarg file. Typically
256 bytes is more than enough to store all of the values required for initial
boot. If you add more, ensure that all values are encapsulated by piping to 'hd'
and checking for a zeroed area at the end of the image.

dd if=/dev/flash/bootarg bs=256 count=1 of=/tmp/mydevice.bootarg

== File naming
Typically files are stored inside our source tree at:
vendors/OpenGear/<model group>/<lower case model name>.bootarg

for instance the ACM7008-2 should be stored at:
vendors/OpenGear/ACM700x/acm7008-2.bootarg


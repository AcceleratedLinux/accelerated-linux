#!/bin/bash
#
# This script is for help with maintaining patches/diet.patch.
#
# This script examines the extension sources under build/iptables*/extensions/
# and the kernel Kconfig symbols under ../../linux and emits the obvious
# aliases to use in the patch. When something is declared 'not found', you'll
# need to figure out yourself if it really is unsupported, or what symbol
# enables compatibility. (In the worst case, you can use an alias value of
# CONFIG_NETFILTER which should always be set.)

export LC_ALL=C

linuxdir=../../linux
makefiles=($(find $linuxdir/net -name Makefile))
Kconfigs=($(find $linuxdir/net -name Kconfig\*))

ksym_in_a_Kconfig ()
{
	grep -q -w "config $1" ${Kconfigs[*]} /dev/null
}

# For a kmod name, emit the Kconfig symbol name that the diet.patch's
# macros test by default, eg ipt_SNAT -> IP_NF_TARGET_SNAT
normal_ksym_for_kmod ()
{
	local kmod=$1
	local name=${kmod#*_}; name=${name^^}
	case $kmod in
	arpt_*[a-z]*)	echo IP_NF_ARP_$name;;
	ebt_*)		echo BRIDGE_EBT_$name;;
	ip6t_*[a-z]*)	echo IP6_NF_MATCH_$name;;
	ip6t_*)		echo IP6_NF_TARGET_$name;;
	ipt_*[a-z]*)	echo IP_NF_MATCH_$name;;
	ipt_*)		echo IP_NF_TARGET_$name;;
	xt_*[a-z]*)	echo NETFILTER_XT_MATCH_$name;;
	xt_*)		echo NETFILTER_XT_TARGET_$name;;
	*)		echo "Unknown kmod form $kmod" >&2; exit 1;;
	esac
}

# Search the linux Makefiles for kmod's "obj-$(CONFIG_<ksym>) += <kmod>.o"
# rule, and print the <ksym> found.
ksym_that_makefile_uses_for_kmod () {
	local kmod=$1
	sed -ne 's!^obj-\$(CONFIG_\([^)]*\)).*+=.* '$kmod'\.o!\1!p' \
			${makefiles[*]} /dev/null
}

# Search the linux sources for a module that has a MODULE_ALIAS(<kmod>)
# macro for the selected kmod. Prints the found module's kmod.
kmod_with_module_alias_for_kmod ()
{
	local kmod=$1
	rgrep -l "^MODULE_ALIAS(\"$kmod\")" $linuxdir/net |
		sed -e 's,.*/,,;s,\.c$,,;q'
}

# Search the linux tree to figure out which CONFIG_<ksym> enables <kmod>.o
ksym_for_kmod ()
{
	local kmod=$1

	local msel=$(ksym_that_makefile_uses_for_kmod $kmod)
	if [ "$msel" ]; then
		#echo "# Found a Makefile using CONFIG_$msel to enable $kmod" >&2
		echo $msel
		return
	fi

	local alias=$(kmod_with_module_alias_for_kmod $kmod)
	if [ "$alias" ]; then
		#echo "# Found use of MODULE_ALIAS($kmod) by module $alias" >&2
		ksym_for_kmod $alias
		return
	fi

	false
}

# Given a netfilter extension, named after a kmod,
# emit a makefile alias assignment that will help it work.
verify_extension ()
{
	local src=$1
	local kmod=${src##*/lib}; kmod=${kmod%.c}
	local normal_ksym=$(normal_ksym_for_kmod $kmod)
	local actual_ksym=$(ksym_for_kmod $kmod)

	if [ ! $actual_ksym ]; then
		echo "# alias_CONFIG_$normal_ksym =  not found"
	elif [ $normal_ksym = $actual_ksym ]; then
		: #echo "# $kmod found"
	else
		echo "alias_CONFIG_$normal_ksym = CONFIG_$actual_ksym"
	fi
}

for file in build/iptables-*/extensions/lib*.c; do
	verify_extension $file
done

#!/bin/sh

. /sbin/config.sh
. /sbin/global.sh

l2tp_pt=`flashconfig.sh get l2tpPassThru`
ipsec_pt=`flashconfig.sh get ipsecPassThru`
pptp_pt=`flashconfig.sh get pptpPassThru`


# note: they must be removed in order
if [ "$CONFIG_NF_CONNTRACK_SUPPORT" = "y" ]; then
	rmmod nf_nat_pptp
	rmmod nf_conntrack_pptp
	rmmod nf_nat_proto_gre
	rmmod nf_conntrack_proto_gre
else
	rmmod ip_nat_pptp
	rmmod ip_conntrack_pptp
fi

if [ "$pptp_pt" = "1" -o "$l2tp_pt" = "1" -o "$ipsec_pt" = "1" ]; then
if [ "$CONFIG_NF_CONNTRACK_SUPPORT" = "y" ]; then
	modprobe -q nf_conntrack_proto_gre
	modprobe -q nf_nat_proto_gre

	if [ "$pptp_pt" = "1" ]; then
		modprobe -q nf_conntrack_pptp
		modprobe -q nf_nat_pptp
	fi
else
	modprobe -q ip_conntrack_pptp
	modprobe -q ip_nat_pptp
fi 
fi 

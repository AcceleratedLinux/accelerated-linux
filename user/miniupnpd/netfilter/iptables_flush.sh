#! /bin/sh
# $Id: iptables_flush.sh,v 1.1 2008-09-15 12:28:53 winfred Exp $
IPTABLES=iptables

#flush all rules owned by miniupnpd
$IPTABLES -t nat -F MINIUPNPD
$IPTABLES -t filter -F MINIUPNPD


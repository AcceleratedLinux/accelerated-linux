#!/bin/sh
#
# $Id: config-igmpproxy.sh,v 1.5.6.1 2009-04-01 02:18:21 steven Exp $
#
# usage: config-igmpproxy.sh <wan_if_name> <lan_if_name>
#

. /sbin/global.sh

igmpproxy.sh $wan_if $lan_if ppp0
killall -q igmpproxy
igmpproxy


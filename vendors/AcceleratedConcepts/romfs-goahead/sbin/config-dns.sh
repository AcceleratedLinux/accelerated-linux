#!/bin/sh

# $Id: config-dns.sh,v 1.2 2007-10-15 08:26:00 winfred Exp $
# usage: config-dns.sh [<dns1>] [<dns2>]

fname="/etc/resolv.conf"
fbak="/tmp/resolv_conf.bak"

# in case no previous file
touch $fname

# backup file without nameserver part
sed -e '/nameserver/d' $fname > $fbak

# set primary and seconday DNS
if [ "$1" != "" ]; then
  echo "nameserver $1" > $fname
else # empty dns
  rm -f $fname
fi
if [ "$2" != "" ]; then
  echo "nameserver $2" >> $fname
fi

cat $fbak >> $fname
rm -f $fbak


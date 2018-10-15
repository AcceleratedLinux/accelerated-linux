#!/bin/sh
#
# $Id: config-udhcpd.sh,v 1.6.4.4 2009-04-22 02:46:43 michael Exp $
#
# usage: see function usage()
#
. /sbin/config.sh
. /sbin/global.sh

fname="/etc/config/udhcpd.conf"
fbak="/tmp/udhcpd.conf_bak"
pidfile="/var/run/udhcpd.pid"
leases="/etc/config/udhcpd.leases"

usage () {
  echo "usage: config-udhcpd.sh [option]..."
  echo "options:"
  echo "  -h              : print this help"
  echo "  -s ipaddr       : set ipaddr as start of the IP lease block"
  echo "  -e ipaddr       : set ipaddr as end of the IP lease block"
  echo "  -i ifc          : set ifc as the interface that udhcpd will use"
  echo "  -d dns1 [dns2]  : set dns1 and dns2 as DNS"
  echo "  -m mask         : set mask as subnet netmask"
  echo "  -g gateway      : set gateway as router's IP address"
  echo "  -t time         : set time seconds as the IP life time"
  echo "  -r [sleep_time] : run dhcp server"
  echo "  -k              : kill the running dhcp server"
  echo "  -S [mac ipaddr] : statically assign IP to given MAC address"
  exit
}

config () {
  case "$1" in
    "-s")
      sed -e '/start/d' $fname > $fbak
      echo "start $2" >> $fbak ;;
    "-e")
      sed -e '/end/d' $fname > $fbak
      echo "end $2" >> $fbak ;;
    "-i")
      sed -e '/interface/d' $fname > $fbak
      echo "interface $2" >> $fbak ;;
    "-d")
      sed -e '/option *dns/d' $fname > $fbak
      echo "option dns $2 $3" >> $fbak ;;
    "-m")
      sed -e '/option *subnet/d' $fname > $fbak
      echo "option subnet $2" >> $fbak ;;
    "-g")
      sed -e '/option *router/d' $fname > $fbak
      echo "option router $2" >> $fbak ;;
    "-t")
      sed -e '/option *lease/d' $fname > $fbak
      echo "option lease $2" >> $fbak ;;
    "-S")
      if [ "$2" = "" ]; then
        sed -e '/static_lease/d' $fname > $fbak
      elif [ "$3" = "" ]; then
	echo "insufficient arguments.."
	usage
      else
        echo "static_lease $2 $3" >> $fname
	return
      fi
      ;;
    *) return;;
  esac
  cat $fbak > $fname
  rm -f $fbak
  return
}

#  arg1:  phy address.
link_down()
{
	# get original register value
	get_mii=`mii_mgr -g -p $1 -r 0`
	orig=`echo $get_mii | sed 's/^.....................//'`

	# stupid hex value calculation.
	pre=`echo $orig | sed 's/...$//'`
	post=`echo $orig | sed 's/^..//'` 
	num_hex=`echo $orig | sed 's/^.//' | sed 's/..$//'`
	case $num_hex in
		"0")	rep="8"	;;
		"1")	rep="9"	;;
		"2")	rep="a"	;;
		"3")	rep="b"	;;
		"4")	rep="c"	;;
		"5")	rep="d"	;;
		"6")	rep="e"	;;
		"7")	rep="f"	;;
		# The power is already down
		*)		echo "Warning in PHY reset script";return;;
	esac
	new=$pre$rep$post
	# power down
	mii_mgr -s -p $1 -r 0 -v $new
}

link_up()
{
	# get original register value
	get_mii=`mii_mgr -g -p $1 -r 0`
	orig=`echo $get_mii | sed 's/^.....................//'`

	# stupid hex value calculation.
	pre=`echo $orig | sed 's/...$//'`
	post=`echo $orig | sed 's/^..//'` 
	num_hex=`echo $orig | sed 's/^.//' | sed 's/..$//'`
	case $num_hex in
		"8")	rep="0"	;;
		"9")	rep="1"	;;
		"a")	rep="2"	;;
		"b")	rep="3"	;;
		"c")	rep="4"	;;
		"d")	rep="5"	;;
		"e")	rep="6"	;;
		"f")	rep="7"	;;
		# The power is already up
		*)		echo "Warning in PHY reset script";return;;
	esac
	new=$pre$rep$post
	# power up
	mii_mgr -s -p $1 -r 0 -v $new
}

reset_all_phys()
{
	sleep_time=$1

	if [ "$CONFIG_RAETH_ROUTER" != "y" -a "$CONFIG_RT_3052_ESW" != "y" ]; then
		return
	fi

	opmode=`flashconfig.sh get OperationMode`

	#skip WAN port
	if [ "$opmode" != "1" ]; then
		link_down 0
	fi
	link_down 1
#	link_down 2
#	link_down 3
#	link_down 4

	#force Windows clients to renew IP and update DNS server
	sleep $sleep_time

	if [ "$opmode" != "1" ]; then
		link_up 0
	fi
	link_up 1
#	link_up 2
#	link_up 3
#	link_up 4
}



# argv 1 is empty
if [ "$1" = "" ]; then
  usage
fi

# argv 2 is empty
if [ "$2" = "" ]; then
  if [ "$1" != "-r" ]; then
    if [ "$1" != "-k" ]; then
      usage
	fi
  fi
fi

touch $fname

case "$1" in
  "-h") usage;;
  "-s") config "$1" "$2";;
  "-e") config "$1" "$2";;
  "-i") config "$1" "$2";;
  "-d") config "$1" "$2" "$3";;
  "-m") config "$1" "$2";;
  "-g") config "$1" "$2";;
  "-t") config "$1" "$2";;
  "-S") config "$1" "$2" "$3";;
  "-r")
    if [ -e ${pidfile} ]; then
      kill `cat $pidfile`
    else
      killall udhcpd
    fi
    rm -f $pidfile
    touch $leases
    echo "lease_file $leases" >> $fname
    udhcpd $fname
    reset_all_phys $2;;
  "-k")
    if [ -e ${pidfile} ]; then
      kill `cat $pidfile`
    else
      killall udhcpd
    fi
    rm -f $pidfile ;;
  *) usage;;
esac



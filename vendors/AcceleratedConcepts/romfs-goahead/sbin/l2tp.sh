#!/bin/sh
CONF_DIR=/etc/l2tp
CONF_FILE=/etc/l2tp/l2tp.conf
L2TP_FILE=/etc/options.l2tp

if [ ! -n "$3" ]; then
  echo "insufficient arguments!"
  echo "Usage: $0 <user> <password> <serv_ip>"
  exit 0
fi

L2TP_USER_NAME="$1"
L2TP_PASSWORD="$2"
L2TP_SERV_IP="$3"
L2TP_OPMODE="$4"
L2TP_OPTIME="$5"

if [ ! -d $CONF_DIR ] ; then mkdir -p $CONF_DIR; fi

echo "global
load-handler \"sync-pppd.so\"
load-handler \"cmd.so\"
listen-port 1701
section sync-pppd
lac-pppd-opts \"file $L2TP_FILE\"
section peer
peer $L2TP_SERV_IP
port 1701
lac-handler sync-pppd
persist yes
maxfail 32767
holdoff 30
hide-avps no
section cmd" > $CONF_FILE

echo "noauth refuse-eap
user \"$L2TP_USER_NAME\"
password \"$L2TP_PASSWORD\"
nomppe nomppc
maxfail 0
usepeerdns" > $L2TP_FILE
if [ $L2TP_OPMODE == "KeepAlive" ]; then
	echo "persist" >> $L2TP_FILE
	echo "holdoff $L2TP_OPTIME" >> $L2TP_FILE
elif [ $L2TP_OPMODE == "OnDemand" ]; then
	L2TP_OPTIME=`expr $L2TP_OPTIME \* 60`
	echo "demand" >> $L2TP_FILE
	echo "idle $L2TP_OPTIME" >> $L2TP_FILE
fi
echo "defaultroute 
ipcp-accept-remote ipcp-accept-local noipdefault
ktune
default-asyncmap nopcomp noaccomp
novj nobsdcomp nodeflate
lcp-echo-interval 10
lcp-echo-failure 6
unit 0 " >> $L2TP_FILE


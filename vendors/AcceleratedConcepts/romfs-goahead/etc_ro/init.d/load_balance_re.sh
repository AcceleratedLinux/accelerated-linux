#!/bin/sh
 if [ "$1" = "start" ]; then
	/etc/init.d/load_balance.sh stop
	sleep 1
	/etc/init.d/load_balance.sh start
 fi

 if [ "$1" = "stop" ]; then
	/etc/init.d/load_balance.sh stop
 fi 

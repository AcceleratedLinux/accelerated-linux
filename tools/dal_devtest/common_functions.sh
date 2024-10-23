#!/bin/bash

# globals
# BEGIN_TIME: seconds, set zero_stats(), used for test file output.
# LAST_TIME: seconds, set zero_stats(), used for test file output.
# TEST_COUNT: no. of tests executed since zero_stats(), used for test file output and summary results.
# PASSED_COUNT: no. of tests passing since zero_stats(), used for test file output and summary results.
# FAILED_COUNT: no. of tests failing since zero_stats(), used for test file output and summary results.
# DURATION: seconds, set zero_stats() & output_results(), used for test file output. time a test took.
# TEST_FILE: test filename, used used for test file output, set below.
# DIR_NAME: directory test file is in. eg. example, set below, used for test file output.
# VERSION: DAL version, set below, used for test file output.
# FATAL: set fatal(), when test cannot continue.

# SCRIPT_DIR: directory of current running test file, set by test file.
# OUTPUT_DIR: directory to put test results and log files.

version()
{
	# NOTE: This string is replaced with build version on tarball creation.
	echo "@@VERSION@@" | sed 's/-dirty//g'
}

test_cleanup()
{
	# this function is called when a test file finishes execution to cleanup.
	[ -n "$CONFIG_ONLY" ] && exit

	if type test_restore >/dev/null 2>&1;then
		# custom cleanup by test file
		test_restore
	fi
	[ -n "$RESTORE_ONLY" ] && exit

	stop_log $DIR_NAME
	output_result_done
}

run_tests()
{
	# this function is called to process any command line arguments to test file

	if [ -n "$1" ]; then
		if [ "$2" = "config" ]; then
			CONFIG_ONLY=1
		elif [ "$2" = "restore" ]; then
			RESTORE_ONLY=1
			exit
		fi
		$1
	else
		for test_func in $TEST_FUNCS;do
			$test_func
		done
	fi
}

get_device_data()
{
	# read in test data for device property
	# args: $1 = property
	# eg. "get_device_data primary_lan_device" prints value to stdout

	local model=$(cat /etc/version|awk '{print $1}'|awk -F/ '{print $2}')
	if [ -e ${SCRIPT_DIR}/../devicedata.csv ]; then
		grep "^${model}," ${SCRIPT_DIR}/../devicedata.csv|\
			grep "$1"|awk -F, '{print $3}'
	fi
}

zero_stats()
{
	# zeros env variables used in generation of junit.xml and satest.json test files

	TEST_COUNT=0
	PASSED_COUNT=0
	FAILED_COUNT=0
	DURATION=0
	BEGIN_TIME=$(date +%s)
	LAST_TIME=$BEGIN_TIME
	VERSION=$(cat /etc/version |awk '{print $3}')
	OUTPUT_DIR=/tmp/dal_devtests
	FATAL=0

	rm -f /tmp/satest.json
	rm -f /tmp/junit.xml
}

save_cfg()
{
	# saves current configuration, run before test.

	if [ ! -e /opt/saved_config.json ]; then
		cp /etc/config/accns.json /opt/saved_config.json
	fi
}

restore_cfg()
{
	# restores previously saved configuration, run after test.
	if [ -e /opt/saved_config.json ]; then
		eval $(config stop)
		eval $(config start)
		config try-create /opt/saved_config.json
		local created=$?
		if [ "$created" == 0 ]; then
			config commit
		else
			logger -t dal_devtests "restore_cfg() failed"
		fi

		eval $(config stop)
		rm -f /opt/saved_config.json
	fi
}

override_file()
{
	# bind mount over file with another.
	# args: $1 = on/off
	# args: $2 = system file to override
	# args: $3 = test file to execute instead of.

	if [ "$1" = "on" ] && [ ! -e "$2" ] || [ ! -e "$3" ]; then
		echo "FATAL: $1 or $2 missing!?"
		return 1
	fi

	local err=1
	local retry=10

	while [ $err -ne 0 ] && [ $retry -gt 0 ]; do
		file=$(basename $2)
		killall -9 $file >/dev/null 2>&1
		sleep 1
		if [ "$1" = "on" ]; then
			if [ $(mount 2>/dev/null|grep -c "$2") -eq 0 ]; then
				mount --bind $3 $2 >/dev/null 2>&1
			else
				true
			fi
		else
			umount -f $2 >/dev/null 2>&1
			if [ $(mount 2>/dev/null|grep -c "$2") -eq 0 ]; then
				true
			else
				false
			fi
		fi
		err=$?
		retry=$(($retry - 1))
	done

	if [ "$1" = "on" ]; then
		if [ $(mount 2>/dev/null|grep -c "$2") -ne 1 ]; then
			return 1
		else
			return 0
		fi
	else
		if [ $(mount 2>/dev/null|grep -c "$2") -ne 0 ]; then
			return 1
		else
			return 0
		fi
	fi
}

log()
{
	# write string to syslog using topic $DIR_NAME so it can be captured to /tmp/$DIR_NAME.log
	# args: $1 = string
	logger -t "$DIR_NAME" "$1"
}

passed()
{
	# args: $1 = test description, appends test results to satest.json, junit.xml

	local desc="${1:0:80}"
	printf "%-80s " "$desc"
	if [ "$TERM" = "xterm-256color" ]; then
		echo -e "\\033[1;32m[OK]\\033[0;39m"
	else
		echo -e "[OK]"
	fi
	output_result "$desc" "" "" "passed"
	PASSED_COUNT=$(($PASSED_COUNT + 1))
}

failed()
{
	# args: $1 = test description, appends test results to satest.json, junit.xml
	# args: $2 = reason string, eg. "($val!=$expected)"

	local desc="${1:0:80}"
	printf "%-80s " "$desc"
	if [ "$TERM" = "xterm-256color" ]; then
		echo -e "\\033[1;31m[FAILED]\\033[0;39m ($2)"
	else
		echo -e "[FAILED] ($2)"
	fi
	output_result "$desc" "$2" "" "failed"
	FAILED_COUNT=$(($FAILED_COUNT + 1))
}

test_eval()
{
	# evaluate $2 expression and report pass/fail as required.
	# args: $1 = test description
	# args: $2 = expression to evaluate
	# args: $3 = reason eg. "x!=1"

	local desc="$1"
	local expression=$(( ${2} ))
	local reason="$3"

	[ $expression -eq 1 ] && passed "${desc}" || failed "${desc}" "${reason}"
}

failure()
{
	# when we need to bail out of running current test func.
	# args: message string
	echo "FATAL: $1"
	FATAL=1
}

wait_for_syslog_line()
{
	# waits until string(s) appear in syslog or timeout expires

	local string1="$1"
	local string2=
	local counter=0

	if [ -n "$3" ]; then
		string2="$2"
		counter=$3

		# wait for a line containing the string to appear in syslog output
		while [ "$(grep -E "$string1" /var/log/messages 2>/dev/null|\
			grep -E -c "$string2")" = "0" ] && \
			[ $counter -gt 0 ]; do
			sleep 1
			counter=$(($counter - 1))
		done
	else
		counter=$2

		# wait for a line containing both strings to appear in the syslog output
		while [ "$(grep -E -c "$string1" /var/log/messages 2>/dev/null)" = "0" ] && \
			[ $counter -gt 0 ]; do
			sleep 1
			counter=$(($counter - 1))
		done
	fi

	if [ $counter -eq 0 ]; then
		return 1
	else
		return 0
	fi
}

wait_for_runt_value()
{
	# waits for runt value to match expected or timeout expires

	local property="$1"
	local expect=$2
	local counter=$3

	while  [ "$(runt get $property 2>/dev/null)" != "$expect" ] && \
		[ $counter -gt 0 ]; do
		sleep 1
		counter=$(($counter - 1))
	done
	if [ $counter -eq 0 ]; then
		return 1
	else
		return 0
	fi
}

wait_for_sysfs_value()
{
	# waits for sysfs value to match expected or timeout expires

	local sysfs_file="$1"
	local expect=$2
	local counter=$3

	while  [ "$(cat $sysfs_file 2>/dev/null)" != "$expect" ] && \
		[ $counter -gt 0 ]; do
		sleep 1
		counter=$(($counter - 1))
	done
	if [ $counter -eq 0 ]; then
		return 1
	else
		return 0
	fi
}

wait_for_output_value()
{
	# waits for command to output value to match expected or timeout expires

	local command="$1"
	local expect=$2
	local counter=$3

	while  [ "$($command)" != "$expect" ] && \
		[ $counter -gt 0 ]; do
		sleep 1
		counter=$(($counter - 1))
	done
	if [ $counter -eq 0 ]; then
		return 1
	else
		return 0
	fi
}

get_device_lan()
{
	# returns primary_lan_device if found in devicedata.csv for device otherwise tries to guess.

	local device=$(get_device_data primary_lan_device)

	if [ -z "$device" ]; then
		if [ -e /sys/class/net/lan ]; then
			device=lan
		elif [ -e /sys/class/net/eth1 ]; then
			device=eth1
		else
			device=eth0
		fi
	fi
	echo $device
}

get_interface_lan()
{
	# returns primary_lan_interface if found in devicedata.csv for device otherwise tries to guess.

	local device=$(get_device_data primary_lan_interface)

	if [ -z "$device" ]; then
		if [ -e /sys/class/net/lan ]; then
			device=lan
		elif [ -e /sys/class/net/eth1 ]; then
			device=eth1
		else
			device=eth0
		fi
	fi
	echo $device
}

get_interface_modem()
{
	# returns primary_modem_interface if found in devicedata.csv for device otherwise tries to guess.

	local device=$(get_device_data primary_modem_interface)

	if [ -z "$device" ]; then
		if [ -n "$(config get network.interface.modem.surelink.enable 2>/dev/null)" ]; then
			device=modem
		elif [ -n "$(config get network.interface.wwan.surelink.enable 2>/dev/null)" ]; then
			device=wwan
		elif [ -n "$(config get network.interface.wwan1.surelink.enable 2>/dev/null)" ]; then
			device=wwan1
		else
			device=
		fi
	fi
	echo $device
}

start_log()
{
	# setup syslog capture output for test file, called from start of test file.
	# args: $1 = name of test group, eg. example
	# args: $2 = string to filter syslog by

	[ -z "$DIR_NAME" ] && DIR_NAME=$1

	local filter="$2"
	rm -f /tmp/${DIR_NAME}.log
	if [ -n "$filter" ]; then
		(exec 2>&-; tail -F /var/log/messages 2>/dev/null|grep $filter \
			>/tmp/${DIR_NAME}.log) &
	else
		(exec 2>&-; tail -F /var/log/messages 2>/dev/null >/tmp/${DIR_NAME}.log) &
	fi

	zero_stats
}

stop_log()
{
	# teardown syslog capture, called from end of test file.

	local pid=$(fuser /tmp/${DIR_NAME}.log)
	[ -n "$pid" ] && kill $pid >/dev/null 2>&1
	pid=$(ps |grep "tail -F /var/log/messages" 2>/dev/null|\
		grep -v grep 2>/dev/null|awk '{print $1}')
	[ -n "$pid" ] && kill $pid >/dev/null 2>&1

	mkdir -p $OUTPUT_DIR
}

output_result()
{
	# called by passed, failed functions, internal use only.

if [ ! -e /tmp/satest.json ]; then
	echo "[" >/tmp/satest.json

cat <<EOF >/tmp/junit.xml
<?xml version="1.0" encoding="utf-8"?>
  <testsuites time="%%total_duration%%">
    <testsuite name="dal_devtests" errors="0" failures="%%FAILED_COUNT%%" skipped="0" tests="%%test_total%%" time="%%total_duration%%" timestamp="$(date '+%Y-%m-%d %H:%M:%S')" hostname="$(hostname)">
EOF
else
	echo "," >>/tmp/satest.json
fi

cat <<EOF >>/tmp/satest.json
  {
    "title": "$1",
    "description": "$2",
    "reference": "$3",
    "status": "$4",
    "section": "${DIR_NAME}/${TEST_FILE}",
    "implementation": "$5",
    "comments": "$6",
    "defects": "$7",
    "suite": "Automation",
    "version": "$VERSION"
  }
EOF
	local current=$(date +%s)
	DURATION=$(($current - $LAST_TIME))
	LAST_TIME=$current
	if [ "$4" = "failed" ]; then
		echo "<testcase classname='tests.${DIR_NAME}.${TEST_FILE}' name='$1' time='${DURATION}'>" >>/tmp/junit.xml
		echo "<failure message='Test failed' type='AssertionError'></failure></testcase>" >>/tmp/junit.xml
	else
		echo "<testcase classname='tests.${DIR_NAME}.${TEST_FILE}' name='$1' time='${DURATION}' />" >>/tmp/junit.xml
	fi
	TEST_COUNT=$(($TEST_COUNT + 1))
}

output_result_done()
{
	# called from end test file

	if [ -e /tmp/satest.json ]; then
		echo "]" >>/tmp/satest.json
	fi
	if [ -e /tmp/junit.xml ]; then
		echo "</testsuite></testsuites>" >>/tmp/junit.xml
	fi

	local test_total=$(($PASSED_COUNT + $FAILED_COUNT))
	local total_duration=$(date +%s)
	total_duration=$(($total_duration - $BEGIN_TIME))
	echo
	printf "TOTAL:%03d, PASSED:%03d, FAILED:%03d, DURATION:%04ds\n" \
		$test_total $PASSED_COUNT $FAILED_COUNT $total_duration

	local dest=$(echo ${TEST_FILE/test_}|sed 's/\.sh//g')
	if [ -e /tmp/junit.xml ]; then
		cat /tmp/junit.xml | \
			sed 's/%%total_duration%%/'${total_duration}'/g' | \
			sed 's/%%FAILED_COUNT%%/'${FAILED_COUNT}'/g' | \
			sed 's/%%test_total%%/'${test_total}'/g' \
				>${OUTPUT_DIR}/junit-${dest}.xml
	fi
	if [ -e /tmp/satest.json ]; then
		mv /tmp/satest.json ${OUTPUT_DIR}/satest-${dest}.json
	fi

	mv /tmp/${DIR_NAME}.log ${OUTPUT_DIR}/${dest}.log
}

if [ $# -ne 1 ]; then
	echo "This file must be call with argument of test file!"
	exit 1
else
	TEST_FILE=$(basename $1)
	DIR_NAME=$(dirname ${SCRIPT_DIR})
	DIR_NAME=$(basename ${SCRIPT_DIR})

	if [ -n "$EXTRA_LOGS" ]; then
		start_log $DIR_NAME "-e $DIR_NAME -e ${EXTRA_LOGS}"
	else
		start_log $DIR_NAME "-e $DIR_NAME"
	fi

	trap test_cleanup EXIT
fi

#!/bin/bash

SCRIPT_DIR=$(dirname `readlink -f $0`)
. ${SCRIPT_DIR}/../common_functions.sh $0


# configuration for test_example_test1
setup_cfg()
{
	eval $(config start)
	config update

	# config changes here
	config set action.surelink.debug 1

	config validate
	local err=$?
	if [ $err -eq 0 ]; then
		config commit
	else
		failure "Test example config" "err=$err"
	fi

	eval $(config stop)
}

# TESTS

test_example_test1()
{
	log "Starting test_example_test1"

	# save existing config
	save_cfg

	# apply configs
	setup_cfg

	# apply config and return to prompt
	[ -n "$CONFIG_ONLY" ] && exit 1

	if [ $FATAL -eq 0 ]; then
		# This is a test, it just checks the config value was set by setup_cfg
		CHECK=$(config get action.surelink.debug)
		if [ $CHECK -eq 1 ]; then
			passed "Test example setting action.surelink.debug"
		else
			failed "Test example setting action.surelink.debug" "val=$CHECK"
		fi
	else
		echo "FATAL: setup_cfg - error applying config"
	fi

	log "Ending test_example_test1"

	# restore saved config
	restore_cfg
}

# specify the list of tests to run by default. eg. "test_example_test1 test_example_test2"
TEST_FUNCS="test_example_test1"

# run tests
run_tests $@

#!/bin/bash

# clear out devtest test info.
runt delete system.devtests

# reset watchdog error counter if required.
COUNT=$(runt get system.watchdog.num_errors 2>/dev/null)
[ -n "$COUNT" ] && [ "$COUNT" -gt 0 ] && runt set system.watchdog.num_errors 0

# we want system tests to run 1st so we can skip feature testing if device not available.
# system tests will flag failed devices under runt system.devtests

TEST_SCRIPTS=$(find -type f -name "test_*.sh"|grep -i '/system/')
TEST_SCRIPTS="$TEST_SCRIPTS $(find -type f -name "test_*.sh"|grep -v '/system/')"

for file in $TEST_SCRIPTS;do
	$file
done

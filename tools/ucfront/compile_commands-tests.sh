#!/bin/bash

CCTEST=$1

# asserts that when compile_commands.json is processed with jq,
# its output matches stdin.
assert_jq () {
	jq "$@" >/dev/null <compile_commands.json || exit 1
	diff --label "$(caller) expected" /dev/stdin \
	     --label actual -u <(jq "$@" <compile_commands.json)
}

set -e

# Creating a new file fresh
rm -f compile_commands.json*
$CCTEST -d . -f foo.c -- cc -Dname=\"it\'s\ a\ wrap\" -c foo.c
assert_jq -S -c <<.
[{"arguments":["cc","-Dname=\"it's a wrap\"","-c","foo.c"],"directory":".","file":"foo.c"}]
.

# idempotent on singleton record
$CCTEST -d . -f foo.c -- cc -Dname=\"it\'s\ a\ wrap\" -c foo.c
assert_jq -S -c <<.
[{"arguments":["cc","-Dname=\"it's a wrap\"","-c","foo.c"],"directory":".","file":"foo.c"}]
.

# Replacing a singleton record with something smaller works
$CCTEST -d . -f foo.c -- cc -c x.c
assert_jq -S -c <<.
[{"arguments":["cc","-c","x.c"],"directory":".","file":"foo.c"}]
.

# Replacing a singleton record with a record of the same length works
$CCTEST -d . -f foo.c -- cc -c f.c
assert_jq -S -c <<.
[{"arguments":["cc","-c","f.c"],"directory":".","file":"foo.c"}]
.

# Replacing a singleton record with a longer record works
$CCTEST -d . -f foo.c -- cc -c foo.c
assert_jq -S -c <<.
[{"arguments":["cc","-c","foo.c"],"directory":".","file":"foo.c"}]
.

# Adding a second record works (./bar.c)
$CCTEST -d . -f bar.c -- cc -c bar.c
assert_jq -S -c sort <<.
[{"arguments":["cc","-c","bar.c"],"directory":".","file":"bar.c"}\
,{"arguments":["cc","-c","foo.c"],"directory":".","file":"foo.c"}]
.

# idempotent on 2nd record
$CCTEST -d . -f bar.c -- cc -c bar.c
assert_jq -S -c sort <<.
[{"arguments":["cc","-c","bar.c"],"directory":".","file":"bar.c"}\
,{"arguments":["cc","-c","foo.c"],"directory":".","file":"foo.c"}]
.

# Shrinking the first record (./foo.c) works
$CCTEST -d . -f foo.c -- cc -c f.c
assert_jq -S -c sort <<.
[{"arguments":["cc","-c","bar.c"],"directory":".","file":"bar.c"}\
,{"arguments":["cc","-c","f.c"],"directory":".","file":"foo.c"}]
.

# Growing the first record works
$CCTEST -d . -f foo.c -- cc -c fff.c
assert_jq -S -c sort <<.
[{"arguments":["cc","-c","bar.c"],"directory":".","file":"bar.c"}\
,{"arguments":["cc","-c","fff.c"],"directory":".","file":"foo.c"}]
.

# Replacing the first record works
$CCTEST -d . -f foo.c -- cc -c foo.c
assert_jq -S -c sort <<.
[{"arguments":["cc","-c","bar.c"],"directory":".","file":"bar.c"}\
,{"arguments":["cc","-c","foo.c"],"directory":".","file":"foo.c"}]
.

# Adding third record, with an output field, works
$CCTEST -d . -f zap.c -o zap.o -- cc -c zap.c
assert_jq -S -c sort <<.
[{"arguments":["cc","-c","bar.c"],"directory":".","file":"bar.c"}\
,{"arguments":["cc","-c","foo.c"],"directory":".","file":"foo.c"}\
,{"arguments":["cc","-c","zap.c"],"directory":".","file":"zap.c","output":"zap.o"}]
.


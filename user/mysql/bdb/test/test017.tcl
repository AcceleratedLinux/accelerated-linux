# See the file LICENSE for redistribution information.
#
# Copyright (c) 1996, 1997, 1998, 1999, 2000
#	Sleepycat Software.  All rights reserved.
#
#	$Id: test017.tcl,v 11.13 2000/12/11 17:42:18 sue Exp $
#
# DB Test 17 {access method}
# Run duplicates with small page size so that we test off page duplicates.
# Then after we have an off-page database, test with overflow pages too.
#
proc test017 { method {contents 0} {ndups 19} {tnum 17} args } {
	source ./include.tcl

	set args [convert_args $method $args]
	set omethod [convert_method $method]

	if { [is_record_based $method] == 1 || \
	    [is_rbtree $method] == 1 } {
		puts "Test0$tnum skipping for method $method"
		return
	}
	set pgindex [lsearch -exact $args "-pagesize"]
	if { $pgindex != -1 } {
		incr pgindex
		if { [lindex $args $pgindex] > 8192 } {
			puts "Test0$tnum: Skipping for large pagesizes"
			return
		}
	}

	puts "Test0$tnum: $method ($args) Off page duplicate tests with $ndups duplicates"

	# Create the database and open the dictionary
	set eindex [lsearch -exact $args "-env"]
	#
	# If we are using an env, then testfile should just be the db name.
	# Otherwise it is the test directory and the name.
	if { $eindex == -1 } {
		set testfile $testdir/test0$tnum.db
		set env NULL
	} else {
		set testfile test0$tnum.db
		incr eindex
		set env [lindex $args $eindex]
	}
	set t1 $testdir/t1
	set t2 $testdir/t2
	set t3 $testdir/t3
	set t4 $testdir/t4

	cleanup $testdir $env

	set db [eval {berkdb_open \
	     -create -truncate -mode 0644 -dup} $args {$omethod $testfile}]
	error_check_good dbopen [is_valid_db $db] TRUE

	set pflags ""
	set gflags ""
	set txn ""
	set count 0

	set ovfl ""
	# Here is the loop where we put and get each key/data pair
	set dbc [eval {$db cursor} $txn]
	puts -nonewline \
	    "\tTest0$tnum.a: Creating duplicates with "
	if { $contents != 0 } {
		puts "file contents as key/data"
	} else {
		puts "file name as key/data"
	}
	set file_list [glob ../*/*.c ./*.lo]
	foreach f $file_list {
		if { $contents != 0 } {
			set fid [open $f r]
			fconfigure $fid -translation binary
			#
			# Prepend file name to guarantee uniqueness
			set filecont [read $fid]
			set str $f:$filecont
			close $fid
		} else {
			set str $f
		}
		for { set i 1 } { $i <= $ndups } { incr i } {
			set datastr $i:$str
			set ret [eval {$db put} \
			    $txn $pflags {$str [chop_data $method $datastr]}]
			error_check_good put $ret 0
		}

		#
		# Save 10% files for overflow test
		#
		if { $contents == 0 && [expr $count % 10] == 0 } {
			lappend ovfl $f
		}
		# Now retrieve all the keys matching this key
		set ret [$db get $str]
		error_check_bad $f:dbget_dups [llength $ret] 0
		error_check_good $f:dbget_dups1 [llength $ret] $ndups
		set x 1
		for {set ret [$dbc get "-set" $str]} \
		    {[llength $ret] != 0} \
		    {set ret [$dbc get "-next"] } {
			set k [lindex [lindex $ret 0] 0]
			if { [string compare $k $str] != 0 } {
				break
			}
			set datastr [lindex [lindex $ret 0] 1]
			set d [data_of $datastr]
			if {[string length $d] == 0} {
				break
			}
			error_check_good "Test0$tnum:get" $d $str
			set id [ id_of $datastr ]
			error_check_good "Test0$tnum:$f:dup#" $id $x
			incr x
		}
		error_check_good "Test0$tnum:ndups:$str" [expr $x - 1] $ndups
		incr count
	}
	error_check_good cursor_close [$dbc close] 0

	# Now we will get each key from the DB and compare the results
	# to the original.
	puts "\tTest0$tnum.b: Checking file for correct duplicates"
	set dlist ""
	for { set i 1 } { $i <= $ndups } {incr i} {
		lappend dlist $i
	}
	set oid [open $t2.tmp w]
	set o1id [open $t4.tmp w]
	foreach f $file_list {
		for {set i 1} {$i <= $ndups} {incr i} {
			puts $o1id $f
		}
		puts $oid $f
	}
	close $oid
	close $o1id
	filesort $t2.tmp $t2
	filesort $t4.tmp $t4
	fileremove $t2.tmp
	fileremove $t4.tmp

	dup_check $db $txn $t1 $dlist
	if {$contents == 0} {
		filesort $t1 $t3

		error_check_good Test0$tnum:diff($t3,$t2) \
		    [filecmp $t3 $t2] 0

		# Now compare the keys to see if they match the file names
		dump_file $db $txn $t1 test017.check
		filesort $t1 $t3

		error_check_good Test0$tnum:diff($t3,$t4) \
		    [filecmp $t3 $t4] 0
	}

	error_check_good db_close [$db close] 0
	set db [eval {berkdb_open} $args $testfile]
	error_check_good dbopen [is_valid_db $db] TRUE

	puts "\tTest0$tnum.c: Checking file for correct duplicates after close"
	dup_check $db $txn $t1 $dlist

	if {$contents == 0} {
		# Now compare the keys to see if they match the filenames
		filesort $t1 $t3
		error_check_good Test0$tnum:diff($t3,$t2) \
		    [filecmp $t3 $t2] 0
	}
	error_check_good db_close [$db close] 0

	puts "\tTest0$tnum.d: Verify off page duplicates and overflow status"
	set db [eval {berkdb_open} $args $testfile]
	error_check_good dbopen [is_valid_db $db] TRUE
	set stat [$db stat]
	if { [is_btree $method] } {
		error_check_bad stat:offpage \
		    [is_substr $stat "{{Internal pages} 0}"] 1
	}
	if {$contents == 0} {
		# This check doesn't work in hash, since overflow
		# pages count extra pages in buckets as well as true
		# P_OVERFLOW pages.
		if { [is_hash $method] == 0 } {
			error_check_good overflow \
			    [is_substr $stat "{{Overflow pages} 0}"] 1
		}
	} else {
		error_check_bad overflow \
		    [is_substr $stat "{{Overflow pages} 0}"] 1
	}

	#
	# If doing overflow test, do that now.  Else we are done.
	# Add overflow pages by adding a large entry to a duplicate.
	#
	if { [llength $ovfl] == 0} {
		error_check_good db_close [$db close] 0
		return
	}
	puts "\tTest0$tnum.e: Add overflow duplicate entries"
	set ovfldup [expr $ndups + 1]
	foreach f $ovfl {
		#
		# This is just like put_file, but prepends the dup number
		#
		set fid [open $f r]
		fconfigure $fid -translation binary
		set fdata [read $fid]
		close $fid
		set data $ovfldup:$fdata

		set ret [eval {$db put} $txn $pflags {$f $data}]
		error_check_good ovfl_put $ret 0
	}
	puts "\tTest0$tnum.f: Verify overflow duplicate entries"
	dup_check $db $txn $t1 $dlist $ovfldup
	filesort $t1 $t3
	error_check_good Test0$tnum:diff($t3,$t2) \
	    [filecmp $t3 $t2] 0

	set stat [$db stat]
	error_check_bad overflow1 \
	    [is_substr $stat "{{Overflow pages} 0}"] 1
	error_check_good db_close [$db close] 0
}

# Check function; verify data contains key
proc test017.check { key data } {
	error_check_good "data mismatch for key $key" $key [data_of $data]
}

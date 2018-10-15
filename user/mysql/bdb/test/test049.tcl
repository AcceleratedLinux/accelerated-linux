# See the file LICENSE for redistribution information.
#
# Copyright (c) 1999, 2000
#	Sleepycat Software.  All rights reserved.
#
#	$Id: test049.tcl,v 11.15 2000/08/25 14:21:56 sue Exp $
#
# Test 049: Test of each cursor routine with unitialized cursors
proc test049 { method args } {
	global errorInfo
	global errorCode
	source ./include.tcl

	set tstn 049
	set renum [is_rrecno $method]

	set args [convert_args $method $args]
	set omethod [convert_method $method]

	puts "\tTest$tstn: Test of cursor routines with unitialized cursors."

	set key	"key"
	set data	"data"
	set txn ""
	set flags ""
	set rflags ""

	if { [is_record_based $method] == 1 } {
		set key ""
	}

	puts "\tTest$tstn.a: Create $method database."
	set eindex [lsearch -exact $args "-env"]
	#
	# If we are using an env, then testfile should just be the db name.
	# Otherwise it is the test directory and the name.
	if { $eindex == -1 } {
		set testfile $testdir/test0$tstn.db
		set env NULL
	} else {
		set testfile test0$tstn.db
		incr eindex
		set env [lindex $args $eindex]
	}
	set t1 $testdir/t1
	cleanup $testdir $env

	set oflags "-create -truncate -mode 0644 $rflags $omethod $args"
	if { [is_record_based $method] == 0 && [is_rbtree $method] != 1 } {
		append oflags " -dup"
	}
	set db [eval {berkdb_open_noerr} $oflags $testfile]
	error_check_good dbopen [is_valid_db $db] TRUE

	set dbc_u [$db cursor]
	error_check_good db:cursor [is_substr $dbc_u $db] 1

	set nkeys 10
	puts "\tTest$tstn.b: Fill page with $nkeys small key/data pairs."
	for { set i 1 } { $i <= $nkeys } { incr i } {
		set ret [$db put $key$i $data$i]
		error_check_good dbput:$i $ret 0
		if { $i == 1 } {
			for {set j 0} { $j < [expr $nkeys / 2]} {incr j} {
				set ret [$db put $key$i DUPLICATE$j]
				error_check_good dbput:dup:$j $ret 0
			}
		}
	}

	# DBC GET
	puts "\tTest$tstn.c: Test dbc->get interfaces..."
	set i 0
	foreach flag { current first last next prev nextdup} {
		puts "\t\t...dbc->get($flag)"
		catch {$dbc_u get -$flag} ret
		error_check_good dbc:get:$flag [is_substr $errorCode EINVAL] 1
	}

	foreach flag { set set_range get_both} {
		puts "\t\t...dbc->get($flag)"
		if { [string compare $flag get_both] == 0} {
			catch {$dbc_u get -$flag $key$i data0} ret
		} else {
			catch {$dbc_u get -$flag $key$i} ret
		}
		error_check_good dbc:get:$flag [is_substr $errorCode EINVAL] 1
	}

	puts "\t\t...dbc->get(current, partial)"
	catch {$dbc_u get -current -partial {0 0}} ret
	error_check_good dbc:get:partial [is_substr $errorCode EINVAL] 1

	puts "\t\t...dbc->get(current, rmw)"
	catch {$dbc_u get -rmw -current } ret
	error_check_good dbc_get:rmw [is_substr $errorCode EINVAL] 1

	puts "\tTest$tstn.d: Test dbc->put interface..."
	# partial...depends on another
	foreach flag { after before current keyfirst keylast } {
		puts "\t\t...dbc->put($flag)"
		if { [string match key* $flag] == 1 } {
			  if { [is_record_based $method] == 1 } {
				# keyfirst/keylast not allowed in recno
			puts "\t\t...Skipping dbc->put($flag) for $method."
				continue
			  } else {
				# keyfirst/last should succeed
		puts "\t\t...dbc->put($flag)...should succeed for $method"
				error_check_good dbcput:$flag \
				    [$dbc_u put -$flag $key$i data0] 0

				# now uninitialize cursor
				error_check_good dbc_close [$dbc_u close] 0
				set dbc_u [$db cursor]
				error_check_good \
				    db_cursor [is_substr $dbc_u $db] 1
			}
		} elseif { [string compare $flag before ] == 0 ||
		    [string compare $flag after ] == 0 } {
			if { [is_record_based $method] == 0 &&
			    [is_rbtree $method] == 0} {
				set ret [$dbc_u put -$flag data0]
				error_check_good "$dbc_u:put:-$flag" $ret 0
			} elseif { $renum == 1 } {
				# Renumbering recno will return a record number
				set currecno \
				    [lindex [lindex [$dbc_u get -current] 0] 0]
				set ret [$dbc_u put -$flag data0]
				if { [string compare $flag after] == 0 } {
					error_check_good "$dbc_u put $flag" \
					    $ret [expr $currecno + 1]
				} else {
					error_check_good "$dbc_u put $flag" \
					    $ret $currecno
				}
			} else {
				puts "\t\tSkipping $flag for $method"
			}
		} else {
			set ret [$dbc_u put -$flag data0]
			error_check_good "$dbc_u:put:-$flag" $ret 0
		}
	}
	# and partial
	puts "\t\t...dbc->put(partial)"
	catch {$dbc_u put -partial {0 0} $key$i $data$i} ret
	error_check_good dbc_put:partial [is_substr $errorCode EINVAL] 1

	# XXX dbc->dup, db->join (dbc->get join_item)
	# dbc del
	puts "\tTest$tstn.e: Test dbc->del interface."
	catch {$dbc_u del} ret
	error_check_good dbc_del [is_substr $errorCode EINVAL] 1

	error_check_good dbc_close [$dbc_u close] 0
	error_check_good db_close [$db close] 0

	puts "\tTest$tstn complete."
}

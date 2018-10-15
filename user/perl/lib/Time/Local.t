#!./perl

BEGIN {
  if ($ENV{PERL_CORE}){
    chdir('t') if -d 't';
    @INC = ('.', '../lib');
  }
}

use strict;

use Config;
use Test;
use Time::Local;

# Set up time values to test
my @time =
  (
   #year,mon,day,hour,min,sec
   [1970,  1,  2, 00, 00, 00],
   [1980,  2, 28, 12, 00, 00],
   [1980,  2, 29, 12, 00, 00],
   [1999, 12, 31, 23, 59, 59],
   [2000,  1,  1, 00, 00, 00],
   [2010, 10, 12, 14, 13, 12],
   # leap day
   [2020,  2, 29, 12, 59, 59],
   [2030,  7,  4, 17, 07, 06],
# The following test fails on a surprising number of systems
# so it is commented out. The end of the Epoch for a 32-bit signed
# implementation of time_t should be Jan 19, 2038  03:14:07 UTC.
#  [2038,  1, 17, 23, 59, 59],     # last full day in any tz
  );

my @bad_time =
    (
     # month too large
     [1995, 13, 01, 01, 01, 01],
     # day too large
     [1995, 02, 30, 01, 01, 01],
     # hour too large
     [1995, 02, 10, 25, 01, 01],
     # minute too large
     [1995, 02, 10, 01, 60, 01],
     # second too large
     [1995, 02, 10, 01, 01, 60],
    );

my @neg_time =
    (
     # test negative epochs for systems that handle it
     [ 1969, 12, 31, 16, 59, 59 ],
     [ 1950, 04, 12, 9, 30, 31 ],
    );

# Use 3 days before the start of the epoch because with Borland on
# Win32 it will work for -3600 _if_ your time zone is +01:00 (or
# greater).
my $neg_epoch_ok = defined ((localtime(-259200))[0]) ? 1 : 0;

# use vmsish 'time' makes for oddness around the Unix epoch
if ($^O eq 'VMS') { 
    $time[0][2]++;
    $neg_epoch_ok = 0; # time_t is unsigned
}

my $tests = (@time * 12);
$tests += @neg_time * 12;
$tests += @bad_time;
$tests += 8;
$tests += 2 if $ENV{PERL_CORE};
$tests += 5 if $ENV{MAINTAINER};

plan tests => $tests;

for (@time, @neg_time) {
    my($year, $mon, $mday, $hour, $min, $sec) = @$_;
    $year -= 1900;
    $mon--;

    if ($^O eq 'vos' && $year == 70) {
        skip(1, "skipping 1970 test on VOS.\n") for 1..6;
    } elsif ($year < 70 && ! $neg_epoch_ok) {
        skip(1, "skipping negative epoch.\n") for 1..6;
    } else {
        my $year_in = $year < 70 ? $year + 1900 : $year;
        my $time = timelocal($sec,$min,$hour,$mday,$mon,$year_in);

        my($s,$m,$h,$D,$M,$Y) = localtime($time);

        ok($s, $sec, 'timelocal second');
        ok($m, $min, 'timelocal minute');
        ok($h, $hour, 'timelocal hour');
        ok($D, $mday, 'timelocal day');
        ok($M, $mon, 'timelocal month');
        ok($Y, $year, 'timelocal year');
    }

    if ($^O eq 'vos' && $year == 70) {
        skip(1, "skipping 1970 test on VOS.\n") for 1..6;
    } elsif ($year < 70 && ! $neg_epoch_ok) {
        skip(1, "skipping negative epoch.\n") for 1..6;
    } else {
        my $year_in = $year < 70 ? $year + 1900 : $year;
        my $time = timegm($sec,$min,$hour,$mday,$mon,$year_in);

        my($s,$m,$h,$D,$M,$Y) = gmtime($time);

        ok($s, $sec, 'timegm second');
        ok($m, $min, 'timegm minute');
        ok($h, $hour, 'timegm hour');
        ok($D, $mday, 'timegm day');
        ok($M, $mon, 'timegm month');
        ok($Y, $year, 'timegm year');
    }
}

for (@bad_time) {
    my($year, $mon, $mday, $hour, $min, $sec) = @$_;
    $year -= 1900;
    $mon--;

    eval { timegm($sec,$min,$hour,$mday,$mon,$year) };

    ok($@, qr/.*out of range.*/, 'invalid time caused an error');
}

ok(timelocal(0,0,1,1,0,90) - timelocal(0,0,0,1,0,90), 3600,
   'one hour difference between two calls to timelocal');

ok(timelocal(1,2,3,1,0,100) - timelocal(1,2,3,31,11,99), 24 * 3600,
   'one day difference between two calls to timelocal');

# Diff beween Jan 1, 1980 and Mar 1, 1980 = (31 + 29 = 60 days)
ok(timegm(0,0,0, 1, 2, 80) - timegm(0,0,0, 1, 0, 80), 60 * 24 * 3600,
   '60 day difference between two calls to timegm');

# bugid #19393
# At a DST transition, the clock skips forward, eg from 01:59:59 to
# 03:00:00. In this case, 02:00:00 is an invalid time, and should be
# treated like 03:00:00 rather than 01:00:00 - negative zone offsets used
# to do the latter
{
    my $hour = (localtime(timelocal(0, 0, 2, 7, 3, 102)))[2];
    # testers in US/Pacific should get 3,
    # other testers should get 2
    ok($hour == 2 || $hour == 3, 1, 'hour should be 2 or 3');
}

if ($neg_epoch_ok) {
    eval { timegm(0,0,0,29,1,1900) };
    ok($@, qr/Day '29' out of range 1\.\.28/);

    eval { timegm(0,0,0,29,1,1904) };
    ok($@, '');
} else {
    skip(1, "skipping negative epoch.\n") for 1..2;
}

# round trip was broken for edge cases
if ($^O eq "aix" && $Config{osvers} =~ m/^4\.3\./) {
    skip( 1, "No fix expected for edge case test for $_ on AIX 4.3") for qw( timegm timelocal );
} else {
    ok(sprintf('%x', timegm(gmtime(0x7fffffff))), sprintf('%x', 0x7fffffff),
       '0x7fffffff round trip through gmtime then timegm');

    ok(sprintf('%x', timelocal(localtime(0x7fffffff))), sprintf('%x', 0x7fffffff),
       '0x7fffffff round trip through localtime then timelocal');
}

if ($ENV{MAINTAINER}) {
    eval { require POSIX; POSIX::tzset() };
    if ($@) {
        skip( 1, "Cannot call POSIX::tzset() on this platform\n" ) for 1..3;
    }
    else {
        local $ENV{TZ} = 'Europe/Vienna';
        POSIX::tzset();

        # 2001-10-28 02:30:00 - could be either summer or standard time,
        # prefer earlier of the two, in this case summer
        my $time = timelocal(0, 30, 2, 28, 9, 101);
        ok($time, 1004229000,
           'timelocal prefers earlier epoch in the presence of a DST change');

        local $ENV{TZ} = 'America/Chicago';
        POSIX::tzset();

        # Same local time in America/Chicago.  There is a transition
        # here as well.
        $time = timelocal(0, 30, 1, 28, 9, 101);
        ok($time, 1004250600,
           'timelocal prefers earlier epoch in the presence of a DST change');

        $time = timelocal(0, 30, 2, 1, 3, 101);
        ok($time, 986113800,
           'timelocal for non-existent time gives you the time one hour later');

        local $ENV{TZ} = 'Australia/Sydney';
        POSIX::tzset();

        # 2001-03-25 02:30:00 in Australia/Sydney.  This is the transition
        # _to_ summer time.  The southern hemisphere transitions are
        # opposite those of the northern.
        $time = timelocal(0, 30, 2, 25, 2, 101);
        ok($time, 985447800,
           'timelocal prefers earlier epoch in the presence of a DST change');

        $time = timelocal(0, 30, 2, 28, 9, 101);
        ok($time, 1004200200,
           'timelocal for non-existent time gives you the time one hour later');
    }
}

if ($ENV{PERL_CORE}) {
  package test;
  require 'timelocal.pl';

  # need to get ok() from main package
  ::ok(timegm(0,0,0,1,0,80), main::timegm(0,0,0,1,0,80),
     'timegm in timelocal.pl');

  ::ok(timelocal(1,2,3,4,5,88), main::timelocal(1,2,3,4,5,88),
     'timelocal in timelocal.pl');
}

#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

use strict;
use warnings;

use vars qw{ @warnings };

BEGIN {
    $SIG{'__WARN__'} = sub { push @warnings, @_ };
    $| = 1;
    print "1..9\n";
}

END { print "not ok\n# Uncaught warnings:\n@warnings\n" if @warnings }

sub test ($$;$) {
    my($num, $bool, $diag) = @_;
    if ($bool) {
	print "ok $num\n";
	return;
    }
    print "not ok $num\n";
    return unless defined $diag;
    $diag =~ s/\Z\n?/\n/;			# unchomp
    print map "# $num : $_", split m/^/m, $diag;
}

sub test_warning ($$$) {
    my($num, $got, $expected) = @_;
    my($pattern, $ok);
    if (($pattern) = ($expected =~ m#^/(.+)/$#s) or
	(undef, $pattern) = ($expected =~ m#^m([^\w\s])(.+)\1$#s)) {
	    # it's a regexp
	    $ok = ($got =~ /$pattern/);
	    test $num, $ok, "Expected pattern /$pattern/, got '$got'\n";
    } else {
	$ok = ($got eq $expected);
	test $num, $ok, "Expected string '$expected', got '$got'\n";
    }
#   print "# $num: $got\n";
}

my $odd_msg = '/^Odd number of elements in hash assignment/';
my $odd_msg2 = '/^Odd number of elements in anonymous hash/';
my $ref_msg = '/^Reference found where even-sized list expected/';

{
    my %hash = (1..3);
    test_warning 1, shift @warnings, $odd_msg;

    %hash = 1;
    test_warning 2, shift @warnings, $odd_msg;

    %hash = { 1..3 };
    test_warning 3, shift @warnings, $odd_msg2;
    test_warning 4, shift @warnings, $ref_msg;

    %hash = [ 1..3 ];
    test_warning 5, shift @warnings, $ref_msg;

    %hash = sub { print "ok" };
    test_warning 6, shift @warnings, $odd_msg;

    {
	# "Pseudo-hashes are deprecated" warnings tested in warnings/av
	no warnings 'deprecated';

	my $avhv = [{x=>1,y=>2}];
	%$avhv = (x=>13,'y');
	test_warning 7, shift @warnings, $odd_msg;

	%$avhv = 'x';
	test_warning 8, shift @warnings, $odd_msg;

	$_ = { 1..10 };
	test 9, ! @warnings, "Unexpected warning";
    }
}

#!./perl -w

BEGIN {
    if ($ENV{PERL_CORE}){
	chdir('t') if -d 't';
	@INC = ('.', '../lib');
    } else {
	unshift @INC, 't';
	push @INC, "../../t";
    }
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
    require 'test.pl';
}

plan tests => 15; # adjust also number of skipped tests !

# Runs a separate perl interpreter with the appropriate lint options
# turned on
sub runlint ($$$;$) {
    my ($opts,$prog,$result,$testname) = @_;
    my $res = runperl(
	switches => [ "-MO=Lint,$opts" ],
	prog	 => $prog,
	stderr	 => 1,
    );
    $res =~ s/-e syntax OK\n$//;
    is( $res, $result, $testname || $opts );
}

runlint 'context', '$foo = @bar', <<'RESULT';
Implicit scalar context for array in scalar assignment at -e line 1
RESULT

runlint 'context', '$foo = length @bar', <<'RESULT';
Implicit scalar context for array in length at -e line 1
RESULT

runlint 'implicit-read', '/foo/', <<'RESULT';
Implicit match on $_ at -e line 1
RESULT

runlint 'implicit-write', 's/foo/bar/', <<'RESULT';
Implicit substitution on $_ at -e line 1
RESULT

SKIP : {

    use Config;
    skip("Doesn't work with threaded perls",11)
       if $Config{useithreads} || ($] < 5.009 && $Config{use5005threads});

    runlint 'implicit-read', '1 for @ARGV', <<'RESULT', 'implicit-read in foreach';
Implicit use of $_ in foreach at -e line 1
RESULT

    runlint 'dollar-underscore', '$_ = 1', <<'RESULT';
Use of $_ at -e line 1
RESULT

    runlint 'dollar-underscore', 'print', <<'RESULT', 'dollar-underscore in print';
Use of $_ at -e line 1
RESULT

    runlint 'private-names', 'sub A::_f{};A::_f()', <<'RESULT';
Illegal reference to private name _f at -e line 1
RESULT

    runlint 'private-names', '$A::_x', <<'RESULT';
Illegal reference to private name _x at -e line 1
RESULT

    runlint 'private-names', 'sub A::_f{};A->_f()', <<'RESULT',
Illegal reference to private method name _f at -e line 1
RESULT
    'private-names (method)';

    runlint 'undefined-subs', 'foo()', <<'RESULT';
Undefined subroutine foo called at -e line 1
RESULT

    runlint 'regexp-variables', 'print $&', <<'RESULT';
Use of regexp variable $& at -e line 1
RESULT

    runlint 'regexp-variables', 's/./$&/', <<'RESULT';
Use of regexp variable $& at -e line 1
RESULT

    runlint 'bare-subs', 'sub bare(){1};$x=bare', '';

    runlint 'bare-subs', 'sub bare(){1}; $x=[bare=>0]; $x=$y{bare}', <<'RESULT';
Bare sub name 'bare' interpreted as string at -e line 1
Bare sub name 'bare' interpreted as string at -e line 1
RESULT

}

#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    unless (find PerlIO::Layer 'perlio') {
	print "1..0 # Skip: not perlio\n";
	exit 0;
    }
    require Config;
    if (($Config::Config{'extensions'} !~ m!\bPerlIO/scalar\b!) ){
        print "1..0 # Skip -- Perl configured without PerlIO::scalar module\n";
        exit 0;
    }
}

$| = 1;
print "1..27\n";

my $fh;
my $var = "ok 2\n";
open($fh,"+<",\$var) or print "not ";
print "ok 1\n";
print <$fh>;
print "not " unless eof($fh);
print "ok 3\n";
seek($fh,0,0) or print "not ";
print "not " if eof($fh);
print "ok 4\n";
print "ok 5\n";
print $fh "ok 7\n" or print "not ";
print "ok 6\n";
print $var;
$var = "foo\nbar\n";
seek($fh,0,0) or print "not ";
print "not " if eof($fh);
print "ok 8\n";
print "not " unless <$fh> eq "foo\n";
print "ok 9\n";
my $rv = close $fh;
if (!$rv) {
    print "# Close on scalar failed: $!\n";
    print "not ";
}
print "ok 10\n";

# Test that semantics are similar to normal file-based I/O
# Check that ">" clobbers the scalar
$var = "Something";
open $fh, ">", \$var;
print "# Got [$var], expect []\n";
print "not " unless $var eq "";
print "ok 11\n";
#  Check that file offset set to beginning of scalar
my $off = tell($fh);
print "# Got $off, expect 0\n";
print "not " unless $off == 0;
print "ok 12\n";
# Check that writes go where they should and update the offset
$var = "Something";
print $fh "Brea";
$off = tell($fh);
print "# Got $off, expect 4\n";
print "not " unless $off == 4;
print "ok 13\n";
print "# Got [$var], expect [Breathing]\n";
print "not " unless $var eq "Breathing";
print "ok 14\n";
close $fh;

# Check that ">>" appends to the scalar
$var = "Something ";
open $fh, ">>", \$var;
$off = tell($fh);
print "# Got $off, expect 10\n";
print "not " unless $off == 10;
print "ok 15\n";
print "# Got [$var], expect [Something ]\n";
print "not " unless $var eq "Something ";
print "ok 16\n";
#  Check that further writes go to the very end of the scalar
$var .= "else ";
print "# Got [$var], expect [Something else ]\n";
print "not " unless $var eq "Something else ";
print "ok 17\n";
$off = tell($fh);
print "# Got $off, expect 10\n";
print "not " unless $off == 10;
print "ok 18\n";
print $fh "is here";
print "# Got [$var], expect [Something else is here]\n";
print "not " unless $var eq "Something else is here";
print "ok 19\n";
close $fh;

# Check that updates to the scalar from elsewhere do not
# cause problems
$var = "line one\nline two\line three\n";
open $fh, "<", \$var;
while (<$fh>) {
    $var = "foo";
}
close $fh;
print "# Got [$var], expect [foo]\n";
print "not " unless $var eq "foo";
print "ok 20\n";

# Check that dup'ing the handle works

$var = '';

open $fh, "+>", \$var;
print $fh "ok 21\n";
open $dup,'+<&',$fh;
print $dup "ok 22\n";
seek($dup,0,0);
while (<$dup>) {
    print;
}
close($fh);
close($dup);

# Check reading from non-string scalars

open $fh, '<', \42;
print <$fh> eq "42" ? "ok 23\n" : "not ok 23\n";
close $fh;

# reading from magic scalars

{ package P; sub TIESCALAR {bless{}} sub FETCH {"ok 24\n"} }
tie $p, P; open $fh, '<', \$p;
print <$fh>;

# don't warn when writing to an undefined scalar

{
    use warnings;
    my $ok = 1;
    local $SIG{__WARN__} = sub { $ok = 0; };
    open my $fh, '>', \my $scalar;
    print $fh "foo";
    close $fh;
    print $ok ? "ok 25\n" : "not ok 25\n";
}

my $data = "a non-empty PV";
$data = undef;
open(MEM, '<', \$data) or die "Fail: $!\n";
my $x = join '', <MEM>;
print $x eq '' ? "ok 26\n" : "not ok 26\n";

{
    # [perl #35929] verify that works with $/ (i.e. test PerlIOScalar_unread)
    my $s = <<'EOF';
line A
line B
a third line
EOF
    open(F, '<', \$s) or die "Could not open string as a file";
    local $/ = "";
    my $ln = <F>;
    close F;
    print $ln eq $s ? "ok 27\n" : "not ok 27\n";
}

#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

print "1..28\n";

my $i = 1;
eval "use 5.000";	# implicit semicolon
if ($@) {
    print STDERR $@,"\n";
    print "not ";
}
print "ok ",$i++,"\n";

eval "use 5.000;";
if ($@) {
    print STDERR $@,"\n";
    print "not ";
}
print "ok ",$i++,"\n";

eval sprintf "use %.6f;", $];
if ($@) {
    print STDERR $@,"\n";
    print "not ";
}
print "ok ",$i++,"\n";


eval sprintf "use %.6f;", $] - 0.000001;
if ($@) {
    print STDERR $@,"\n";
    print "not ";
}
print "ok ",$i++,"\n";

eval sprintf("use %.6f;", $] + 1);
unless ($@) {
    print "not ";
}
print "ok ",$i++,"\n";

eval sprintf "use %.6f;", $] + 0.00001;
unless ($@) {
    print "not ";
}
print "ok ",$i++,"\n";


{ use lib }	# check that subparse saves pending tokens

local $lib::VERSION = 1.0;

eval "use lib 0.9";
if ($@) {
    print STDERR $@,"\n";
    print "not ";
}
print "ok ",$i++,"\n";

eval "use lib 1.0";
if ($@) {
    print STDERR $@,"\n";
    print "not ";
}
print "ok ",$i++,"\n";

eval "use lib 1.01";
unless ($@) {
    print "not ";
}
print "ok ",$i++,"\n";


eval "use lib 0.9 qw(fred)";
if ($@) {
    print STDERR $@,"\n";
    print "not ";
}
print "ok ",$i++,"\n";

print "not " unless ($INC[0] eq "fred" || ($^O eq 'MacOS' && $INC[0] eq ":fred:"));
print "ok ",$i++,"\n";

eval "use lib 1.0 qw(joe)";
if ($@) {
    print STDERR $@,"\n";
    print "not ";
}
print "ok ",$i++,"\n";

print "not " unless ($INC[0] eq "joe" || ($^O eq 'MacOS' && $INC[0] eq ":joe:"));
print "ok ",$i++,"\n";

eval "use lib 1.01 qw(freda)";
unless ($@) {
    print "not ";
}
print "ok ",$i++,"\n";

print "not " if ($INC[0] eq "freda" || ($^O eq 'MacOS' && $INC[0] eq ":freda:"));
print "ok ",$i++,"\n";

{
    local $lib::VERSION = 35.36;
    eval "use lib v33.55";
    print "not " if $@;
    print "ok ",$i++,"\n";

    eval "use lib v100.105";
    unless ($@ =~ /lib version 100\.105 required--this is only version 35\.3/) {
	print "not ";
    }
    print "ok ",$i++,"\n";

    eval "use lib 33.55";
    print "not " if $@;
    print "ok ",$i++,"\n";

    eval "use lib 100.105";
    unless ($@ =~ /lib version 100\.105 required--this is only version 35\.3/) {
	print "not ";
    }
    print "ok ",$i++,"\n";

    local $lib::VERSION = '35.36';
    eval "use lib v33.55";
    print "not " if $@;
    print "ok ",$i++,"\n";

    eval "use lib v100.105";
    unless ($@ =~ /lib version 100\.105 required--this is only version 35\.36/) {
	print "not ";
    }
    print "ok ",$i++,"\n";

    eval "use lib 33.55";
    print "not " if $@;
    print "ok ",$i++,"\n";

    eval "use lib 100.105";
    unless ($@ =~ /lib version 100\.105 required--this is only version 35\.36/) {
	print "not ";
    }
    print "ok ",$i++,"\n";

    local $lib::VERSION = v35.36;
    eval "use lib v33.55";
    print "not " if $@;
    print "ok ",$i++,"\n";

    eval "use lib v100.105";
    unless ($@ =~ /lib v100\.105 required--this is only v35\.36/) {
	print "not ";
    }
    print "ok ",$i++,"\n";

    eval "use lib 33.55";
    print "not " if $@;
    print "ok ",$i++,"\n";

    eval "use lib 100.105";
    unless ($@ =~ /lib version 100\.105 required--this is only version 35\.036/) {
	print "not ";
    }
    print "ok ",$i++,"\n";
}


{
    # Regression test for patch 14937: 
    #   Check that a .pm file with no package or VERSION doesn't core.
    open F, ">xxx.pm" or die "Cannot open xxx.pm: $!\n";
    print F "1;\n";
    close F;
    eval "use lib '.'; use xxx 3;";
    unless ($@ =~ /^xxx defines neither package nor VERSION--version check failed at/) {
	print "not ";
    }
    print "ok ",$i++,"\n";
    unlink 'xxx.pm';
}

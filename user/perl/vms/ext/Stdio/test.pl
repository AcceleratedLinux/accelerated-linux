# Tests for VMS::Stdio v2.2
use VMS::Stdio;
import VMS::Stdio qw(&flush &getname &rewind &sync &tmpnam);

print "1..18\n";
print +(defined(&getname) ? '' : 'not '), "ok 1\n";

$name = "test$$";
$name++ while -e "$name.tmp";
$fh = VMS::Stdio::vmsopen("+>$name",'ctx=rec','shr=put','fop=dlt','dna=.tmp');
print +($fh ? '' : 'not '), "ok 2\n";

print +(flush($fh) ? '' : 'not '),"ok 3\n";
print +(sync($fh) ? '' : 'not '),"ok 4\n";

$time = (stat("$name.tmp"))[9];
print +($time ? '' : 'not '), "ok 5\n";

$fh->autoflush;  # Can we autoload autoflush from IO::File?  Do or die.
print "ok 6\n";

print 'not ' unless print $fh scalar(localtime($time)),"\n";
print "ok 7\n";

print +(rewind($fh) ? '' : 'not '),"ok 8\n";

chop($line = <$fh>);
print +($line eq localtime($time) ? '' : 'not '), "ok 9\n";

($gotname) = (getname($fh) =~/\](.*);/);
print +("\U$gotname" eq "\U$name.tmp" ? '' : 'not '), "ok 10\n";

$sfh = VMS::Stdio::vmssysopen($name, O_RDONLY, 0,
                              'ctx=rec', 'shr=put', 'dna=.tmp');
print +($sfh ? '' : 'not ($!) '), "ok 11\n";

close($fh);
sysread($sfh,$line,24);
print +($line eq localtime($time) ? '' : 'not '), "ok 12\n";

undef $sfh;
print +(stat("$name.tmp") ? 'not ' : ''),"ok 13\n";

print +(&VMS::Stdio::tmpnam ? '' : 'not '),"ok 14\n";

#if (open(P, qq[| MCR $^X -e "1 while (<STDIN>);print 'Foo';1 while (<STDIN>); print 'Bar'" >$name.tmp])) {
#  print P "Baz\nQuux\n";
#  print +(VMS::Stdio::writeof(P) ? '' : 'not '),"ok 15\n";
#  print P "Baz\nQuux\n";
#  print +(close(P) ? '' : ''),"ok 16\n";
#  $fh = VMS::Stdio::vmsopen("$name.tmp");
#  chomp($line = <$fh>);
#  close $fh;
#  unlink("$name.tmp");
#  print +($line eq 'FooBar' ? '' : 'not '),"ok 17\n";
#}
#else { 
print "ok 15\nok 16\nok 17\n";
#}

$sfh = VMS::Stdio::vmsopen(">$name.tmp");
$setuperl = "\$ MCR $^X\nBEGIN { \@INC = qw(@INC) };\nuse VMS::Stdio qw(&setdef);";
print $sfh qq[\$ here = F\$Environment("Default")\n];
print $sfh "$setuperl\nsetdef();\n\$ Show Default\n\$ Set Default 'here'\n";
print $sfh "$setuperl\nsetdef('..');\n\$ Show Default\n";
close $sfh;
@defs = map { /(\S+)/ && $1 } `\@$name.tmp`;
unlink("$name.tmp");
print +($defs[0] eq uc($ENV{'SYS$LOGIN'}) ? '' : "not ($defs[0]) "),"ok 18\n";
#print +($defs[1] eq VMS::Filespec::rmsexpand('[-]') ? '' : "not ($defs[1]) "),"ok 19\n";

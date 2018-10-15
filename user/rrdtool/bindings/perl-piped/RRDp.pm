package RRDp;

=head1 NAME

RRDp - Attach RRDtool from within a perl script via a set of pipes;

=head1 SYNOPSIS

use B<RRDp>

B<RRDp::start> I<path to RRDtool executable>

B<RRDp::cmd>  I<rrdtool commandline>

$answer = B<RRD::read>

$status = B<RRD::end>

B<$RRDp::user>,  B<$RRDp::sys>, B<$RRDp::real>

=head1 DESCRIPTION

With this module you can safely communicate with the RRDtool. 

After every B<RRDp::cmd> you have to issue an B<RRDp::read> command to get
B<RRDtool>s answer to your command. The answer is returned as a pointer,
in order to speed things up. If the last command did not return any
data, B<RRDp::read> will return an undefined variable. 

If you import the PERFORMANCE variables into your namespace, 
you can access RRDtool's internal performance measurements.

=over 8

=item  use B<RRDp>

Load the RRDp::pipe module.

=item B<RRDp::start> I<path to RRDtool executable>

start RRDtool. The argument must be the path to the RRDtool executable

=item B<RRDp::cmd> I<rrdtool commandline>

pass commands on to RRDtool. check the RRDtool documentation for
more info on the RRDtool commands.

=item $answer = B<RRDp::read>

read RRDtool's response to your command. Note that the $answer variable will
only contain a pointer to the returned data. The reason for this is, that
RRDtool can potentially return quite excessive amounts of data
and we don't want to copy this around in memory. So when you want to 
access the contents of $answer you have to use $$answer which dereferences
the variable.

=item $status = B<RRDp::end>

terminates RRDtool and returns RRDtool's status ... 

=item B<$RRDp::user>,  B<$RRDp::sys>, B<$RRDp::real>

these variables will contain totals of the user time, system time and
real time as seen by RRDtool.  User time is the time RRDtool is
running, System time is the time spend in system calls and real time
is the total time RRDtool has been running.

The difference between user + system and real is the time spent
waiting for things like the hard disk and new input from the perl
script.

=back


=head1 EXAMPLE

 use RRDp;
 RRDp::start "/usr/local/bin/rrdtool";
 RRDp::cmd   qw(create demo.rrd --step 100 
               DS:in:GAUGE:100:U:U
	       RRA:AVERAGE:0.5:1:10);
 $answer = RRDp::read;
 print $$answer;
 ($usertime,$systemtime,$realtime) =  ($RRDp::user,$RRDp::sys,$RRDp::real);

=head1 SEE ALSO

For more information on how to use RRDtool, check the manpages.

=head1 AUTHOR

Tobias Oetiker <oetiker@ee.ethz.ch>

=cut
#'  this is to make cperl.el happy

use strict;
use Fcntl;
use Carp;
use IO::Handle;
use IPC::Open2;
use vars qw($Sequence $RRDpid $VERSION);
my $Sequence;
my $RRDpid;

# Prototypes

sub start ($);
sub cmd (@);
sub end ();
sub read ();

$VERSION=1.2010;

sub start ($){
  croak "rrdtool is already running"
    if defined $Sequence;
  $Sequence = 'S';    
  my $rrdtool = shift @_;    
  $RRDpid = open2 \*RRDreadHand,\*RRDwriteHand, $rrdtool,"-" 
    or croak "Can't Start rrdtool: $!";
  RRDwriteHand->autoflush(); #flush after every write    
  fcntl RRDreadHand, F_SETFL,O_NONBLOCK|O_NDELAY; #make readhandle NON BLOCKING
  return $RRDpid;
}


sub read () {
  croak "RRDp::read can only be called after RRDp::cmd" 
    unless $Sequence eq 'C';
  $Sequence = 'R';
  my $inmask = 0;
  my $srbuf;
  my $minibuf;
  my $buffer;
  my $nfound;
  my $timeleft;
  my $ERR = 0;
  vec($inmask,fileno(RRDreadHand),1) = 1; # setup select mask for Reader
  while (1) {
    my $rout;    
    $nfound = select($rout=$inmask,undef,undef,2);
    if ($nfound == 0 ) {
      # here, we could do something sensible ...
      next;
    }
    sysread(RRDreadHand,$srbuf,4096);
    $minibuf .= $srbuf;
    while ($minibuf =~ s|^(.+?)\n||s) {
      my $line = $1;
      # print $line,"\n";
      if ($line =~  m|^ERROR|) {
	croak $line;
	$ERR = 1;
      } 
      elsif ($line =~ m|^OK u:([\d\.]+) s:([\d\.]+) r:([\d\.]+)|){
	($RRDp::sys,$RRDp::user,$RRDp::real)=($1,$2,$3);
	return $ERR == 1 ? undef : \$buffer;
      } else {
	$buffer .= $line. "\n";
      }
    }
  }
}

sub cmd (@){
  croak "RRDp::cmd can only be called after RRDp::read or RRDp::start"
    unless $Sequence eq 'R' or $Sequence eq 'S';
  $Sequence = 'C';
  my $cmd = join " ", @_;
  if ($Sequence ne 'S') {
  }
  $cmd =~ s/\n/ /gs;
  $cmd =~ s/\s/ /gs;
  print RRDwriteHand "$cmd\n";
}

sub end (){
  croak "RRDp::end can only be called after RRDp::start"
    unless $Sequence;
  close RRDwriteHand;
  close RRDreadHand;
  $Sequence = undef;
  waitpid $RRDpid,0;
  return $?
}

1;

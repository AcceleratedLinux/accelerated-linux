#!/usr/bin/perl -w
#
# mail2fax glue logic - read mail from stdin, extract phone number
#                       from e-mail address (12345@fax), call faxspool
# to be run from sendmail, qmail, ...
#
# $Id: mail2fax.pl,v 1.4 2007/04/05 08:45:58 gert Exp $
#
# $Log: mail2fax.pl,v $
# Revision 1.4  2007/04/05 08:45:58  gert
# skip empty text/plain MIME parts
#
# Revision 1.3  2006/10/19 10:29:56  gert
# path change: use /usr/bin/perl
# bugfix: if "-f" isn't specified, don't pass empty "-f" to faxspool
#
# Revision 1.2  2006/09/26 14:29:05  gert
# skip parts that are .p7s or application/x-pkcs7-signature
#
# Revision 1.1  2006/09/22 22:02:18  gert
# initial draft
#
#
use strict;
use Getopt::Std;
use File::Temp qw/ tempfile tempdir /;
use MIME::Parser;

# configuration section
my $logfile = "/var/log/mgetty/mail2fax.log";
my $workdir = "/var/spool/fax/tmp";
my $faxspool = "/usr/local/bin/faxspool";

# get arguments
#  calling convention from sendmail is: A=mail2fax -h $h -f $f $u

use vars qw /$opt_f $opt_h $opt_d $opt_v/;
$opt_h = '';		# fax relay host (currently unused)
$opt_f = '';		# sender's e-mail address (envelope-from)
$opt_d = 0;		# debug
$opt_v = 0;		# verbose output

unless( getopts('f:h:dv') )
{
    print STDERR "ERROR: invalid option\n";
    exit 64;		# EX_USAGE
}
if ( $#ARGV != 0 )	# exactly one argument needed
{
    print STDERR "ERROR: call syntax: exactly one argument (faxnr) needed\n";
    exit 64;		# EX_USAGE
}

my $fax_to = shift @ARGV;
$fax_to =~ s/@.*$//;

if ( $fax_to !~ /^\d+$/ )
{
    print STDERR "ERROR: invalid character found in fax number: only digits 0-9 allowed\n";
    exit 64;		# EX_USAGE
}

unless( open LOG, ">>$logfile" )
{
    print STDERR "ERROR: can't open logfile '$logfile': $!\n";
    exit 73;		# EX_CANTCREAT
}

unless( chdir($workdir) && -w '.' )
{
    print STDERR "ERROR: workdir '$workdir' does not exist or is not writeable\n";
    exit 73;
}
&logmsg( "--- from=$opt_f, to=$fax_to" );

# create temporary directory for input files, below workdir
my $dir = tempdir( DIR => '.', CLEANUP => 1 );

unless( defined $dir && -d $dir && -w $dir )
	{ print STDERR "ERROR: can't create tempdir: $!\n"; exit 73; }

&logmsg( "dir=$dir" );

# now parse mail on stdin...
my $parser = new MIME::Parser;
$parser->output_dir($dir);

my $entity = $parser->parse(\*STDIN);

# flatten 0- or 1-part multipart into singlepart structure
$entity->make_singlepart;

# dump structure to log file
$entity->dump_skeleton(\*LOG);

# we could/should do authentication here (sender / from: / ... vs. fax number)
# **TODO**!

# generate command line for faxspool
my @pages = ();

my @parts = $entity->parts_DFS;
foreach my $part (@parts)
{
my $type = $part->effective_type;

    print LOG "part: effective_type=$type\n";
    print LOG "part: is_multipart=TRUE\n" if( $part->is_multipart);
    if ( defined( $part->bodyhandle ) )
    {
	my $path = $part->bodyhandle->path;
        print LOG "part: pathname=$path\n";

	# everything that has a file name (- is not metadata) and doesn't
        # match an exclude list of unconvertable content is passed to faxspool

	if ( $type eq 'text/html' || $path =~ /\.html?$/ ||
	     $type eq 'application/x-pkcs7-signature' || $path =~ /\.p7s/
	   )
	{
	    print LOG "-> skip part, unconvertable body\n";
	    next;
	}

	# some e-mail clients produce empty text/plain parts when sending
	# "just attachment" mails (e.g. sending a PDF) -> skip these
	if ( $type eq 'text/plain' && -z $path )
	{
	    print LOG "-> skip part, empty file $path\n";
	    next;
	}
    
	push @pages, $path;
    }
}

# we're called from sendmail as uid=root, euid=fax -> make this uid=fax
$<=$>;

my $cmd="$faxspool";

$cmd .= " -f $opt_f" if ( $opt_f ne '' );
$cmd .= " $fax_to \"". join('" "', @pages) . "\" 2>&1";

&logmsg( "$cmd" );

my $out=`$cmd`;
my $rc = $?;
print LOG $out;
&logmsg( "faxspool return code=$rc" );

close LOG;

if ( $rc == 0 ) { exit 0; } 
           else { print STDERR $out; exit 65; } # EX_DATAERR

#
# -------------------------------------------------------------------------
#
sub logmsg
{
    print LOG localtime() .' '. join( ' ', @_ ) ."\n";
}

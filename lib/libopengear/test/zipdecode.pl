#!/usr/bin/env perl
# A debugging script to examine the binary content of a ZIP file

open(STDIN, "<".(shift @ARGV)) if @ARGV;
$offset = 0;
while (!eof STDIN) {
	$sig = &pri(4, undef);
	if ($sig == 0x04034b50) {
		print "[local file header]\n";
		&local_file_header;
	} elsif ($sig == 0x08074b50) {
		print "[data descriptor header]\n";
		&data_descriptor_header;
	} elsif ($sig == 0x02014b50) {
		print "[central directory]\n";
		&central_directory_structure;
	} elsif ($sig == 0x06054b50) {
		print "[end of central directory]\n";
		&end_of_central_directory;
	} else {
		print "unknown\n";
	}
}

# base function for pretty-printing hex data
sub pr_base ($) {
	my ($nbytes) = @_;

	printf "%08x: ", $offset;
	my $i;
	my $c;
	my @data = ();
	for ($i = 0; $i < $nbytes; $i++) {
		if ($i and ($i % 8 == 0)) { printf "\n%08x: ", $offset; }
		read STDIN, $c, 1 or die;
		$offset++;
		$c = unpack("C", $c);
		printf "%02x ", $c;
		push(@data, $c);
	}
	for (; $i < 8; $i++) {
		print "   ";
	}
	return @data;
}

# print with just a label
sub prx ($$) { my ($nbytes, $label) = @_;
	my @data = &pr_base($nbytes);
	print "$label\n" if defined $label;
	return @data;
}

# print a labeled integer
sub pri ($$) { my ($nbytes, $label) = @_;
	my @data = &pr_base($nbytes);
	my $n = 0;
	my $shift = 0;
	for (@data) { $n = $n | ($_ << $shift); $shift += 8; }
	if (defined $label) {
		print $label;
		if ($n != 0) { printf "  0x%x", $n; }
		print "\n";
	}
	return $n;
}

# print a labeled string
sub prs ($$) { my ($nbytes, $label) = @_;
	my @data = &pr_base($nbytes);
	my $s = "";
	for (@data) {
		if ($_ >= 20 and $_ < 0x7f)     { $s .= chr; }
		elsif ($_ == 92 or $_ == 34)    { $s .= "\\" . chr; }
		elsif ($_ == 0)                 { $s .= "\\0"; }
		else                            { $s .= sprintf("\\x%02x", $_);}
	}
	if (defined $label) {
		print $label;
		if (@data) { print "  \"$s\""; }
		print "\n";
	}
	return $s;
}


sub local_file_header { 
	# 50 4b 03 04
	&pri(2, "file version");
	&pri(2, "general purpose flags");
	&pri(2, "compression method");
	&pri(4, "modification date/time");
	&pri(4, "crc32");
	&pri(4, "compressed size");
	&pri(4, "uncompressed size");
	my $fnlen = &pri(2, "filename length");
	my $extlen = &pri(2, "extra length");
	&prs($fnlen, "filename");
	&prx($extlen, "extra");
}

sub data_descriptor_header {
	# 50 4b 07 08
	&pri(4, "crc32");
	&pri(4, "compressed_size");
	&pri(4, "uncompressed_size");
}

sub central_directory_structure {
	# 50 4b 01 02
	&pri(2, "version made by");
	&pri(2, "version needed");
	&pri(2, "general purpose flags");
	&pri(2, "compression method");
	&pri(4, "modification time");
	&pri(4, "crc32");
	&pri(4, "compressed_size");
	&pri(4, "uncompressed_size");
	my $len = &pri(2, "filename length");
	my $extlen = &pri(2, "extra length");
	my $comlen = &pri(2, "comment length");
	&pri(2, "disk number start");
	&pri(2, "internal file attributes");
	&pri(4, "external file attributes");
	&pri(4, "local header offset");

	&prs($fnlen, "file name");
	&prx($extlen, "extra");
	&prs($comlen, "comment");
}

sub end_of_central_directory {
	&pri(2, "number of this disk");
	&pri(2, "number of first disk");
	&pri(2, "#entries this disk");
	&pri(2, "total #entries all disks");
	&pri(4, "cde size");
	&pri(4, "cde offset on disk 0");
	my $comlen = &pri(2, "comment length");
	&prs($comlen, "comment");
}

#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

use warnings;
use Text::ParseWords;

print "1..22\n";

@words = shellwords(qq(foo "bar quiz" zoo));
print "not " if $words[0] ne 'foo';
print "ok 1\n";
print "not " if $words[1] ne 'bar quiz';
print "ok 2\n";
print "not " if $words[2] ne 'zoo';
print "ok 3\n";

{
  # Gonna get some undefined things back
  no warnings 'uninitialized' ;

  # Test quotewords() with other parameters and null last field
  @words = quotewords(':+', 1, 'foo:::"bar:foo":zoo zoo:');
  print "not " unless join(";", @words) eq qq(foo;"bar:foo";zoo zoo;);
  print "ok 4\n";
}

# Test $keep eq 'delimiters' and last field zero
@words = quotewords('\s+', 'delimiters', '4 3 2 1 0');
print "not " unless join(";", @words) eq qq(4; ;3; ;2; ;1; ;0);
print "ok 5\n";

# Big ol' nasty test (thanks, Joerk!)
$string = 'aaaa"bbbbb" cc\\ cc \\\\\\"dddd" eee\\\\\\"ffff" "gg"';

# First with $keep == 1
$result = join('|', parse_line('\s+', 1, $string));
print "not " unless $result eq 'aaaa"bbbbb"|cc\\ cc|\\\\\\"dddd" eee\\\\\\"ffff"|"gg"';
print "ok 6\n";

# Now, $keep == 0
$result = join('|', parse_line('\s+', 0, $string));
print "not " unless $result eq 'aaaabbbbb|cc cc|\\"dddd eee\\"ffff|gg';
print "ok 7\n";

# Now test single quote behavior
$string = 'aaaa"bbbbb" cc\\ cc \\\\\\"dddd\' eee\\\\\\"ffff\' gg';
$result = join('|', parse_line('\s+', 0, $string));
print "not " unless $result eq 'aaaabbbbb|cc cc|\\"dddd eee\\\\\\"ffff|gg';
print "ok 8\n";

# Make sure @nested_quotewords does the right thing
@lists = nested_quotewords('\s+', 0, 'a b c', '1 2 3', 'x y z');
print "not " unless (@lists == 3 && @{$lists[0]} == 3 && @{$lists[1]} == 3 && @{$lists[2]} == 3);
print "ok 9\n";

# Now test error return
$string = 'foo bar baz"bach blech boop';

@words = shellwords($string);
print "not " if (@words);
print "ok 10\n";

@words = parse_line('s+', 0, $string);
print "not " if (@words);
print "ok 11\n";

@words = quotewords('s+', 0, $string);
print "not " if (@words);
print "ok 12\n";

{
  # Gonna get some more undefined things back
  no warnings 'uninitialized' ;

  @words = nested_quotewords('s+', 0, $string);
  print "not " if (@words);
  print "ok 13\n";

  # Now test empty fields
  $result = join('|', parse_line(':', 0, 'foo::0:"":::'));
  print "not " unless ($result eq 'foo||0||||');
  print "ok 14\n";

  # Test for 0 in quotes without $keep
  $result = join('|', parse_line(':', 0, ':"0":'));
  print "not " unless ($result eq '|0|');
  print "ok 15\n";

  # Test for \001 in quoted string
  $result = join('|', parse_line(':', 0, ':"' . "\001" . '":'));
  print "not " unless ($result eq "|\1|");
  print "ok 16\n";

}

# Now test perlish single quote behavior
$Text::ParseWords::PERL_SINGLE_QUOTE = 1;
$string = 'aaaa"bbbbb" cc\ cc \\\\\"dddd\' eee\\\\\"\\\'ffff\' gg';
$result = join('|', parse_line('\s+', 0, $string));
print "not " unless $result eq 'aaaabbbbb|cc cc|\"dddd eee\\\\"\'ffff|gg';
print "ok 17\n";

# test whitespace in the delimiters
@words = quotewords(' ', 1, '4 3 2 1 0');
print "not " unless join(";", @words) eq qq(4;3;2;1;0);
print "ok 18\n";

# [perl #30442] Text::ParseWords does not handle backslashed newline inside quoted text
$string = qq{"field1"	"field2\\\nstill field2"	"field3"};

$result = join('|', parse_line("\t", 1, $string));
print "not " unless $result eq qq{"field1"|"field2\\\nstill field2"|"field3"};
print "ok 19\n";

$result = join('|', parse_line("\t", 0, $string));
print "not " unless $result eq "field1|field2\nstill field2|field3";
print "ok 20\n";

# unicode
$string = qq{"field1"\x{1234}"field2\\\x{1234}still field2"\x{1234}"field3"};
$result = join('|', parse_line("\x{1234}", 0, $string));
print "not " unless $result eq "field1|field2\x{1234}still field2|field3";
print "ok 21\n";

# missing quote after matching regex used to hang after change #22997
"1234" =~ /(1)(2)(3)(4)/;
$string = qq{"missing quote};
$result = join('|', shellwords($string));
print "not " unless $result eq "";
print "ok 22\n";

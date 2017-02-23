######################################
# dsk.pl
#
# reads in apple disk sectors but reverses each group of 16
#
#also allows for specific orderings
#

use strict;
use warnings;

my $reverseRead = 0;
my $x;
my $x1;
my $y;
my $z;
my $buffer;

my $count = 0;
my $toRead = "";

my @order = (0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0);

while ($count <= $#ARGV )
{
  $a = lc($ARGV [$count]);
  for ($a)
  {
   /^-?1f$/ && do { @order = (0x1, 0xf, 0x0, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2); $count++; next; };
   /^-?r/i && do { $reverseRead = 1; $count++; next; };
   /^(\?|-)/ && do { print "INVALID ARGUMENT $a\n"; usage(); exit; };
   if ($toRead) { die("2 files read, $toRead/$a."); }
   $toRead = $a; $count++;
  }
}

if (!$toRead) { die ("Need a file name."); }

open(A, $toRead) || die ("No $toRead");
open(B, ">$ARGV[0].x");

binmode(A);
binmode(B);

for $x (0..0x22)
{
  $x1 = $x;
  if ($reverseRead) { $x1 = 0x22 - $x; }
  for $y (0..0xf)
  {
    seek(A, 256*$order[$y] + 4096*$x, 0);
    $z = read(A, $buffer, 256);
    #print "$x $y " . length($buffer) . "\n";
    print B $buffer;
  }
}

close(A);
close(B);

###############################################
sub usage
{
print <<EOT;
USAGE========================
-r revese the order of each 0x10 block of 0x100 sectors
-1f specifies 1 f 0 e then a descent
EOT
}
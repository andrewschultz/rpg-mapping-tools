################################
#
# dsk2bmp
#
# converts a DSK image to 14x16 icons
#

use strict;
use warnings;

my $x;

if (!defined($ARGV[0])) { die ("Need a file to open."); }

open(A, "$ARGV[0]") || die ("No such file $ARGV[0].");

if (-s "$ARGV[0]" != 0x23000) { die("Wrong size for file, should be 143360."); }

binmode(A);

for (0..0x22)
{
  writeBmp($_);
}

close(A);

###################################
# the main function
sub writeBmp
{
  my $i1;
  my $i2;
  my $j1;
  my $j2;

  my $buffer;

  my $thisByte;
  my $byte;

  my $outFile = sprintf ("bmpout-%03x.bmp", $_[0]);
  open(C, "c:\\coding\\games\\256.bmp");

  read(C, $buffer, 0x436, 0);
  binmode(C);

  open(B, ">$outFile");
  binmode(B);

  my @q = split(//, $buffer);
  for ($x = 0; $x < 0x436; $x++)
  {
    if ($x == 0x12) { print B chr(0xe0); }
    elsif ($x == 0x13) { print B chr(0); }
    elsif ($x == 0x16) { print B chr(0x80); }
    elsif ($x == 0x17) { print B chr(0); }
	elsif (($x <= 0x3c) && ($x >= 0x3a)) { print B chr(0xff); }
    else { print B $q[$x]; }
  }

  seek(A, $_[0] * 0x1000, 0);
  read(A, $buffer, 0x1000, 0);

  @q = split(//, $buffer);
  for ($j2 = 0; $j2 < 8; $j2++)
  {
    for ($j1 = 0; $j1 < 16; $j1++)
	{
      for ($i2 = 0; $i2 < 16; $i2++)
      {
        for ($i1 = 0; $i1 < 2; $i1++)
        {
		  $thisByte = $i1 + 32 * $i2 + 2 * $j1 + 512 * $j2;
          $byte = ord($q[$thisByte]);
		  if (($_[0] == 0xa) && ($j1 == 0) && ($i2 < 4)) { printf("Processing byte %x, %02x.\n", $thisByte, $byte); }
          for ($x = 6; $x >= 0; $x--)
          { print B chr(($byte>>$x) & 1); }
        }
      }
    }
  }
  close(B);
  close(C);
}
#dh.pl
#doubles or halves a BMP based on what you want
#usage dh.pl X.bmp to double
#dh.pl /X.bmp to halve

$double = 1;

if (!@ARGV[0]) { print "You need to put in a file to read. / at the start means cut in half.\n"; }

$myFile = @ARGV[0];

if ($myFile =~ /^\//)
{
  $double = 0;
  $myFile =~ s/^\///g;
}

open(A, "$myFile") || die ("Can't open file $myFile.");

$outfile = $myFile;

if ($double) { $outfile =~ s/([^\\\/]*)$/dbl-$1/; }
else { $outfile =~ s/([^\\\/]*)$/hal-$1/; }

open(B, ">$outfile") || die ("Can't open outfile $outfile.");

binmode(A);
binmode(B);

$firstHdr = read(A, $buffer, 0x12);

print B $buffer;

$wid = readlong();

if ($double) { $outWid = $wid * 2; }
else { $outWid = $wid / 2; }

putlong($outWid);

$hei = readlong();

if ($double) { $outHei = $hei * 2; }
else { $outHei = $hei / 2; }

putlong($outWid);

#account for if not multiple of 4. Not likely, but who knows.
$wbuf = $wid - 4 * ($wid / 4); if ($wbuf) { $wbuf = 4 - $wbuf; }
$owbuf = $outWid - 4 * ($outWid / 4); if ($owbuf) { $owbuf = 4 - $wbuf; }

$nextHdr = read (A, $buffer, 0x41c);
print B $buffer;

for $h (0..$hei-1)
{
  
if ($double)
{
  $tb = "";
  for (0..$wid-1)
  {
    $try = read(A, $buffer, 1);
    print B ($buffer);
    print B ($buffer);
    $tb .= "$buffer$buffer"; # we could seek back but that's a pain
  }
  if ($owbuf) { for (1..$owbuf) { print B 0; } }
  #need to double print rows as well as buffers.
  print B $tb;
  if ($owbuf) { for (1..$owbuf) { print B 0; } }
} else
{
  for (0..$wid-1)
  {
    $try = read(A, $buffer, 1); # since pixels are identical, junk the first
    $try = read(A, $buffer, 1);
    if ($h % 2)
    {
    print B ($buffer);
    }
  }
  if (($owbuf) && ($h % 2)) { for (1..$owbuf) { print B 0; } }
  $h++;
}
}

close(A);
close(B);

sub readlong
{
  my $temp = 0;
  for (0..3)
  {
    $try = read(A, $buffer, 1);
    print "$try Byte " . ord($buffer) . "\n";
    $temp += ord($buffer) * (256**$_);
  }
  return $temp;
}

sub putlong
{
  print "Putting long $_[0]\n";
  my $temp = 0;
  for (0..3)
  {
    print B chr(0xff & ($_[0] >> ($_ * 8)));
  }
}

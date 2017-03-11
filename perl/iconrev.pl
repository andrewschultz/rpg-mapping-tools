####################################
#
#iconrev.pl
#
#reverse engineers the icons we find in any one generated map
#note this is not perfect, as for instance we will have town names that give false "new" icons
#
#examples
#
#iconrev.pl -s 6 -ct 20 -f bikkuriman_world.bmp > iconrev.txt
#iconrev.pl -s 8 -cr 128 -f ultima_3_sosaria.bmp -b u3.bmp,132,132,196,196 > iconrev.txt
#
#icons are 6x6, cut 20 off the top from my bikkurian world map
#
# features to add include threshold icon can appear to be flagged as important
# as well as actual icons from a real world map
# also where an icon first occurs etc.
#

use Image::Info qw(image_info dim);
use Image::Size;

#use List::MoreUtils qw(uniq);
use List::Util qw[min max];

use strict;
use warnings;

######################options
my $trackOccur = 1; #tells where the icons first/last pop up. You can disable if you want

######################these really should be set. If not, I slap the user on the wrist.
my $iconHeight = 0;
my $iconWidth = 0;
my $threshold = 2;

######################these don't need to but may come in handy
my $bottomEdge = 0;
my $topEdge = 0;
my $leftEdge = 0;
my $rightEdge = 0;
my $fileName = "";
my @excludeArray = ();
my @binAry = ();

my $iconGuess = "";

my $toRead = 0;

my $count = 0;

my %lastOccur;
my %firstOccur;

my %tempcolor;
my $mytempcolor;

while ($count <= $#ARGV)
{
  my $this = $ARGV[$count];
  my $that = "";
  if (defined($ARGV[$count+1])) { $that = $ARGV[$count+1]; }

  for ($this)
  {
  /^-b$/ && do { @binAry = split(/,/, $that); verifyBinArray(); $count += 2; next; };
  /^-h$/ && do { $iconHeight = $that; $count += 2; next; };
  /^-w$/ && do { $iconWidth = $that; $count += 2; next; };
  /^-(hw|wh|s)$/ && do { $iconWidth = $iconHeight = $that; $count += 2; next; };
  /^-cl$/ && do { $leftEdge = $that; $count += 2; next; };
  /^-cr$/ && do { $rightEdge = $that; $count += 2; next; };
  /^-ct$/ && do { $topEdge = $that; $count += 2; next; };
  /^-cb$/ && do { $bottomEdge = $that; $count += 2; next; };
  /^-f$/ && do { $fileName = $that; $count += 2; next; };
  /^-f$/ && do { $iconGuess = $that; $count += 2; next; };
  /^-nt$/ && do { $trackOccur = 0; $count++; next; };
  /^-t$/ && do { $threshold = $that; $count += 2; next; };
  /^-x$/ && do { @excludeArray = split(/,/, $that); $count += 2; next; };
  /^-\?$/ && do { usage(); };
  print "Invalid parameter $this\n\n";
  usage();
  }
}

if (!$iconWidth) { print "WARNING you should've set icon width to nonzero, defaulting to 10.\n"; $iconWidth = 10; }
if (!$iconHeight) { print "WARNING you should've set icon height to nonzero, defaulting to 10.\n"; $iconHeight = 10; }

my ($imgWidth, $imgHeight) = imgsize($fileName);
my $type = Image::Info::image_type($fileName);
if ((!defined($type->{file_type})) || ($type->{file_type} ne "BMP"))
{ die ("Need a BMP file. Have " . (defined($type->{file_type}) ? $type->{file_type} : "nothing") ."."); }
print "W $imgWidth H $imgHeight type $type->{file_type}\n";

my $virtH = $imgHeight - $topEdge - $bottomEdge;
my $virtW = $imgWidth - $leftEdge - $rightEdge;
if ($virtW % $iconWidth) { die ("Image width $imgWidth isn't divisible by icon width $iconWidth."); }
if ($virtH % $iconHeight) { die ("Image height $imgHeight isn't divisible by icon height $iconHeight."); }

showLikelyIcons();

###############################
# subroutines

sub showLikelyIcons
{

   my %iconHash;
   my $i1; my $i2; my $j1; my $j2;
   my @iconList;
   my @byteBuf;
   my $buffer;
   my $thisByte;
   my %gotByte;
   my %palette;
   my $col;

   my $badPattern = 0;
   my $belowthreshold = 0;

   my $ih;
   my $temp;
   my $binWidth;
   my $binHeight;

   #side binary file vars
   my $doBin = defined($binAry[0]);
   my %binProb;
   my $binOffset;
   my @binVals;

  open(A, "$fileName") || die ("Can't open $fileName!");
  binmode(A);

  seek(A, 0x436, 0);
  seek(A, $bottomEdge * $imgWidth + $leftEdge, 1);

  my $iconsHigh = ($imgHeight-$topEdge-$bottomEdge) / $iconHeight;
  my $iconsWide = ($imgWidth-$leftEdge-$rightEdge) / $iconWidth;

  print "$iconsWide by $iconsHigh read-in.\n";

  if ($doBin)
  {
    open(B, $binAry[0]) || die ("Couldn't open $binAry[0]");
	binmode(B);
	($binWidth, $binHeight) = imgsize($binAry[0]);
	my $binCutWidth = $binAry[3] - $binAry[1];
	my $binCutHeight = $binAry[4] - $binAry[2];
	if ($binCutWidth != $iconsWide) { die ("width of bin file/icons doesn't match: $binCutWidth($binAry[0]) vs $iconsWide($fileName)"); }
	if ($binCutHeight != $iconsHigh) { die ("height of bin file/icons doesn't match: $binCutHeight($binAry[0]) vs $iconsHigh($fileName)"); }
  }
  for ($j2=0; $j2 < $iconsHigh; $j2++)
  {
    if ($doBin)
	{
	  $binOffset = $binAry[1] + $binWidth * ($binHeight - ($binAry[2] + $iconsHigh - $j2));
	  seek(B, 0x436+$binOffset, 0);
	  read(B, $buffer, $binAry[3] - $binAry[1]);
	   @binVals = split(//, $buffer);
	}
    @iconList = ();
    for ($j1=0; $j1 < $iconHeight; $j1++)
	{
	  read(A, $buffer, $imgWidth);
	  @byteBuf = split(//, $buffer);
      for ($i2=0; $i2 < $iconsWide; $i2++)
      {
        for ($i1=0; $i1 < $iconHeight; $i1++)
        {
		  $thisByte = $byteBuf[$i2*$iconWidth+$i1];
		  $gotByte{ord($thisByte)} = 1;
		  $iconList[$i2] .= "-" . ord($thisByte);
        }
      }
	  #for (0..$#iconList) { print "$_ $iconList[$_]\n"; }
    }

	if ($doBin)
	{
	#print join(",", map {ord($_)} @binVals) . "\n";
	for (0..min($#iconList, $#binVals))
	{
	  if (!$binProb{ord($binVals[$_])})
	  {
	    #printf("Assigning icon value %d to the icon at %d, %d.\n", ord($binVals[$_]), $_, $iconsHigh - $j2 - 1);
	    $binProb{ord($binVals[$_])} = $iconList[$_];
	  }
	}
	}
	if ($trackOccur)
	{
    for (reverse(0..$#iconList))
	{
	  $temp = sprintf("(%d,%d)", $_, $iconsHigh - $j2 - 1);
	  $iconHash{$iconList[$_]}++;
	  if (!$lastOccur{$iconList[$_]}) { $lastOccur{$iconList[$_]} = $temp; }
	  $firstOccur{$iconList[$_]} = $temp;
    }
	}
  }

 $mytempcolor = 0;

 if ($doBin)
 {
   my $idx;
   if (!$iconGuess)
   {
     if (defined($binAry[5]))
	 {
	 $iconGuess = $binAry[5];
	 }
	 else
	 {
     $iconGuess = $fileName;
	 $iconGuess =~ s/(\.bmp)?$/-icons.txt/;
	 }
   }
   open(C, ">$iconGuess");
   for $idx(sort {$a <=> $b} keys %binProb)
   {
     print C sprintf("0x%02x\n%s\n", $idx, vflip($binProb{$idx}));
   }
   close(C);
 }

for my $k (sort { $a <=> $b } keys %gotByte)
{
  $tempcolor{$k} = $mytempcolor++;
  seek(A, 0x36 + 4 * $k, 0);
  read(A, $buffer, 4);
  @byteBuf = split(//, $buffer);
  $temp = 0;
  for (0..2) { $temp += ord($byteBuf[$_])  * (256 ** $_); }
  $palette{$k} = sprintf("%06x", $temp);
  print "$k => $tempcolor{$k}, $palette{$k}\n";
}

print "Bytes used:";
for (sort {$a <=> $b} keys %gotByte) { print " $_=$palette{$_}"; }
print "\n";

print "Results of which icons go where:\n";

my $foundOne;

#for (sort keys %binProb) { print "$_: $binProb{$_}\n"; } die;

OUTER:
for $ih (sort { $iconHash{$b} <=> $iconHash{$a} } keys %iconHash)
{
  for $col (@excludeArray)
  {
  if ($iconHash{$ih} =~ /\b$col\b/) { $badPattern++; next OUTER; }
  }
  if ($iconHash{$ih} >= $threshold)
  {
    if ($trackOccur)
	{
	  print "#first occurrence of icon below at $firstOccur{$ih}, last at $lastOccur{$ih}\n";
	}
    print "#icon with $iconHash{$ih} hits\n0x";
	$foundOne = 0;
	for (sort keys %binProb)
	{
	  if ($binProb{$_} eq $ih)
	  {
	  if ($foundOne) { print "/"; }
	  printf("%02x", $_);
	  $foundOne = 1;
	  }
	}
	if (!$foundOne) { print "??"; }
	print "\n";
	print vflip($ih);
  }
  elsif ($iconHash{$ih} > 0) { $belowthreshold++; }
}

if ($badPattern) { print "#$badPattern icons with bad patterns were ignored.\n"; }
if ($belowthreshold) { print "#$belowthreshold icons occurred below $threshold times and were ignored.\n"; }
}

sub vflip
{
  my $x;
  my $y;
  my $temp = $_[0];
  my $retVal = "";

  $temp =~ s/^-//;
  my @a1 = split(/-/, $temp);
  @a1 = map {defined($tempcolor{$_}) ? $tempcolor{$_} : $_ } @a1;
  my @a2 = ();
  for $y (reverse (0..$iconHeight-1))
  {
    #print @a1[$y*$iconWidth..$y*$iconWidth+$iconWidth-1];
	#print "\n";
    $retVal .= join("", @a1[$y*$iconWidth..$y*$iconWidth+$iconWidth-1]) . "\n";
	#print "$y: Ret now $retVal!\n";
  }
  return $retVal;
}

sub verifyBinArray
{
  if ($#binAry != 4 && $#binAry != 5) { die ("Binary array needs 5-6 arguments: file, Xi, Yi, Xf, Yf, optional out file."); }
  if (! -f $binAry[0]) { die ("No file $binAry[0]."); }
  if ($binAry[1] > $binAry[3]) { die("Xi needs to be less than Xf."); }
  if ($binAry[2] > $binAry[4]) { die("Yi needs to be less than Yf."); }
  my $type = Image::Info::image_type($binAry[0]);
  if ((!defined($type->{file_type})) || ($type->{file_type} ne "BMP"))
  { die ("$binAry[0] must be a BMP file. Have " . (defined($type->{file_type}) ? $type->{file_type} : "nothing") ."."); }
}

sub usage
{
print<<EOT;
-h/-w set icon height-width, -s sets both
-c(lbrt) sets cutoff for what to read, in pixels
-f specifies file
-nt specifies no tracking of first/last icons
-ig specifies icon guess file (can also be in -b CSV)
-b specifies binary file to crib from to guess icons, then Xi Yi Xf Yf
-? gives this
EOT
exit()
}
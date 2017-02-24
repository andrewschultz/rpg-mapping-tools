####################################
#
#iconrev.pl
#
#reverse engineers the icons we find in any one generated map
#note this is not perfect, as for instance we will have town names that give false "new" icons
#
#example
#
#iconrev.pl -s 6 -ct 20 -f bikkuriman_world.bmp > iconrev.txt
#
#icons are 6x6, cut 20 off the top from my bikkurian world map
#
# features to add include threshhold icon can appear to be flagged as important
# as well as actual icons from a real world map
# also where an icon first occurs etc.
#

use Image::Info qw(image_info dim);
use Image::Size;

use strict;
use warnings;

######################these really should be set. If not, I slap the user on the wrist.
my $iconHeight = 0;
my $iconWidth = 0;

######################these don't need to but may come in handy
my $bottomEdge = 0;
my $topEdge = 0;
my $leftEdge = 0;
my $rightEdge = 0;
my $fileName = "";

my $toRead = 0;

my $count = 0;

my %tempcolor;
my $mytempcolor;

while ($count <= $#ARGV)
{
  my $this = $ARGV[$count];
  my $that = "";
  if (defined($ARGV[$count+1])) { $that = $ARGV[$count+1]; }

  for ($this)
  {
  /^-h$/ && do { $iconHeight = $that; $count += 2; next; };
  /^-w$/ && do { $iconWidth = $that; $count += 2; next; };
  /^-(hw|wh|s)$/ && do { $iconWidth = $iconHeight = $that; $count += 2; next; };
  /^-cl$/ && do { $leftEdge = $that; $count += 2; next; };
  /^-cb$/ && do { $rightEdge = $that; $count += 2; next; };
  /^-ct$/ && do { $topEdge = $that; $count += 2; next; };
  /^-cr$/ && do { $bottomEdge = $that; $count += 2; next; };
  /^-f$/ && do { $fileName = $that; $count += 2; next; };
  /^-\?$/ && do { usage(); };
  print "Invalid parameter $this\n\n";
  usage();
  }
}

if (!$iconWidth) { print "WARNING you should've set icon width to nonzero, defaulting to 10.\n"; $iconWidth = 10; }
if (!$iconHeight) { print "WARNING you should've set icon height to nonzero, defaulting to 10.\n"; $iconHeight = 10; }

my ($imgWidth, $imgHeight) = imgsize($fileName);
my $type = Image::Info::image_type($fileName);
if ($type->{file_type} ne "BMP") { die ("Need a BMP file. Have $type->{file_type}."); }
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

   my $ih;
   my $temp;

  open(A, "$fileName") || die ("Can't open $fileName!");
  binmode(A);

  seek(A, 0x436, 0);
  seek(A, $bottomEdge * $imgWidth, 1);

  for ($j2=0; $j2 < ($imgHeight-$topEdge-$bottomEdge) / $iconHeight; $j2++)
  {
    @iconList = ();
    for ($j1=0; $j1 < $iconHeight; $j1++)
	{
	  read(A, $buffer, $imgWidth);
	  @byteBuf = split(//, $buffer);
      for ($i2=0; $i2 < $imgWidth / $iconWidth; $i2++)
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
    for (@iconList) { $iconHash{$_}++; }
  }

 $mytempcolor = 0;

for my $k (sort { $a <=> $b } keys %gotByte)
{
  $tempcolor{$k} = $mytempcolor++;
  seek(A, 0x36 + 4 * $k, 0);
  read(A, $buffer, 4);
  @byteBuf = split(//, $buffer);
  $temp = 0;
  for (0..2) { $temp *= 256; $temp += ord($byteBuf[$_]); }
  $palette{$k} = sprintf("%06x", $temp);
  print "$k => $tempcolor{$k}, $palette{$k}\n";
}

print "Bytes used:";
for (sort {$a <=> $b} keys %gotByte) { print " $_=$palette{$_}"; }
print "\n";

print "Results of which icons go where:\n";
for $ih (sort { $iconHash{$b} <=> $iconHash{$a} } keys %iconHash)
{
  if ($iconHash{$ih} > 1) { print "#icon with $iconHash{$ih} hits\n0x??\n"; print vflip($ih); }
}

}

sub vflip
{
  my $x;
  my $y;
  my $temp = $_[0];
  $temp =~ s/^-//;
  my @a1 = split(/-/, $temp);
  @a1 = map {$tempcolor{$_}} @a1;
  my @a2 = ();
  for $y (reverse (0..$iconHeight-1))
  {
    print @a1[$y*$iconWidth..$y*$iconWidth+$iconWidth-1];
	print "\n";
  }
}

sub usage
{
print<<EOT;
-h/-w set icon height-width, -s sets both
-c(lbrt) sets cutoff for what to read, in pixels
-f specifies file
-? gives this
EOT
exit()
}
################################
# x2b.pl
#
# converts XML for world.xml for Outlaw Editor to a BMP
#

use strict;
use warnings;

my $dir = ".";
my $tile;
my $line;
my $count = 0;
my $chunks;
my $mapName;
my $launch = 0;

my $readFile = "";;
my $xmlFile;
my %iconHash;
my $iconIdx = 0;
my $totalIcon = 0;

my $xi=0;
my $yi=0;
my $xc=0;
my $yc=0;
my $yl=0;
my $xl=0;
my $debug = 0;

my @mapAry;
my $outFile = "al-raw.bmp";

if (defined($ARGV[0]))
{
  if (($ARGV[0] eq "?") || ($ARGV[0] eq "?")) { usage(); }
  if (-f "$ARGV[0]") { $readFile = "$ARGV[0]"; }
  elsif (-d "$ARGV[0]") { $readFile = "$ARGV[0]\\al.txt"; }
}
else { die ("Need a vaild directory for world.xml, or a file.");  }

initArray(0);

open(A, "$readFile") || die ("Can't open read-file $readFile. If you want options, or how to write an extractor file, push -?");
while ($line = <A>)
{
  if ($line =~ /^;/) { last; }
  if ($line =~ /^#/) { next; }
  chomp($line);
  if ($line =~ /^init=/)
  {
    $line =~ s/^init=//;
	$line = hex($line);
	initArray($line);
  }
  if (!$xmlFile)
  {
    $xmlFile = $line;
    if (! -f $xmlFile) { die("$xmlFile is not a file on line $."); }
	print "XML file is $xmlFile\n";
	next;
  }
	if ($line eq "locs")
	{
	  print "xc $xc xi $xi yc $yc yi $yi\n";
	  next;
	}
  if ($line =~ /^xi=?/) { $line =~ s/.*=//; $xi = $line; $xc = 0; print "xi now $xi\n"; next; }
  if ($line =~ /^yi=?/) { $line =~ s/.*=//; $yi = $line; $yc = 0; next; }
  if ($line =~ /^xr=?/) { $line =~ s/.*=//; $xc = $line; next; }
  if ($line =~ /^yr=?/) { $line =~ s/.*=//; $yc = $line; next; }
  if ($line =~ /^x[+-]/) { $line =~ s/^x//; $xc += $line; next; }
  if ($line =~ /^y[+-]/) { $line =~ s/^y//; $yc += $line; next; }
  if ($line =~ /^>/) { $line =~ s/^>//; $xc += $line; $yc = $yl; next; }
  if ($line =~ /^m=/) { $line =~ s/^m=//; writeMapChunk($line); next; }
  $line = lc($line);
  if ($line eq "ir" or $line eq "reset") { $iconIdx = 0; %iconHash = (); next; }
  if ($line eq "launch") { $launch = 1; next; }
  if ($line eq "debug") { $debug = 1; next; }
  if ($line eq "nodebug") { $debug = 0; next; }
  print "Unknown line $line\n";
}
close(A);

writeFullMap();
if ($launch) { `msp $outFile`; }

#####################################
#
#subroutines
#

sub initArray
{
  my $i;
  my $j;
  for ($i=0; $i < 256; $i++)
  {
    for ($j=0; $j < 256; $j++)
	{
	  $mapAry[$i][$j] = 0;
	}
  }
}

sub writeFullMap
{
  my $buffer;
  open(B, "256.bmp");
  binmode(B);
  open(C, ">$outFile");
  binmode(C);

  read(B, $buffer, 0x436, 0);
  print C $buffer;

  my $i;
  my $j;
  for ($j=255; $j >= 0; $j--)
  {
    for ($i=0; $i <= 255; $i++)
	{
	  #if (!defined($mapAry[$i][$j])) { print("$i $j undefined\n"); } else { print "$i $j defined\n"; }
	  print C chr($mapAry[$i][$j]);
	}
  }
  close(B);
  close(C);
}

sub writeMapChunk
{
  my $mapRead = 0;
  my $row;
  my $l;
  my @tiles;
  open(B, "$xmlFile") || die ("Can't read XML file $xmlFile");
  my $ycur = $yc + $yi;
  my $xcur = $xc + $xi;
  print "Map $_[0] starts at $xcur $ycur\n";
  while ($l = <B>)
  {
    #print "$l";
	chomp($l);
    if ($l =~ /map name=\"$_[0] - [23]D/i)
    {
      $mapRead = 1;
	  $ycur = $yc + $yi;
    }
    if (($mapRead == 1) && ($l =~ /<row>/))
    {
	  $xcur = $xc + $xi;
      $l =~ s/.*<row>//;
	  $l =~ s/<\/row>.*//;
	  @tiles = split(/ /, $l);
	  for (@tiles)
	  {
	    if (!defined($iconHash{$_}))
		{
		  $iconHash{$_} = $iconIdx;
		  $iconIdx++;
		  if ($iconIdx == 257) { die ("Too many icons read in at line $.. Use ir in a line somewhere, or open another file."); }
		}
		if ($debug)
		{
		printf("%2x,%2x,%2x\n", $xc + $xcur, $yc +$ycur, $iconHash{$_});
		}
		$mapAry[$xcur][$ycur] = $iconHash{$_};
		$xcur++;
	  }
	  if ($debug) { print "Adding to $ycur\n"; }
	  $ycur++;
    }
    if (($mapRead == 1) && ($l =~ /<\/chunk>/))
    {
	  #print("Out at line $.\n");
      last;
    }
  }
  if (!$mapRead) { print "WARNING didn't find $_[0]\n"; }
  close(B);
  $yl = $yc;
  $yc = $ycur - $yi;
  print "yc ends at $yc, yi at $yi\n";

}

sub usage
{
print<<EOT;
x2b.pl extracts a bitmap from the world.xml code that Outlaw Editor generates.
Type in the name of the map file you wish to read in. This script should do the rest.
m= = name of map, specified by <map name="..." ... ignore the 3d/2d
xi=x initial, a frame of reference for
xr=x relative, where we move the starting x-coordinates
yi= y initial, a frame of reference for
yr=y relative, where we move the starting x-coordinates
x(+/-)=move the x-coordinate right or left
y(+/-)=move the y-coordinate down or up
>=move x-coordinate right, and y back up
m=write a map chunk according to the map name
ir/reset=clear the icon hash
launch=launch after
debug=turn on debug
nodebug=turn off debug
EOT
exit();

}

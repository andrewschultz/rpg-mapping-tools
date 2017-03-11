###################################################
#
#ahsort.pl
#
#sorts an AHS file by icon number
#
#note imperfect if something is defined twice with itself
#

use strict;
use warnings;

my $inHeader = 1;
my $readTheRest = 0;
my $currentComment = "";
my $currentIcon = "";
my $inIcon = 0;

my @icons;

open(A, "$ARGV[0]") || die ("Need valid file.");

open(B, ">mytemp.ahs");

while ($a = <A>)
{
  if ($readTheRest) { print B $a; next; }
  if ($a =~ /^0/) { $inHeader = 0; } # don't start until in header
  if ($inHeader) { print B $a; next; }
    print "($inIcon) Line $.: $a";
  if ($a =~ /^#/)
  {
    if ($inIcon)
    {
      $currentIcon = "$currentComment$currentIcon";
      push(@icons, $currentIcon);
      $currentComment = $currentIcon = "";
	  $inIcon = 0;
    }
    $currentComment .= $a;
    next;
  }
  if ($a =~ /^;/)
  {
    $currentIcon = "$currentComment$currentIcon";
    push(@icons, $currentIcon);
    printIcons();
    print B $a;
    $readTheRest = 1;
    next;
  }
  if ($a =~ /^0x/)
  {
	if ($inIcon)
	{
    $currentIcon = "$currentComment$currentIcon";
    push(@icons, $currentIcon);
	$currentIcon = $a;
	}
	else
	{
	$currentIcon = "$currentComment$a";
	print "New comment $currentIcon";
    $inIcon = 1;
	}
    $currentComment = "";
	next;
  }
  $currentIcon .= $a;
}

if (!$readTheRest) { die ("You need to end with a semicolon. Well, you don't need to, but you should."); }
close(A);
close(B);

sub printIcons
{
  my $x;
  my @i2 = sort { bareIcon($a) cmp bareIcon($b) } @icons;
  for $x (@i2)
  {
    print B $x;
  }
}

sub bareIcon
{
  my $temp = $_[0];
  $temp =~ s/.*0x/0x/s;
  return $temp;
}
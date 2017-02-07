#################################
# mcbmp.pl
# sends each location chunk to a bitmap
#
# requires the binaries for a Magic Candle game
#

use strict;
use warnings;

opendir(DIR, ".");

my %args;
my $mcm;
my $temp;

my @readdir = readdir(DIR);

closedir(DIR);
 
for $mcm (@readdir)
{
  if ($mcm =~ /M.*\.MCM/i)
  {
    $temp = $mcm; $temp =~ s/.*E//; $temp =~ s/\..*//;
    $args{$temp} .= " $mcm";
  }
}

for (sort keys %args)
{
  print "mcmap E$_.MCT$args{$_}\n";
}

opendir(DIR, ".");

@readdir = readdir(DIR);

open(A, ">mcbmp.htm");

for $mcm (@readdir)
{
  if ($mcm =~ /m.*e.*bmp/i)
  {
    print A "<img src = $mcm>\n";
  }
}

close(A);
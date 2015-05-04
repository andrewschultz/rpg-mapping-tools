#SSFROM.PL
#
#this takes a save state and alters the player's X/Y coordinates to where we can potentially make a complete map from save states
#
#arguments: -d save dir, -x and -y = x/y increment, -xm/-ym = x/y max, -o=offset of x/y
#
#note stuff to add = possibly make x-y 2 bytes

$prefix = "msj";
$saveslot = 0;
$savDir = "c:/emu/nes/fcs";

$xInc = 30; $yInc = 28;
$xMax = 248; $yMax = 245;

$offset = 0x894; #this is pretty arbitrary

while ($count <= @ARGV[0])
{
  $a = @ARGV[$count];
  $b = @ARGV[$count+1];
  for ($a)
  {
    /-d/ && do { $savDir = $b; $count += 2; next; };
    /^[a-z]/ && do { $prefix = $a; $count++; next; };
	/^-x$/ && do { $xInc = $b; $count += 2; next; };
	/^-y$/ && do { $yInc = $b; $count += 2; next; };
	/^-2b$/ && do { $twoBytes = true; next; };
	/^-xm$/ && do { $xMax = $b; $count += 2; next; };
	/^-ym$/ && do { $yMax = $b; $count += 2; next; };
	/^-o$/ && do { $offset = tohex($b); $count += 2; next; };
	/^[0-9]$/ && do { $saveslot = $a; $count++; next; };
	usage();
  }
}

for ($b=7; $b <= $yMax; $b += $xInc)
{
for ($a=8; $a <= $xMax; $a += $yInc)
{
$newFile = "$prefix-$a-$b.map";

print "Writing $newFile.\n";
open(A, "$savDir/$prefix.fc0");

binmode(A);

read(FILE, $buffer, $offset);

open(B, ">$newFile");

print B $buffer;

if ($twoBytes)
{
  print B $a & 0xff;
  print B $a / 0x100;
  print B $b & 0xff;
  print B $b / 0x100;
}
else
{
  print B $a;
  print B $b;
}

read(A, $buffer, 2);

if ($twoBytes) { read(A, $buffer, 2); }

read(A, $buffer, 13000);

print B $buffer;

binmode(B);

close(A);
close(B);
}
}

sub usage
{
USAGE
====================
print<<EOT;
0-9 says which save state to copy from, default 0
an alphanumeric sequence gives the file's base name
-d (dir) switches the directory from c:/emu/nes/fcs
EOT
exit;
}
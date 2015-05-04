$prefix = "msj";
$saveslot = 0;
$savDir = "c:/emu/nes/fcs";

while ($count <= @ARGV[0])
{
  $a = @ARGV[$count];
  $b = @ARGV[$count+1];
  for ($a)
  {
    /-d/ && do { $savDir = $b; $count += 2; next; };
    /^[a-z]/ && do { $prefix = $a; $count++; next; };
	/^[0-9]$/ && do { $saveslot = $a; $count++; next; };
	usage();
  }
}

for ($b=7; $b <=245; $b += 28)
{
for ($a=8; $a <=248; $a += 30)
{
$newFile = "$prefix-$a-$b.map";

print "Writing $newFile.\n";
open(A, "$savDir/$prefix.fc0");

binmode(A);

read(FILE, $buffer, 0x894);

open(B, ">$newFile");

print B $buffer;

print B $a;

print B $b;

read(A, $buffer, 2);

read(A, $buffer, 13000);

print B $buffer;

binmode(B);

close(A);
close(B);
}
}

sub usage
{
print<<EOT;
0-9 says which save state to copy from, default 0
an alphanumeric sequence gives the file's base name
-d (dir) switches the directory from c:/emu/nes/fcs
EOT
}
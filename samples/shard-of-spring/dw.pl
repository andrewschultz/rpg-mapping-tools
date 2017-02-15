open(A, "sum.map");
binmode(A);

open(B, ">dwout.bin");
binmode(B);

read(A, $buffer, 1, 0); # throwaway at start

while (read(A, $buffer, 1, 0))
{
  #$count++;
  $o = ord($buffer);
  $j = $o & 0x80;
  if ($j)
  {
    $rep = $o & 0x7f;
    if (!$rep)
    { $rep = 128; }
    read(A, $buf2, 1, 0);
    print B chr($rep) x (ord($buf2) & 0xff);
    print "Placed $rep " . ord($buf2) .".\n";
  }
  else
  {
    print "Placed $buffer.\n";
    print B $buffer;
  }
  #print $count . " " . ord($buffer) . "\n";
}

close(A);
close(B);
<?php
/*
 $Id: sieve.php,v 1.2 2004/04/21 14:23:45 tim Exp $
 http://www.bagley.org/~doug/shootout/
*/
$n = 18;

sieve($n);

function sieve($n) {
  $count = 0;
  while ($n-- > 0) {
    $count = 0;
    $flags = range (0,8192);
    for ($i=2; $i<8193; $i++) {
      if ($flags[$i] > 0) {
	for ($k=$i+$i; $k <= 8192; $k+=$i) {
	  $flags[$k] = 0;
	}
	$count++;
      }
    }
  }
  print "Count: $count\n";
}
?>

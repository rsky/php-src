--TEST--
Test getimagesize() function : basic functionality - read WebP lossless format.
--SKIPIF--
<?php
if (!defined("IMAGETYPE_WEBP")) {
	die("skip webp format is not available");
}
?>
--FILE--
<?php
var_dump(getimagesize(__DIR__ . "/testLossless.webp", $info));
var_dump($info);
?>
--EXPECT--
array(7) {
  [0]=>
  int(300)
  [1]=>
  int(300)
  [2]=>
  int(18)
  [3]=>
  string(24) "width="300" height="300""
  ["bits"]=>
  int(8)
  ["channels"]=>
  int(3)
  ["mime"]=>
  string(10) "image/webp"
}
array(1) {
  ["WebP"]=>
  string(8) "lossless"
}

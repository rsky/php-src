--TEST--
Test getimagesize() function : basic functionality - read WebP extended format.
--SKIPIF--
<?php
if (!defined("IMAGETYPE_WEBP")) {
	die("skip webp format is not available");
}
?>
--FILE--
<?php
var_dump(getimagesize(__DIR__ . "/test_alpha.webp", $info));
var_dump($info);
var_dump(getimagesize(__DIR__ . "/test_extended.webp", $info));
foreach ($info as $key => $value) {
	echo "$key - length: ". strlen($value) ."; md5: " . md5($value) .  "\n" ;
}
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
  int(4)
  ["mime"]=>
  string(10) "image/webp"
}
array(1) {
  ["WebP"]=>
  string(8) "extended"
}
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
WebP - length: 8; md5: d3e78e3d3b68cb0fbd9f66dcaef93cea
ICCP - length: 7261; md5: 2d6631ab1ed62c9c5359d68ad1b08025
EXIF - length: 1711; md5: 966cce9f013266756e568bdfcef30e70

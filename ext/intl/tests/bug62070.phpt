--TEST--
Bug #62070: Collator::getSortKey() returns garbage
--SKIPIF--
<?php
if (!extension_loaded('intl'))
	die('skip intl extension not enabled');
--FILE--
<?php
$s1 = 'Hello';

$coll = collator_create('en_US');
$res = collator_get_sort_key($coll, $s1);

echo urlencode($res);
--EXPECT--
5%2F%3D%3DC%01%09%01%8F%08

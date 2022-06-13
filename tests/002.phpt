--TEST--
test1() Basic test
--EXTENSIONS--
pdo_mimer
--FILE--
<?php
$ret = test1();

var_dump($ret);
?>
--EXPECT--
The extension pdo_mimer is loaded and working!
NULL

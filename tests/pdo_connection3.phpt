--TEST--
Mimer SQL (Connection): Connect to DB using DBname in DSN, username and password as args

--EXTENSIONS--
pdo_mimer

--SKIPIF--
<?php require('skipif.inc'); ?>

--FILE--
<?php
require("testdb.inc");
$dbh = new PDO(PDO_MIMER_TEST_DSN, PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS);
print "done"
?>
--EXPECT--
done
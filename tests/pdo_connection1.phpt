--TEST--
Mimer SQL (Connection): Connect to DB using only DB name

--EXTENSIONS--
pdo_mimer

--SKIPIF--
<?php require('skipif.inc'); ?>

--FILE--
<?php
require("testdb.inc");
$dbh = new PDO(PDO_MIMER_TEST_DSN);
print "done"
?>
--EXPECT--
done

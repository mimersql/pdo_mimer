--TEST--
Mimer SQL (Connection): Connect to non-existing DB

--EXTENSIONS--
pdo_mimer

--SKIPIF--
<?php require('skipif.inc'); ?>

--FILE--
<?php
require("testdb.inc");
try {
    $dbh = new PDO("mimer:dbname=this-db-does-not-exist");
} catch (PDOException $e) {
    print "PDOException was thrown";
}

?>
--EXPECT--
PDOException was thrown

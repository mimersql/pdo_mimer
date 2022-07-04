--TEST--
Mimer SQL (Connection): Connect to DB with invalid DSN option

--EXTENSIONS--
pdo_mimer

--DESCRIPTION--
As of 04/07/22, expected behaviour not yet defined, i.e. should ivalid options be ignored or cause exception

--SKIPIF--
<?php require('skipif.inc'); ?>

--FILE--
<?php
require("testdb.inc");
try {
    $dbh = new PDO("mimer:non-existing-option=val", PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS);
} catch (PDOException $e) {
    print "PDOException was thrown";
}

?>
--EXPECT--
PDOException was thrown

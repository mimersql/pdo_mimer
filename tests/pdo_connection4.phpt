--TEST--
Mimer SQL (Connection): Connect to DB with incorrect PW

--EXTENSIONS--
pdo_mimer

--SKIPIF--
<?php require('skipif.inc'); ?>

--FILE--
<?php
require("testdb.inc");
try {
    $dbh = new PDO(PDO_MIMER_TEST_DSN, PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS . "wrong");
} catch (PDOException $e) {
    print "PDOException was thrown";
}

?>
--EXPECT--
PDOException was thrown

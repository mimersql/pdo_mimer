--TEST--
PDO Mimer (Connection): Connect to DB with invalid DSN option
--DESCRIPTION--
Intended behaviour by PDO seems to be to ignore invalid DSN options.
--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>
--FILE--
<?php
require 'pdo_mimer_test.inc';
try {
    $dbh = new PDO("mimer:non-existing-option=val", PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS);
} catch (PDOException $e) {
    print PDOMimerTest::error($e);
}
?>
--EXPECT--

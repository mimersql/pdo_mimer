--TEST--
PDO Mimer (Connection): Connect to DB using DBname in DSN, username and password as args

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php
require 'pdo_mimer_test.inc';
try {
    $db = new PDO(PDO_MIMER_TEST_DSN, PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS);
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

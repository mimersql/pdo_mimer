--TEST--
PDO Mimer (Connection): Connect to DB with incorrect PW

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php
require_once 'pdo_mimer_test.inc';
try {
    $db = new PDO(PDO_MIMER_TEST_DSN, PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS . "wrong");
} catch (PDOException $e) {
    print PDOMimerTest::error($e);
}
?>

--EXPECTF--
[%d]: %s

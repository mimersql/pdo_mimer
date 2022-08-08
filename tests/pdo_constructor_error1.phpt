--TEST--
PDO Mimer (Connection): Connect to non-existing DB
--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>
--FILE--
<?php require_once 'pdo_mimer_test.inc';
try {
    $db = new PDO("mimer:dbname=this-db-does-not-exist");
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>
--EXPECTF--
[-%d]: %s
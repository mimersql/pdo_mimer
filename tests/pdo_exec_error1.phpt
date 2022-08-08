--TEST--
PDO Mimer(doer): Error on executing a 'SELECT' statement
--DESCRIPTION--
Tests that PDO::exec triggers error with result set statement
--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>
--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());

try {
    $db = new PDOMimerTest();
    $db->exec("SELECT $column FROM $table");
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>
--EXPECTF--
[-%d]: %s
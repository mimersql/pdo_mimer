--TEST--
PDO Mimer(exec): Error on executing a 'SELECT' statement

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

--EXPECT--

--XFAIL-- 
Expected error message is TBD. See issue #77.
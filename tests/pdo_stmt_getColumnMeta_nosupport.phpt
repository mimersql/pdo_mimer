--TEST--
PDO Mimer(stmt-getColumnMeta): not supported

--DESCRIPTION--
Tests that usage of non-supported function throws exception with error message.

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(true);
    $stmt = $db->query("SELECT * FROM $table");
    $stmt->getColumnMeta(0);
    
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--
SQLSTATE[IM001]: Driver does not support this function: driver does not support getColumnMeta()
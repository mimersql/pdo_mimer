--TEST--
PDO Mimer(lastInsertId): not supported

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
    $db = new PDOMimerTest(null);
    $db->exec("CREATE TABLE $table ($column $type)");

    foreach ($values as $value){
        $db->exec("INSERT INTO $table ($column) VALUES ($value)");
    }

    $db->lastInsertId();
    
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--
SQLSTATE[IM001]: Driver does not support this function: driver does not support lastInsertId()
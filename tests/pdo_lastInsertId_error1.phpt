--TEST--
PDO Mimer(lastInsertId): calling function raises exception

--DESCRIPTION--
Currently not supported.

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

--EXPECTF--
SQLSTATE[%s]: %s
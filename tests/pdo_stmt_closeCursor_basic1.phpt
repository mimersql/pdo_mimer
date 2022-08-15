--TEST--
PDO Mimer(stmt-closeCursor): re-using result set statements

--DESCRIPTION--
When trying to re-use a prepared statement that has a result set
with unfetched rows, the driver throws an API out-of-order error. 
This test verifies that by using closeCursor, a statement can be re-used.

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(true);
    $stmt = $db->prepare("SELECT $column FROM $table WHERE $column = ?");
    $stmt->bindParam(1, $colval, $pdo_type);
    
    $colval = $values[0];
    $stmt->execute();
    $stmt->fetch();

    $stmt->closeCursor();

    $colval = $values[1];
    $stmt->execute();
    
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

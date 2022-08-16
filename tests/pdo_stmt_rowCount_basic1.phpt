--TEST--
PDO Mimer(stmt-rowCount): Count after UPDATE

--DESCRIPTION--
Tests that the function returns the correct number of rows 
affected by the last UPDATE statement. 

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::makeExTablePerson();
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest();
    $stmt = $db->query("UPDATE $table SET name = 'Christoph' WHERE id = 1");
    if (($res = $stmt->rowCount()) !== 1)
        die("Expected num. rows: 1\nActual num. rows: $res");

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

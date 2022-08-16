--TEST--
PDO Mimer(stmt-rowCount): Count after INSERT

--DESCRIPTION--
Tests that the function returns the correct number of rows 
affected by the last INSERT statement. 

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
    $stmt = $db->query("INSERT INTO $table (id, name, lastname) VALUES (68, 'Nick', 'Cage')");
    if (($res = $stmt->rowCount()) !== 1)
        die("Expected num. rows: 1\nActual num. rows: $res");

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

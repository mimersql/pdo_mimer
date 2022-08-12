--TEST--
PDO Mimer (stmt-fetchColumn): Fetch one column from next row

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php include 'pdo_mimer_test.inc';
PDOMimerTest::makeExTableStd();
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(true);
    $stmt = $db->query("SELECT * FROM $table");

    $cell1 = $stmt->fetchColumn();

    if ($cell1 !== $columns[0]->value(1))
        die("Fetched value ($cell1) differ from test table value ({$columns[0]->value(1)})");
    
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

--TEST--
PDO Mimer (stmt-columnCount): count columns in result set

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(true);
    $stmt = $db->prepare("SELECT * FROM $table");
    if ($stmt->columnCount() !== 0)
        die("columnCount should return 0 before result set is available");

    $stmt->execute();
    if ($stmt->columnCount() !== count($columns))
        die("columnCount does not return correct number of columns");

} catch (PDOException $e) {
    print PDOMimerTest::error($e);
}
?>

--EXPECT--

--TEST--
PDO Mimer(prepare): Create PDOStatement object from valid SQL 

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(true);
    $sql = "SELECT $column FROM $table WHERE $column = $values[0]";
    $stmt = $db->prepare($sql);
    if (!is_a($stmt, "PDOStatement"))
        die("Prepare did not create a PDOStatement object");

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

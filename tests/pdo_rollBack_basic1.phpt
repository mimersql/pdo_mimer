--TEST--
PDO Mimer(rollBack): no changes to DB after rollBack

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

    $db->beginTransaction();
    foreach ($values as $value){
        $db->exec("INSERT INTO $table ($column) VALUES ($value)");
    }
    $db->rollback();

    $result = $db->query("SELECT $column FROM $table");
    $nrows = count($result->fetchAll(PDO::FETCH_COLUMN));

    if ($nrows !== 0)
        die ("Rollback did not revert changes");

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>
--EXPECT--

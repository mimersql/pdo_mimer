--TEST--
PDO Mimer(commit): commit makes changes in database visible from other connections

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db_1 = new PDOMimerTest(null);
    $db_2 = new PDOMimerTest(null);
    $db_1->exec("CREATE TABLE $table ($column $type)");

    $db_1->beginTransaction();
    foreach ($values as $value){
        $db_1->exec("INSERT INTO $table ($column) VALUES ($value)");
    }

    $result = $db_2->query("SELECT $column FROM $table");
    $nrows_before = count($result->fetchAll(PDO::FETCH_COLUMN));
    $db_1->commit();
    $result = $db_2->query("SELECT $column FROM $table");
    $nrows_after = count($result->fetchAll(PDO::FETCH_COLUMN));

    if ($nrows_after === $nrows_before)
        die ("Commit did not make changes in DB");

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

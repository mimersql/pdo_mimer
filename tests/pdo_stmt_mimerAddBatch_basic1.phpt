--TEST--
PDO Mimer(stmt-mimerAddBatch): MimerAddBatch called from PDOStatement instance

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
    $stmt = $db->prepare("INSERT INTO $table ($column) VALUES (?)");

    foreach ($values as $i => $value) {
        $stmt->bindValue(1, $value, $pdo_type);
        if (++$i < count($values))
             $stmt->mimerAddBatch();
    }
    $stmt->execute();

    foreach ($db->query("SELECT $column FROM $table", PDO::FETCH_ASSOC) as $i => $row)
        $row["$column"] !== $values[$i] &&
            die("fetched rows don't match inserted values");
} catch (PDOException $e) {
    print PDOMimerTest::error($e);
}
?>

--EXPECT--

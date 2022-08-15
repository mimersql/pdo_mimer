--TEST--
PDO Mimer(query): fetching a result set

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest();
    foreach($db->query("SELECT $column FROM $table", PDO::FETCH_ASSOC) as $i => $row)
        if ($row["$column"] != $values[$i])
            die("Fetch does not match insert: " . $row["$column"] . " => " . $values[$i]);
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

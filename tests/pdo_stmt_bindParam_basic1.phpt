--TEST--
PDO Mimer (stmt-bindParam): bind named placeholders

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

    $stmt = $db->prepare("INSERT INTO $table ($column) VALUES ($param)");
    $stmt->bindParam($param, $value, $pdo_type);

    foreach ($values as $value)
        $stmt->execute();
    $stmt = null;

    $result = $db->query("SELECT $column FROM $table")->fetch(PDO::FETCH_NUM);
    foreach ($result as $i => $val)
        if (($fetched = $result[$i]) !== ($inserted = $values[$i]))
            die("fetched value did not match inserted value: fetched($fetched) != inserted($inserted)");
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

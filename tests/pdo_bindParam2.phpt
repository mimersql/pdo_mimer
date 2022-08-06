--TEST--
PDO Mimer (bindParam): bind positional placeholders

--EXTENSIONS--
pdo_mimer

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>
--FILE--
<?php include 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(null);
    $db->exec("CREATE TABLE $table ($column $type)");

    $stmt = $db->prepare("INSERT INTO $table ($column) VALUES (?)");
    $stmt->bindParam(1, $value, $pdo_type);

    foreach($values as $value)
        $stmt->execute();
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>
--EXPECT--

--TEST--
PDO Mimer(prepare): return false on invalid SQL

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php include 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(true);
    $sql = "This is not valid sql";
    $stmt = $db->prepare($sql);

    if ($stmt !== false)
        die("Prepare did not return false on invalid SQL");

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

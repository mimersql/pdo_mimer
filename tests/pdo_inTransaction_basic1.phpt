--TEST--
PDO Mimer(inTransaction): correct behaviour before, during and after transaction

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

    if ($db->inTransaction())
        die("inTransaction gives true before starting transaction");

    $db->beginTransaction();
    if (!$db->inTransaction())
        die("inTransaction gives false although transaction started");

    $db->commit();
    if ($db->inTransaction())
        die("inTransaction gives true although transaction ended");
    
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>
--EXPECT--

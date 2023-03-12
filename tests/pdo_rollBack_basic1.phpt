--TEST--
PDO Mimer(rollBack): no changes to DB after rollBack

--EXTENSIONS--
pdo
pdo_mimer

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_basic", false);
$dsn = $util->getFullDSN();
$tblName = "basic";

try {
    $db = new PDO($dsn);
    $sql = "SELECT * FROM $tblName";

    // Check table state before transaction
    $nrows_before = count($db->query($sql)->fetchAll());

    $db->beginTransaction();

    // Change table state
    $dmlStmts = $util->getTableDML($tblName);
    foreach ($dmlStmts as $stmt)
        $db->exec($stmt);
    
    // Verify state change
    $nrows_in = count($db->query($sql)->fetchAll());
    if($nrows_in === $nrows_before)
        die("Table state did not change");

    $db->rollback();

    // Verify changes are reverted after rollback
    $nrows_after = count($db->query($sql)->fetchAll());
    if ($nrows_after !== $nrows_before)
        die ("Rollback did not revert changes");

} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--

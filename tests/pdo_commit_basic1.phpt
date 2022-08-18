--TEST--
PDO Mimer(commit): commit makes changes in database visible from other connections

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
    $db_1 = new PDO($dsn);
    $db_2 = new PDO($dsn);   

    $db_1->beginTransaction();

    // Fill table with values from first connection
    $dmlStmts = $util->getTableDML($tblName);
    foreach ($dmlStmts as $stmt)
        $db_1->exec($stmt);

    // Check table state from second connection before commit
    $result = $db_2->query("SELECT * FROM $tblName");
    $nrows_before = count($result->fetchAll(PDO::FETCH_COLUMN));

    $db_1->commit();

    // Are changes visible from second connection?
    $result = $db_2->query("SELECT * FROM $tblName");
    $nrows_after = count($result->fetchAll(PDO::FETCH_COLUMN));
    if ($nrows_after === $nrows_before)
        die ("Commit didn't make changes visible");

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

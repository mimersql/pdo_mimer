--TEST--
PDO Mimer(stmt-mimerAddBatch): MimerAddBatch called from PDOStatement instance

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
$tbl = $util->getTable($tblName);
$rows = $tbl->getRows();
$cols = $tbl->getAllColumns(false);

try {
    $db = new PDO($dsn);
    $insertSQL = $tbl->getInsertSQL();
    $stmt = $db->prepare($insertSQL);

    // Add all parameter batches
    foreach ($rows as $i => $row) {
        foreach($cols as $j => $col)
            $stmt->bindValue($j+1, $row[$col->getName()], $col->getPDOType());
        
        if($i !== count($rows)-1)
            $stmt->mimerAddBatch();
    }

    // Insert and verify
    if(!$stmt->execute())
        die("Executing batch failed\n");
    $stmt = $db->query("SELECT * FROM $tblName");
    $tbl->verifyResultSet($stmt);

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

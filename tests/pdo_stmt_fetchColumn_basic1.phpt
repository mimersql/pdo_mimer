--TEST--
PDO Mimer(stmt-fetchColumn): Fetch one column from next row

--EXTENSIONS--
pdo
pdo_mimer

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_basic");
$dsn = $util->getFullDSN();
$tblName = "basic";
$tbl = $util->getTable($tblName);

try {
    $db = new PDO($dsn);
    $stmt = $db->query("SELECT * FROM $tblName");

    $firstCol = $tbl->getAllColumns(false)[0];
    foreach($firstCol->getValues() as $exp)
        if(($val = $stmt->fetchColumn(0)) !== $exp)
            die("Fetched value ($val) differ from test table value ($exp)\n");
 
} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--

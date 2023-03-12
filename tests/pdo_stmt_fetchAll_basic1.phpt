--TEST--
PDO Mimer(stmt-fetchAll): Fetch all remaining rows

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Tests that a call to fetchAll fetches all remaining rows
and that the fetched values are as expected. 

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
    
    // Rows left after fetchAll?
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    if($stmt->fetch())
        die("There were unfetched rows after call to fetchAll\n");

    // Has expected number of rows?
    if (($act = count($rows)) !== ($exp = $tbl->numberOfRows()))
        die("fetchAll fetched $act rows, expected $exp rows\n");

    // Are the fetched values correct?
    foreach($rows as $i => $row){
        foreach($row as $colName => $val){
            if($val !== ($act = $tbl->getVal($colName, $i)))
                die("Value in fetched row ($val) differ ".
                    "from test table value ($act)\n");
        }
    }

} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--

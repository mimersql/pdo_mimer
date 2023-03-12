--TEST--
PDO Mimer(stmt-getIterator): get iterator to result set

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Uses function to retrieve a result set iterator, which is 
then used to iterate through entire test table whilst
verifying the values. 

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
    $stmt->setFetchMode(PDO::FETCH_NUM);
    $it = $stmt->getIterator();

    foreach($it as $rowidx => $row)
        foreach($row as $colidx => $val){
            $colName = $tbl->getColumnName($colidx);
            $expVal = $tbl->getVal($colName, $rowidx);
            
            if($val !== $expVal)
                die("In column $colName:\nExpected: $expVal\nActual: $val");
        }

    $it = null;

} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--

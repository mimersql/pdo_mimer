--TEST--
PDO Mimer(stmt-fetchObject): Fetching next row as object

--EXTENSIONS--
pdo
pdo_mimer

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_person");
$dsn = $util->getFullDSN();
$tblName = "person";
$tbl = $util->getTable($tblName);

try {
    $db = new PDO($dsn);
    $stmt = $db->query("SELECT * FROM $tblName");
    
    $rows = $tbl->getRows();
    foreach($rows as $rowVals){
        $person = $stmt->fetchObject();
        foreach($rowVals as $colName => $colVal){
            $fetched = $person->$colName;
            if ($fetched !== $colVal)
                die("Column $colName: Fetched value ($fetched) differ ". 
                    "from expected value ($colVal)\n");
        }
    }

} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--

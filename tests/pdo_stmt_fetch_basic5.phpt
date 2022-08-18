--TEST--
PDO Mimer(stmt-fetch): Using PDO::FETCH_OBJ

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Tests that a anonymous class is created 
from the fetched data.

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
        $person = $stmt->fetch(PDO::FETCH_OBJ);
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
?>

--EXPECT--

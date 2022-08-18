--TEST--
PDO Mimer(stmt-fetch): Using PDO::FETCH_CLASS

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Tests that the fetched data is correctly inserted into 
the members of a user defined class. 

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

class Person
{
    public $id;
    public $firstname;
    public $lastname;
    public $birthday;
}

try {
    $db = new PDO($dsn);
    $stmt = $db->query("SELECT * FROM $tblName");
    $stmt->setFetchMode(PDO::FETCH_CLASS, 'Person');
    
    $rows = $tbl->getRows();
    foreach($rows as $rowVals){
        $person = $stmt->fetch();
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

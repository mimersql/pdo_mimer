--TEST--
PDO Mimer(stmt-debugDumpParams): prepare statement, bind params and dump

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Verifies the output of the debugDumpParams function for a statement
with two named parameters, one bound via bindParam and the other 
via bindValue.  

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
    $id_min = 1;
    $stmt = $db->prepare("SELECT lastname FROM $tblName WHERE id > :idmin AND firstname = :fname");
    $stmt->bindParam(':idmin', $id_min, PDO::PARAM_INT);
    $stmt->bindValue(':fname', "Alice", PDO::PARAM_STR);
    $stmt->debugDumpParams();

} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--
SQL: [68] SELECT lastname FROM person WHERE id > :idmin AND firstname = :fname
Params:  2
Key: Name: [6] :idmin
paramno=0
name=[6] ":idmin"
is_param=1
param_type=1
Key: Name: [6] :fname
paramno=1
name=[6] ":fname"
is_param=1
param_type=2
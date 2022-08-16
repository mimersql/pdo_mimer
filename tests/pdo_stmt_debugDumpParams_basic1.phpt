--TEST--
PDO Mimer(stmt-debugDumpParams): with named parameters

--DESCRIPTION--
Verifies the output of the debugDumpParams function for a statement
with two named parameters, one bound via bindParam and the other 
via bindValue.  

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::makeExTablePerson();
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest();
    $id_min = 1;
    $stmt = $db->prepare("SELECT name FROM $table WHERE id > :idmin AND id < :idmax");
    $stmt->bindParam(':idmin', $id_min, PDO::PARAM_INT);
    $stmt->bindValue(':idmax', 12, PDO::PARAM_INT);
    $stmt->execute();
    $stmt->debugDumpParams();

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--
SQL: [72] SELECT name FROM pdo_mimer_test_person WHERE id > :idmin AND id < :idmax
Params:  2
Key: Name: [6] :idmin
paramno=0
name=[6] ":idmin"
is_param=1
param_type=1
Key: Name: [6] :idmax
paramno=1
name=[6] ":idmax"
is_param=1
param_type=1
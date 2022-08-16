--TEST--
PDO Mimer(stmt-debugDumpParams): with positional parameters

--DESCRIPTION--
Verifies the output of the debugDumpParams function for a statement
with two positional parameters, one bound via bindParam and the other 
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
    $stmt = $db->prepare("SELECT name FROM $table WHERE id > (?) AND id < (?)");
    $stmt->bindParam(1, $id_min, PDO::PARAM_INT);
    $stmt->bindValue(2, 12, PDO::PARAM_INT);
    $stmt->execute();
    $stmt->debugDumpParams();

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--
SQL: [66] SELECT name FROM pdo_mimer_test_person WHERE id > (?) AND id < (?)
Params:  2
Key: Position #0:
paramno=0
name=[0] ""
is_param=1
param_type=1
Key: Position #1:
paramno=1
name=[0] ""
is_param=1
param_type=1
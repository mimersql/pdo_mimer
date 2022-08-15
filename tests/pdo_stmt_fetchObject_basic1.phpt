--TEST--
PDO Mimer(stmt-fetchObject): Fetch next row as object

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::makeExTablePerson();
extract(PDOMimerTest::extract());

try {
    $db = new PDOMimerTest(true);
    $stmt = $db->query("SELECT * FROM $table");

    $rowcnt = 1;
    while($person = $stmt->fetchObject()){
        if($person->id !== $id->value($rowcnt) || 
            $person->name !== $name->value($rowcnt))
            die("Value in class member differs from test data");
        $rowcnt++;
    }

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

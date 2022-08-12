--TEST--
PDO Mimer (stmt-fetch): Fetching using PDO::FETCH_LAZY

--DESCRIPTION--
Tests that PDO::FETCH_LAZY fetches data into 
anonymous object correctly. Does not test the lazy behaviour. 

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
    while($person = $stmt->fetch(PDO::FETCH_LAZY)){
        if($person->id !== $columns[0]->value($rowcnt) || 
            $person->name !== $columns[1]->value($rowcnt))
            die("Value in class member differs from test data");
        $rowcnt++;
    }

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

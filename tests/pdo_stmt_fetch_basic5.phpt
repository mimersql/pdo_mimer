--TEST--
PDO Mimer(stmt-fetch): Fetching using PDO::FETCH_OBJ

--DESCRIPTION--
Tests that a anonymous class is created 
from the fetched data.

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
    while($person = $stmt->fetch(PDO::FETCH_OBJ)){
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

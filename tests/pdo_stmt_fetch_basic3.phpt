--TEST--
PDO Mimer (stmt-fetch): Fetching using PDO::FETCH_CLASS

--DESCRIPTION--
Tests that the fetched data is correctly inserted into 
the members of a user defined class. 

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::makeExTablePerson();
extract(PDOMimerTest::extract());

class Person
{
    public $id;
    public $name;
}

try {
    $db = new PDOMimerTest(true);
    $stmt = $db->query("SELECT * FROM $table");
    $stmt->setFetchMode(PDO::FETCH_CLASS, 'Person');

    $rowcnt = 1;
    while($person = $stmt->fetch()){
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

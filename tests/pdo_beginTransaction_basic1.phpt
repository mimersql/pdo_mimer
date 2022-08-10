--TEST--
PDO Mimer(beginTransaction): no insertion in table before commit

--DESCRIPTION--
beginTransaction should turn off autocommit mode, so no changes should 
be made to the database before a following commit. The user making the 
transaction will see the changes even before the commit, so we must check
the database from a different connection. 

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db_1 = new PDOMimerTest(null);
    $db_1->exec("CREATE TABLE $table ($column $type)");
    
    $db_1->beginTransaction();
    foreach ($values as $value){
        $db_1->exec("INSERT INTO $table ($column) VALUES ($value)");
    }

    $db_2 = new PDOMimerTest(null);
    $result = $db_2->query("SELECT $column FROM $table");
    $nrows = count($result->fetchAll(PDO::FETCH_COLUMN));
    if ($nrows !== 0)
        die ("DB changed before a commit was made");

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

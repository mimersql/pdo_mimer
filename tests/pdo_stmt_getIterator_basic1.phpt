--TEST--
PDO Mimer(stmt-getIterator): get iterator to result set

--DESCRIPTION--
Uses function to retrieve a result set iterator, which is 
then used to iterate through entire test table whilst
verifying the values. 

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());

try {
    $db = new PDOMimerTest();
    $stmt = $db->query("SELECT * FROM $table");
    $stmt->setFetchMode(PDO::FETCH_NUM);
    $it = $stmt->getIterator();

    foreach($it as $rowidx => $row)
        foreach($row as $colidx => $val)
            if($val !== ($exp = $columns[$colidx]->value($rowidx+1)))
                die("In column {$columns[$colidx][NAME]}:\nExpected: $exp\nActual: $val");

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

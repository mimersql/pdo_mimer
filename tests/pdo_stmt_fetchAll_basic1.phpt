--TEST--
PDO Mimer (stmt-fetchAll): Fetch all remaining rows

--DESCRIPTION--
Tests that a call to fetchAll fetches all remaining rows
and that the fetched values are as expected. 

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php include 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(true);
    $stmt = $db->query("SELECT * FROM $table");
    
    // Does it fetch all rows?
    $rows = $stmt->fetchAll(PDO::FETCH_NUM);
    if($stmt->fetch())
        die("There were unfetched rows after call to fetchAll");

    // Are the fetched values correct?
    foreach($rows as $i => $row){
        foreach($columns as $j => $col){
            if($row[$j] !== $col->value($i+1))
                die("Value in fetched row differs from test table value");
        }
    }

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

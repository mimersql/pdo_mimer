--TEST--
PDO Mimer (stmt-fetch): Fetching using PDO::FETCH_ASSOC

--DESCRIPTION--
Fetches each row in test table as an associative array and verifies 
that the array keys are the same as the column names and that the values
are the same as in the test table.

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
    
    $rowcnt = 1;
    while($row = $stmt->fetch(PDO::FETCH_ASSOC)){
        foreach ($columns as $col){
            if (!array_key_exists($col[NAME], $row))
                die("Key for column $col[NAME] missing from array");

            if ($row[$col[NAME]] !== $col->value($rowcnt))
                die("Fetched value differs from expected value");
        }
        $rowcnt++;
    }

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

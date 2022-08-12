--TEST--
PDO Mimer (stmt-fetch): Fetching using PDO::FETCH_BOTH

--DESCRIPTION--
Tests that the fetched array has the correct format and values.

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(true);
    $stmt = $db->query("SELECT * FROM $table");
    
    $rowcnt = 1;
    while($row = $stmt->fetch(PDO::FETCH_BOTH)){
        foreach ($columns as $i => $col){
            // Check array format
            if (!array_key_exists($col[NAME], $row))
                die("No column name key for column $col in array");
            if (!array_key_exists($i, $row))
                die("No column index key for column $col in array");
            
            // Check array values
            if ($row[$col[NAME]] !== $row[$i])
                die("Values index by column name and column index differ");
            if ($row[$i] !== $col->value($rowcnt))
                die("Fetched value ($row[$i]) differ from test table value ($col->value($rowcnt)) ");
        }
        $rowcnt++;
    }

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

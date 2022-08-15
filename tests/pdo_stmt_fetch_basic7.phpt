--TEST--
PDO Mimer(stmt-fetch): Fetching using PDO::FETCH_NUM

--DESCRIPTION--
Should fetch each row as a 0-indexed array. Test verifies 
format and values of that array.

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::makeExTableStd();
extract(PDOMimerTest::extract());

try {
    $db = new PDOMimerTest(true);
    $stmt = $db->query("SELECT * FROM $table");

    $rowcnt = 1;
    while($row = $stmt->fetch(PDO::FETCH_NUM)){

        if(count($row) !== count($columns))
            die("Size of fetched row does not matched num. of columns in test table");

        if (array_keys($row) !== range(0, count($columns)-1))
            die("Fetched row keys do not match test table column indexes");

        foreach($row as $i => $cell){
            if ($cell !== $columns[$i]->value($rowcnt))
                die("Fetched value differs from test table value");
        }

        $rowcnt++;
    }

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

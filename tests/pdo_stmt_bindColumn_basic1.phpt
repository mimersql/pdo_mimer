--TEST--
PDO Mimer (stmt-bindColumn): 

--DESCRIPTION--
Binds a different variable to each column in test table, 
iterates through the rows and verifies the values. 

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php include 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(true);
    $stmt = $db->prepare("SELECT * FROM $table");
    $stmt->execute();
    
    // Create the vars
    foreach ($columns as $i => $col){
        extract(PDOMimerTest::extract($i));
        $stmt->bindColumn($i+1, ${"res_" . $col}, $pdo_type);
    }
    
    // Fetch and verify
    $rowcnt = 0;
    while($stmt->fetch(PDO::FETCH_BOUND)){
        foreach ($columns as $i => $col){
            $curr_val = ${'res_' . $col};
            $exp_val = $col->value($rowcnt+1);
          
            if ($curr_val !== $exp_val)
                die("Column ($col): Value bound to variable ($curr_val) " .
                        "differ from table content ($exp_val)");
        }
        $rowcnt++;
    }

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

--TEST--
PDO Mimer(LOB): access LOBs as stream

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
LOBs in DB are supposed to be accessed as a PHP stream.
This test verifies that that is possible by extracting 
some LOB test data using bindColumn(). 

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_lobs");
$dsn = $util->getFullDSN();
$tblName = "lobs";
$tbl = $util->getTable($tblName);

try {
    foreach($tbl->getColumnsExcept(["id"]) as $colName => $col){
        $type = $col->getMimerType();
        print "Testing $type... ";
        $db = new PDO($dsn);
        $stmt = $db->query("SELECT $colName FROM $tblName WHERE id = 1");
        $stmt->bindColumn(1, $lob, PDO::PARAM_LOB);
        $stmt->fetch(PDO::FETCH_BOUND);
        
        $expVal = $tbl->getVal($colName, 0);
        $expValLen = strlen($expVal);
        $fetchedVal = fread($lob, $expValLen); 
    
        if($expVal !== $fetchedVal) {
            if($type === "BLOB"){ // for more readable error output
                $expVal = bin2hex($expval);
                $fetchedVal = bin2hex($fetchedVal);
            }
            die("Expected value ($expVal) differ from fetched value ($fetchedVal)\n");
        }
        print "OK\n";
    }
}
catch (PDOException $e) {
    print $e->getMessage();
}

?>

--EXPECT--
Testing CLOB... OK
Testing NCLOB... OK
Testing BLOB... OK
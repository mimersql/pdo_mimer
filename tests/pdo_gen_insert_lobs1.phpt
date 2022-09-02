--TEST--
PDO Mimer(LOB): inserting LOBs from file using bindParam

--EXTENSIONS--
pdo
pdo_mimer

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
$id = $util->getNextTableID($tblName);

try {
    foreach($tbl->getColumnsExcept(["id"]) as $colName => $col){
        $type = $col->getMimerType();
        print "Testing $type... ";

        // Put test data in a file
        $expVal = $tbl->getVal($colName, 0);
        $expValLen = strlen($expVal); 
        fwrite($fp = tmpfile(), $expVal);
        rewind($fp);

        // Insert into DB from file
        $db = new PDO($dsn);
        $stmt = $db->prepare("INSERT INTO $tblName (id, $colName) VALUES ($id, :$colName)");
        $stmt->bindParam(":$colName", $fp, PDO::PARAM_LOB);
        $stmt->execute();
        fclose($fp);

        // Verify value of inserted data
        $stmt = $db->query("SELECT $colName FROM $tblName WHERE id = $id");
        $stmt->bindColumn(1, $lob, PDO::PARAM_LOB);
        $res = $stmt->fetch(PDO::FETCH_BOUND);
        if (get_resource_type($lob) !== 'stream')
            die("Bound variable is not a stream resource after fetch()");

        $fetchedVal = fread($lob, $expValLen); 
        if ($fetchedVal !== $expVal) {
            die("Inserted valued ($expVal) differ from fetched value ($fetchedVal)");
        }

        print "OK\n";
        fclose($lob);
        $id++;
    }

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--
Testing CLOB... OK
Testing NCLOB... OK
Testing BLOB... OK
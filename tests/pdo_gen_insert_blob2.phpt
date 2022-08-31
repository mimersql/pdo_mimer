--TEST--
PDO Mimer(LOB): inserting a BLOB larger than available process memory

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
The purpose of this test is to verify that the PDO driver is reading in
the LOB data in chunks, instead of trying to allocate a buffer to hold
the entire data before inserting it into DB.

Validates the number but not content of inserted bytes.

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_lobs");
$dsn = $util->getFullDSN();
$tblName = "lobs";
$colName = "blobcol";
$tbl = $util->getTable($tblName);
$nextID = $util->getNextTableID($tblName);

try {
    // Put test data in file (repeat it until file is larger that process memory)
    $tstBinStr = $tbl->getVal($colName, 0);
    $binStrLen = strlen($tstBinStr); 
    $availableMem = $util::limitMemory();
    $fileSize = $availableMem + 10000;
    $requiredReps = ceil($fileSize / $binStrLen);
    $fp = $util::createFile($requiredReps, $tstBinStr);

    // Insert into DB from file
    $db = new PDO($dsn);
    $db->exec("ALTER TABLE $tblName ALTER COLUMN $colName SET DATA TYPE BLOB(10M)"); //increase size
    $stmt = $db->prepare("INSERT INTO $tblName (id, $colName) VALUES ($nextID, :$colName)");
    $stmt->bindParam(":$colName", $fp, PDO::PARAM_LOB);
    $stmt->execute();
    $stmt = null; 

    // Verify value of inserted data
    $stmt = $db->query("SELECT $colName FROM $tblName WHERE id = $nextID");
    $stmt->bindColumn(1, $lob, PDO::PARAM_LOB);
    $stmt->fetch(PDO::FETCH_BOUND);
    
    if (get_resource_type($lob) !== 'stream')
        die("Bound variable is not a stream resource after fetch()\n");
    
    // Content should just be test data repeated
    $fetchedReps = 0;
    while($tstBinStr === ($fetchedStr = fread($lob, $binStrLen)))
        $fetchedReps++;

    if($fetchedReps != $requiredReps)
        die("Content of BLOB differ from test data\n");

    if(!feof($lob))
        die("File stream EOF not set after all matching data read\n");

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

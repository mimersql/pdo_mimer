--TEST--
PDO Mimer(LOB): inserting a BLOB from file

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Inserts a blob of just a few bytes and validates the content.

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
$id = $util->getNextTableID($tblName);

try {
    // Put test data in file
    $tstBinStr = $tbl->getVal($colName, 0);
    $binStrLen = strlen($tstBinStr); 
    fwrite($fp = tmpfile(), $tstBinStr);
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

    $fetchedBinStr = fread($lob, $binStrLen); 
    if ($fetchedBinStr !== $tstBinStr) {
        die("Inserted valued ($tstnum) differ from fetched value ($blobAsInt)");
    }
    fclose($lob);

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

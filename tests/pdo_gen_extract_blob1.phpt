--TEST--
PDO Mimer(LOB): access BLOB as stream

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
LOBs in DB are supposed to be accessed as a PHP stream.
This test verifies that that is possible with BLOBs. 

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
$colName = "blobcol";

try {
    $db = new PDO($dsn);
    $stmt = $db->query("SELECT $colName FROM $tblName FETCH 1");
    $stmt->bindColumn(1, $lob, PDO::PARAM_LOB);
    $stmt->fetch(PDO::FETCH_BOUND);
    
    $binStr = fread($lob, 4); 
    $expVal = $tbl->getVal($colName, 0);

    $fetchedAsHex = bin2hex($binStr);
    $expAsHex = bin2hex($expVal);

    if($expAsHex !== $fetchedAsHex)
      die("Expected value ($expAsHex) differ from fetched value ($fetchedAsHex)");

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

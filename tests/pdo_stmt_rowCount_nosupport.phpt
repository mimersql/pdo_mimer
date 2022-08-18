--TEST--
PDO Mimer(stmt-rowCount): return 0 after updating one row

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Currently rowCount is not supported and should always return 0.
This test verifies that it returns 0 after having updated 
one row. 

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_basic");
$dsn = $util->getFullDSN();
$tblName = "basic";
$colName = "text";

try {
    $db = new PDO($dsn);
    $stmt = $db->query("UPDATE $tblName SET $colName = NULL WHERE id = 1");
    if (($res = $stmt->rowCount()) !== 0)
        die("rowCount() returned non-zero value\n");

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

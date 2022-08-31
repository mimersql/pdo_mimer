--TEST--
PDO Mimer(stmt-getColumnMeta): not supported

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Tests that usage of non-supported function throws exception with error message.

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_basic");
$dsn = $util->getFullDSN();
$tblName = "basic";

try {
    $db = new PDO($dsn);
    $stmt = $db->query("SELECT * FROM $tblName");
    $stmt->getColumnMeta(0);
    
} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--
SQLSTATE[IM001]: Driver does not support this function: driver doesn't support meta data
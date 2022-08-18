--TEST--
PDO Mimer(stmt-columnCount): count columns in result set

--EXTENSIONS--
pdo
pdo_mimer

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_basic");
$dsn = $util->getFullDSN();
$tblName = "basic";
$tbl = $util->getTable($tblName);

try {
    $db = new PDO($dsn);
    $stmt = $db->prepare("SELECT * FROM $tblName");

    if ($stmt->columnCount() !== 0)
        die("columnCount should return 0 before result set is available");

    $stmt->execute();
    if ($stmt->columnCount() !== count($tbl->getAllColumns()))
        die("columnCount does not return correct number of columns");

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

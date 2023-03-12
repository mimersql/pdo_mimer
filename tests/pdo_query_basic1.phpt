--TEST--
PDO Mimer(query): create statement with result set

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
    // Does it generate a statement object?
    $db = new PDO($dsn);
    $stmt = $db->query("SELECT * FROM $tblName", PDO::FETCH_ASSOC);
    if (!is_a($stmt, "PDOStatement"))
        die("Query did not create a PDOStatement object");

    // Is the data correct? 
    $tbl->verifyResultSet($stmt);

} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--

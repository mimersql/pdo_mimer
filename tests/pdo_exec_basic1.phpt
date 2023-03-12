--TEST--
PDO Mimer(exec): Basic usage - DELETE

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Tests that PDO::exec can be used for executing a 
simple DML statement. 

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
    $db1 = new PDO($dsn);

    // Save table state before 
    $stmt = $db->query("SELECT COUNT(*) as nrows FROM $tblName");
    $res = $stmt->fetch(PDO::FETCH_ASSOC);
    $nrows_before = $res['nrows'];

    if ($db->exec("DELETE FROM $tblName") === false)
        die("exec() returned false");

    // Verify table change
    $stmt = $db->query("SELECT COUNT(*) as nrows FROM $tblName");
    $res = $stmt->fetch(PDO::FETCH_ASSOC);
    $nrows_after = $res['nrows'];
    
    if ($nrows_before === $nrows_after)
        die("exec() did not change state of table\n");

} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--
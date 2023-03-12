--TEST--
PDO Mimer(prepare): Create PDOStatement object from valid SQL 

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
    $stmt = $db->prepare("SELECT * FROM $tblName");
    if (!is_a($stmt, "PDOStatement"))
        die("Prepare did not create a PDOStatement object");

} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--

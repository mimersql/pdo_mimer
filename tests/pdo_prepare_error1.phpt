--TEST--
PDO Mimer(prepare): return false on invalid SQL

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

try {
    $db = new PDO($dsn);
    $stmt = $db->prepare("Invalid SQL");
    if ($stmt !== false)
        die("Prepare did not return false");

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

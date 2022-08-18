--TEST--
PDO Mimer(prepare): false when using placeholder for column or table name

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
    $stmt = $db->prepare("SELECT ? FROM $tblName");
    if ($stmt !== false)
        die("Prepare did not return false for column name");

    $stmt = $db->prepare("SELECT id FROM ?");
    if ($stmt !== false)
        die("Prepare did not return false for table name");

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

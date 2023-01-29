--TEST--
PDO Mimer(prepare): throw exception when using placeholder for column or table name

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
    die($e->getMessage());
}
?>

--EXPECT--
SQLSTATE[HY000]: General error: -12262 The type of the parameter marker cannot be determined
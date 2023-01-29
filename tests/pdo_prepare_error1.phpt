--TEST--
PDO Mimer(prepare): throw exception on invalid SQL

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
    //unreachable
} catch (PDOException $e) {
    die($e->getMessage());
}
?>

--EXPECT--
SQLSTATE[HY000]: General error: -12103 Syntax error, IDENTIFIER 'SQL'  assumed to mean 'COMMIT'
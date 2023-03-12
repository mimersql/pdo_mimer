--TEST--
PDO Mimer(exec): Number of affected rows - DELETE

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION-- 
Getting the number of affected rows is currently
not supported, the expected return value is always 0.

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
    $nAffected = $db->exec("DELETE FROM $tblName");
    if($nAffected !== 0)
        die("exec() returns non-zero value ($nAffected) on DELETE statement");
} catch (PDOException $e) {
    print $e->getMessage();
}

$db = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--
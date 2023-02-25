--TEST--
PDO Mimer(Constructor): Connect to DB with incorrect PW

--EXTENSIONS--
pdo
pdo_mimer

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil();
$dsnArr = $util->getConfigValue("connection->dsn");

try {
    $db = new PDO("mimer:{$dsnArr['dbname']}", $dsnArr['user'], $dsnArr['password'] . "wrong");
    $db->exec("SELECT * FROM system.manyrows");
} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECTF--
SQLSTATE[%s] [%s] %s

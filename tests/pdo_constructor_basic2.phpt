--TEST--
PDO Mimer(Constructor): Connect to DB using DBname in DSN, username and password as args

--EXTENSIONS--
pdo
pdo_mimer

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
PDOMimerTestUtil::skipIfWindows();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil();
$dsnArr = $util->getConfigValue("connection->dsn");

try {
    $db = new PDO("mimer:" . $dsnArr['dbname'], $dsnArr['user'], $dsnArr['password']);
} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

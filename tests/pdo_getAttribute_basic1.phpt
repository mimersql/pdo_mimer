--TEST--
PDO Mimer(getAttribute): getting the generic attributes

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
$dsn = $util->getFullDSN();

$attributes = array(
    "AUTOCOMMIT","ERRMODE", "CASE", "CLIENT_VERSION", "CONNECTION_STATUS",
    "ORACLE_NULLS", "PERSISTENT", "PREFETCH", "SERVER_INFO", "SERVER_VERSION",
    "TIMEOUT"
);

try {
    $db = new PDO($dsn);

    foreach ($attributes as $val) {
        if($db->getAttribute(constant("PDO::ATTR_$val")) === null)
            print("Could not get attribute PDO::ATTR_$val" . "\n");
    }

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

--XFAIL-- 
This test is WIP pending discussion on exception vs. null-returning 
behaviour. See issue #65. 

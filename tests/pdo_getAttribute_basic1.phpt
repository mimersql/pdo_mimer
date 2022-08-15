--TEST--
PDO Mimer(getAttribute): getting the generic attributes

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());

$attributes = array(
    "AUTOCOMMIT","ERRMODE", "CASE", "CLIENT_VERSION", "CONNECTION_STATUS",
    "ORACLE_NULLS", "PERSISTENT", "PREFETCH", "SERVER_INFO", "SERVER_VERSION",
    "TIMEOUT"
);

try {
    $db = new PDOMimerTest(null);

    foreach ($attributes as $val) {
        if($db->getAttribute(constant("PDO::ATTR_$val")) === null)
            print("Could not get attribute PDO::ATTR_$val" . "\n");
    }

} catch (PDOException $e) {
    die($e->getMessage());
}
?>

--EXPECT--

--XFAIL-- 
This test is WIP pending discussion on exception vs. null-returning 
behaviour. See issue #65. 

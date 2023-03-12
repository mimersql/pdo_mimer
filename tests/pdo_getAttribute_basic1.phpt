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

        try {
            if($db->getAttribute(constant("PDO::ATTR_$val")) === null)
                print("Could not get attribute PDO::ATTR_$val" . "\n");
        } catch (PDOException $e) {
            print $e->getMessage() . "\n";
        }
    }

} catch (PDOException $e) {
    die($e->getMessage());
}

PDOMimerTestSetup::tearDown();
?>

--EXPECTREGEX--
(SQLSTATE\[IM001\]: Driver does not support this function: driver does not support that attribute\n*){5}
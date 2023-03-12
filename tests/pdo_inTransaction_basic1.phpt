--TEST--
PDO Mimer(inTransaction): correct behaviour before, during and after transaction

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

try {
    $db = new PDO($dsn);

    if ($db->inTransaction())
        die("inTransaction gives true before starting transaction");

    $db->beginTransaction();
    if (!$db->inTransaction())
        die("inTransaction gives false although transaction started");

    $db->commit();
    if ($db->inTransaction())
        die("inTransaction gives true although transaction ended");
    
} catch (PDOException $e) {
    print $e->getMessage();
}

PDOMimerTestSetup::tearDown();
?>

--EXPECT--

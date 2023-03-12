--TEST--
PDO Mimer(lastInsertId): not supported

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Tests that usage of non-supported function throws exception with error message.

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_basic");
$dsn = $util->getFullDSN();

try {
    $db = new PDO($dsn);
    $db->lastInsertId();
    
} catch (PDOException $e) {
    print $e->getMessage();
}

PDOMimerTestSetup::tearDown();
?>

--EXPECT--
SQLSTATE[IM001]: Driver does not support this function: driver does not support lastInsertId()
--TEST--
PDO Mimer(TODO function): TODO Subject

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
TODO description

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_basic");
$dsn = $util->getFullDSN();

// TODO setup
try {
    $db = new PDO($dsn);
    // TODO tests
} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

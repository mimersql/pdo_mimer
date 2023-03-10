--TEST--
PDO Mimer(Constructor): Connect to non-existing DB

--EXTENSIONS--
pdo
pdo_mimer

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
try {
    $db = new PDO("mimer:dbname=this-db-does-not-exist");
} catch (PDOException $e) {
    print $e->getMessage();
}

PDOMimerTestSetup::tearDown();
?>

--EXPECTF--
SQLSTATE[%c%c%d] [%i] Database <%s> not found in SQLHOSTS file
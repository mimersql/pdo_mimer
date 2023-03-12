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

try {
    $db = new PDO(PDOMimerTestConfig::getDSN());
} catch (PDOException $e) {
    print $e->getMessage();
}

PDOMimerTestSetup::tearDown();
?>

--EXPECT--

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
?>

--EXPECTF--

--XFAIL--
Expected error message is TBD. See issue #75. 
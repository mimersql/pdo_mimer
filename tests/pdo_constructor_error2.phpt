--TEST--
PDO Mimer(Constructor): Connect to DB with incorrect PW

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
$dbname = PDOMimerTestConfig::getDBName();
$user = PDOMimerTestConfig::getUser();
$pass = PDOMimerTestConfig::getPassword();

try {
    $db = new PDO("mimer:dbname=$dbname", $user, $pass . "wrong");
    $db->exec("SELECT * FROM SYSTEM.MANYROWS");
} catch (PDOException $e) {
    print $e->getMessage();
}

PDOMimerTestSetup::tearDown();
?>

--EXPECTF--
SQLSTATE[%s] [%s] %s

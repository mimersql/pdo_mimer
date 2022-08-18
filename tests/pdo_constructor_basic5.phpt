--TEST--
PDO Mimer(Constructor): Connect to DB with invalid DSN option

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Intended behaviour by PDO seems to be to ignore invalid DSN options.

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil();
$dsn = $util->getFullDSN();
try {
    $db = new PDO($dsn . "non-existent-option=val");
} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

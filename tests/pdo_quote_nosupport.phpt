--TEST--
PDO Mimer(quote): not supported 

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
$util = new PDOMimerTestUtil();
$dsn = $util->getFullDSN();

try {
    $db = new PDO($dsn);
    $str = 'Test \' string';
    $res = $db->quote($str);

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--
SQLSTATE[IM001]: Driver does not support this function: driver does not support quoting

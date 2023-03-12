--TEST--
PDO Mimer(stmt-getColumnMeta): not supported

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
$tblName = "basic";

try {
    $db = new PDO($dsn);
    $stmt = $db->query("SELECT * FROM $tblName");
    var_dump($stmt->getColumnMeta(0));

} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--
array(7) {
  ["pdo_type"]=>
  int(1)
  ["type"]=>
  string(7) "integer"
  ["native_type"]=>
  string(7) "INTEGER"
  ["flags"]=>
  array(0) {
  }
  ["name"]=>
  string(2) "id"
  ["len"]=>
  int(-1)
  ["precision"]=>
  int(0)
}
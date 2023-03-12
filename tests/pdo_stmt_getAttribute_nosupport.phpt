--TEST--
PDO Mimer(stmt-getAttribute): getting non-supported attributes

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Currently statements have no settable/gettable attributes. 
This test verifies that the usage of the function throws an exception
with a relevant error message for all generic attribute constants defined by PDO
and one check at the start of the range where any potential driver specific
attribute constants would be.

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_basic");
$dsn = $util->getFullDSN();
$tblName = "basic";

$max_pdo_attr_const_val = 21;
$start_of_driver_specific = 1000;

$db = new PDO($dsn);
$stmt = $db->query("SELECT * FROM $tblName");

// Test getting all generic attribute constants
for ($i = 0; $i <= $max_pdo_attr_const_val; $i++){
    try {
        $stmt->getAttribute($i);
    } catch (PDOException $e) {
        print $e->getMessage() . "\n";
    }
}

// Test at start of driver specific attribute range
try{
    $stmt->getAttribute($start_of_driver_specific);
} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECTREGEX--
(SQLSTATE\[IM001\]: Driver does not support this function: This driver doesn't support getting attributes\n*){22}
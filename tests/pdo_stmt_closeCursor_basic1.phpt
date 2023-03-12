--TEST--
PDO Mimer(stmt-closeCursor): re-using result set statements

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
When trying to re-use a prepared statement that has a result set
with unfetched rows, the driver throws an API out-of-order error. 
This test verifies that by using closeCursor, a statement can be re-used.

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';

$util = new PDOMimerTestUtil("db_basic");
$dsn = $util->getFullDSN();
$tblName = "basic";
$tbl = $util->getTable($tblName);
$colName = "text";

try {
    $db = new PDO($dsn);
    $stmt = $db->prepare("SELECT $colName FROM $tblName WHERE id = ?");

    foreach ($tbl->getColumn('id')->getValues() as $id){
        // Fetch
        $stmt->bindParam(1, $id, PDO::PARAM_INT);
        $stmt->execute();
        $res = $stmt->fetch(PDO::FETCH_ASSOC);
        $stmt->closeCursor();

        // Verify 
        if(($act = $res[$colName]) !== ($exp = $tbl->getVal($colName, $id-1)))
            die("Value fetched with reused statement ($act) is not as expected ($exp)\n");
    }

} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--

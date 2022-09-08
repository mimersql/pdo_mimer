--TEST--
PDO Mimer(stmt-fetch): Using PDO::FETCH_NUM

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Verifies that:
1. fetch() returns an array of values with indexes that match column index
2. that the values are as expected (same as test data)

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_allTypes");
$tables = $util->getAllTables();
$util->testEachTable($tables, 'test');

function test($table, $dsn): ?string {
    try {
        $db = new PDO($dsn);
        $rowVals = array_values($table->getRow(0));
        $tblName = $table->getName();
        $stmt = $db->query("SELECT * FROM $tblName WHERE id = 1");
        $fetched = $stmt->fetch(PDO::FETCH_NUM);

        if(($act = count($fetched)) !== ($exp = count($rowVals)))
            return "Number of elements in result set ($act) differ from expected ($exp)\n";

        foreach(array_keys($rowVals) as $key)
            if(!array_key_exists($key, $fetched))
                return "Array key $key does not exist in fetched array";

        foreach($fetched as $colIdx => $val) {
            if($val !== ($exp = $rowVals[$colIdx]))
                return "Column index $colIdx: Fetched value ($val) differ from expected ($exp)";
        }
        
        return null;

    } catch (PDOException $e) {
        return $e->getMessage();
    }
}

?>

--EXPECT--
Testing table integer... OK
Testing table floating_point... OK
Testing table string... OK
Testing table national_string... OK
Testing table lob... OK
Testing table binary... OK
Testing table datetime... OK
Testing table boolean... OK
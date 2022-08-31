--TEST--
PDO Mimer(stmt-fetch): Using PDO::FETCH_BOTH

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Verifies that:
1. fetch() returns an array of values indexed by both
    column name and column index
2. that the values are as expected (same as test data)

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_allTypes");
$tables = $util->getTablesExcept(["lob"]);
$util->testEachTable($tables, 'test');

function test($table, $dsn): ?string {
    try {
        $db = new PDO($dsn);
        $rowVals_assoc = $table->getRow(0);
        $rowVals_idx = array_values($rowVals_assoc);
        $rowVals = array_merge($rowVals_assoc, $rowVals_idx);
        $tblName = $table->getName();
        $stmt = $db->query("SELECT * FROM $tblName WHERE id = 1");
        $fetched = $stmt->fetch(PDO::FETCH_BOTH);

        if(($act = count($fetched)) !== ($exp = count($rowVals)))
            return "Number of elements in result set ($act) differ from expected ($exp)\n";

        foreach(array_keys($rowVals) as $key)
            if(!array_key_exists($key, $fetched))
                return "Array key $key does not exist in fetched array";

        foreach($fetched as $key => $val) {
            if($val !== ($exp = $rowVals[$key]))
                return "Column key $key: Fetched value ($val) differ from expected ($exp)";
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
Testing table binary... OK
Testing table datetime... OK
Testing table boolean... OK
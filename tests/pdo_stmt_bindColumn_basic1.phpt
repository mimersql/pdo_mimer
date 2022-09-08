--TEST--
PDO Mimer(stmt-bindColumn): binding all data types

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Binds variables to columns in result set statements and 
verifies the values. Test is repeated for all data types 
except LOBs, who are handled differently. 

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
        $columns = $table->getAllColumns();
        $tblName = $table->getName();
        $sql_format = "SELECT %s FROM $tblName WHERE id = 1";
    
        // Fetching each column separately helps 
        // spotting a specific failing data type
        foreach ($columns as $colName => $col){
            $sql = sprintf($sql_format, $colName);
            $stmt = $db->prepare($sql);
            $pdoType = $col->getPDOType();
            $stmt->bindColumn(1, $res, $pdoType);
            
            if (!$stmt->execute())
                return "Column $colName: Could not execute statement: \"$sql\"";

            $stmt->fetch(PDO::FETCH_BOUND);
            
            if ($res !== ($exp = $table->getVal($colName, 0)))
                return "Column $colName: Fetched value ($res) differ " .
                    "from expected value ($exp)\n";
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
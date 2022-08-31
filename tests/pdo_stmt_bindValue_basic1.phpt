--TEST--
PDO Mimer(stmt-bindValue): binding all data types - positional placeholders

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Tests binding values to parameters to insert
data in DB, for every target column type except LOBs.

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
        $columns = $table->getColumnsExcept(['id']);
        $tblName = $table->getName();
        $id = $table->getNextID();

        $sql_format_in = "INSERT INTO $tblName(id, %s) VALUES (%d, ?)";
        $sql_format_out = "SELECT %s FROM $tblName WHERE id = %d";
        
        // One insert statement for every column
        foreach ($columns as $colName => $col){
            $sql_in = sprintf($sql_format_in, $colName, $id);
            $sql_out = sprintf($sql_format_out, $colName, $id++);

            // insert value in DB using bindParam
            $stmt = $db->prepare($sql_in);
            $pdoType = $col->getPDOType();
            $inVal = $table->getVal($colName, 0);
            $stmt->bindValue(1, $inVal, $pdoType);
            
            if (!$stmt->execute())
                return "Column $colName: Could not execute statement: \"$sql_in\"";

            // verify
            $stmt = $db->query($sql_out);
            $stmt->bindColumn(1, $outVar, $pdoType);
            $stmt->fetch(PDO::FETCH_BOUND);
            
            if ($inVal !== $outVar)
                return "Column $colName: Fetched value ($outVar) differ " . 
                        "from inserted value ($inVal)\n";
            
            $outVar = null; // keeping value gives confusing error message when fetch() fails
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
Testing table interval... OK
Testing table boolean... OK
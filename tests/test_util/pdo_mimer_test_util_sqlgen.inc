<?php
/**
 * Responsible for creating SQL scripts from the DB objects.
 */
class PDOMimerTestSQLGenerator {

     /**
     * Prints the SQL statements which a given preset would produce.
     */
    public static function printPreset(PDOMimerTestPreset $preset): void {
        $all_stmts = array_merge(self::getPresetDDL($preset), 
            self::getPresetDML($preset));
        foreach ($all_stmts as $stmt)
            print($stmt . "\n");
    }

    /**
     * Returns all the DDL statements for the preset as an
     * array of strings.
     */
    public static function getPresetDDL(PDOMimerTestPreset $testdb): array {
        $ddl = [];

        foreach($testdb->getTables() as $table)
            $ddl[] = self::getTableDDL($table);

        foreach ($testdb->getStoredProcedures() as $procedure) {
            if (PDOMimerTestDBInfo::objectExists("PROCEDURE", $procedure->getName()))
                $ddl[] = $procedure->drop();
            $ddl[] = $procedure->create();
        }

        return $ddl;
    } 

    /**
     * Returns all the DML statements for the preset as an
     * array of strings.
     */
    public static function getPresetDML(PDOMimerTestPreset $testdb): array {
        $dml = [];
        foreach($testdb->getTables() as $table){
            $dml = array_merge($dml, self::getTableDML($table));
        }
        return $dml;
    } 

    /**
     * Generates strings on the form: 
     * 'CREATE TABLE example (id INTEGER NOT NULL PRIMARY KEY, name VARCHAR(20)) IN data_bank_name'
     */
    private static function getTableDDL(PDOMimerTestTable $tbl): string{
        $databank = PDOMimerTestConfig::getDatabank();
        $sql = "CREATE TABLE ";
        $sql .= $tbl->getName() . " (";
        foreach($tbl->getAllColumns() as $col){
            $sql .= self::getColumnDDL($col) . ", ";
        }
        $sql = substr($sql, 0, -2); // remove last ", "
        $sql .= ") IN $databank";
        return $sql;
    }

    /**
     * Generates strings on the form:
     * 'id INTEGER NOT NULL PRIMARY KEY'
     */
    private static function getColumnDDL(PDOMimerTestColumn $col): string {
        $sql = $col->getName() . " ";
        $sql .= PDOMimerDataTypeMap::getMimerTypeName($col->getType());

        if (($prec = $col->getPrecision()) !== null)
            $sql .= "(" . implode(",", $prec) . ")";
        if ($col->isPrimary())
            $sql .= " PRIMARY KEY";
        if ($col->isNotNull())
            $sql .= " NOT NULL";
        if ($col->isUnique())
            $sql .= " UNIQUE";
        if (($def = $col->getDefault()) !== false){
            $sql .= " DEFAULT ";
            if ($def === null)
                $sql .= " NULL";
            else
                $sql .= " " . self::$formatVal4SQL($col->getType(), $def);
        }
        return $sql;
    }

    /**
     * Generates an array of strings on the form: 
     * "INSERT INTO example (name) VALUES ('Alice')"
     * in order to fill the table with the test data.
     * 
     */
    public static function getTableDML(PDOMimerTestTable $tbl): array {
        $statements = [];
        $columns = $tbl->getAllColumns();
        $rows = $tbl->getRows();
        
        foreach($rows as $row){
            $sql = "INSERT INTO ";
            $sql .= $tbl->getName() . " VALUES(";
            foreach($columns as $colname => $col)
                $sql .= self::formatVal4SQL($col->getType(), $row[$colname]) . ", ";
            
            $sql = substr($sql, 0, -2); // remove last ", "
            $sql .= ")";
            $statements[] = $sql;
        }
        return $statements;
    }

    /**
     * Some values need to have a certain format when used in 
     * SQL strings. For example, when inserting blob literals,
     * one must use the format "x'F0122321'". This function takes
     * a value and converts it to a string of the correct format 
     * for that Mimer type.
     */
    public static function formatVal4SQL(PDOMimerDataType $type, $val): string{
        switch ($type) {
            case PDOMimerDataType::BINARY:
            case PDOMimerDataType::VARBINARY:
            case PDOMimerDataType::BLOB:
                $hexstr = bin2hex($val);
                return "x'$hexstr'";
            
            case PDOMimerDataType::DATE:
                return "DATE'$val'";
            
            case PDOMimerDataType::TIME:
                return "TIME'$val'";

            case PDOMimerDataType::TIMESTAMP:
                return "TIMESTAMP'$val'";
            
            case PDOMimerDataType::INTERVAL:
                return "INTERVAL'$val' HOUR TO MINUTE";

            case PDOMimerDataType::SMALLINT:
            case PDOMimerDataType::INT:
            case PDOMimerDataType::BIGINT:
            case PDOMimerDataType::REAL:
            case PDOMimerDataType::DEC:
            case PDOMimerDataType::DOUBLE_PRECISION:
            case PDOMimerDataType::FLOAT:
                return strval($val);

            case PDOMimerDataType::CHAR:
            case PDOMimerDataType::VARCHAR:
            case PDOMimerDataType::CLOB:
            case PDOMimerDataType::NCHAR:
            case PDOMimerDataType::NVARCHAR:
            case PDOMimerDataType::NCLOB:
                return "'$val'";

            case PDOMimerDataType::BOOLEAN:
                return $val ? "TRUE" : "FALSE";

            default:
                die("Cannot format value of unknown type");
        }
    }
}
?>
<?php
require_once("pdo_mimer_test_util_types.inc");

/**
 * Represents a table in the database.
 */
class PDOMimerTestTable {
    private string $name;
    private array $columns = []; // Associative, keys = column names

    function __construct(string $name){
        $this->name = $name;
    }

    public function addColumn(PDOMimerTestColumn $col): void {
        $colname = $col->getName();
        if (array_key_exists($colname, $this->columns))
            die("Could not add column $colname to table $this->name: Column name already exists");
        $this->columns[$col->getName()] = $col;
    }

    /**
     * Adds the values to the columns in the table. 
     * 
     * Expects a multi-dimensional array on the form:
     * [
     *  [col1_name,     col2_name],
     *  [row1_col1_val, row1_col2_val], 
     *  [row2_col1_val, row2_col2_val]
     *              ...
     * ]
     * 
     * See the PDOMimerTestCatalog make-functions for examples.
     */
    public function addValues(array $values){
        foreach($values as $i => $row){
            if ($i === 0) continue; // first row contains headers
            if(count($row) !== count($this->columns))
                die("Could not add values to table $this->name: " .
                    "num. of columns in row $i of value matrix " .
                    "do not match num. of columns in table");
            
            foreach($row as $colidx => $cellval){
                $colname = $values[0][$colidx];
                $this->columns[$colname]->addValue($cellval);
            }
        }
    }

    public function getName(): string {
        return $this->name;
    }

    /**
     * Assumes each column has same number of values. 
     * Returns number of values in first column.
     */
    public function numberOfRows(): int {
        $firstKey = array_key_first($this->columns);
        return count($this->columns[$firstKey]->getValues());
    }

    public function numberOfColumns(): int {
        return count($this->columns);
    }

    /**
     * Get column either by name or by index.
     */
    public function getVal(string|int $col, int $rowNum): mixed {
        if (gettype($col === "string"))
            return $this->columns[$col]->getValues()[$rowNum];
        else {
            return $this->getAllColumns(false)[$col];
        } 
    }

    /**
     * Returns the columns either as an associative array 
     * indexed by column name, or as a numbered array.
     */
    public function getAllColumns(bool $associative = true): array {
        if ($associative)
            return $this->columns;
        else 
            return array_values($this->columns);
    }

    /**
     * Returns all columns except those mentioned by name
     * in argument array.
     */
    public function getColumnsExcept(array $excludes, bool $associative = true): array{
        $cols = $this->getAllColumns($associative);
        foreach($excludes as $ex)
            unset($cols[$ex]);
        return $cols;
    }   

    public function getColumn(string $colName): PDOMimerTestColumn {
        return $this->columns[$colName];
    }

    public function getColumnIndex(string $colName): int {
        return array_search($colName, array_keys($this->getAllColumns()));
    }

    /**
     * Returns the name of the column with the given index.
     */
    public function getColumnName(int $idx): string {
        $cols = $this->getAllColumns(false);
        return $cols[$idx]->getName();
    }

    /**
     * Returns all the names of the columns in a table
     * as an array of strings.
     */
    public function getColumnNames(): array {
        $names = [];
        foreach($this->columns as $col)
            $names[] = $col->getName();
        return $names;
    }

    /**
     * Returns the values currently in the table object
     * as rows, i.e. an array of arrays where each outmost
     * element is an array with the values of one single row.
     * The inner array is indexed by the column names.
     */
    public function getRows(): array {
        $tblVals = [];
        foreach($this->columns as $col){
            $colname = $col->getName();
            foreach($col->getValues() as $i => $val)
                $tblVals[$i][$colname] = $val;
        }
        return $tblVals;
    }

    /**
     * Convenience function to get a single row.
     */
    public function getRow(int $rowNum): array {
        $rows = $this->getRows();
        return $rows[$rowNum];
    }

    /**
     * Returns an SQL string for inserting values into all 
     * columns of the table. The string can be formatted for 
     * either positional or named parameters. 
     */
    public function getInsertSQL(bool $positional = true): string {
        $sql = "INSERT INTO " . $this->getName() . " VALUES(";
        foreach($this->getColumnNames() as $colName)
            $sql .= $positional ? "?, " : ":$colName, ";
        $sql = substr($sql, 0, -2); // remove last ", "
        $sql .= ")";
        return $sql;
    }

    /**
     * Given that table has a primary key column called 'id',
     * returns the next id, i.e. an id not yet in table.
     */
    public function getNextID(): int {
        if (!array_key_exists("id", $this->getAllColumns()))
            die("Table " . $this->getName() . "does not have id column");
        
        return $this->getVal("id", $this->numberOfRows()-1) + 1;
    }

    /**
     * Given an executed statement which has a result set, this function 
     * compares the content of the result set to the values of the table. 
     * Expects that the result set is complete, i.e. all data from table has been
     * selected. Dies with error message if the values differ.
     */
    public function verifyResultSet(PDOStatement $stmt): void {
        $rows = $this->getRows();
        $cols = $this->getAllColumns();

        foreach($rows as $i => $row){
            $res = $stmt->fetch(PDO::FETCH_ASSOC);
            foreach($row as $colName => $expVal){
                $fetched = $res[$colName];
                if ($fetched !== $expVal)
                    die("Column $colName: Expected value ($expVal) differ from " . 
                        "fetched value ($fetched)\n");
            }
        }
    }
}

enum PDOMimerParameterType {
    case IN;
    case OUT;
    case INOUT;
}

class PDOMimerTestStoredProcedureParameter {
    private string $name;
    private PDOMimerParameterType $parameterType;
    private PDOMimerDataType $dataType;

    public function __construct(PDOMimerParameterType $parameter, string $name, PDOMimerDataType $type) {
        $this->name = $name;
        $this->parameterType = $parameter;
        $this->dataType = $type;
    }

    /**
     * @return string
     */
    public function getName(): string
    {
        return $this->name;
    }

    /**
     * @return PDOMimerParameterType
     */
    public function getParameterType(): PDOMimerParameterType
    {
        return $this->parameterType;
    }

    /**
     * @return PDOMimerDataType
     */
    public function getDataType(): PDOMimerDataType
    {
        return $this->dataType;
    }

    public function __toString(): string
    {
        return $this->name;
    }

    public function asParam(): string {
        return $this->parameterType->name . " $this " . $this->dataType->name;
    }
}

class PDOMimerTestStoredProcedure {

    private string $name;
    /**
     * @var PDOMimerTestStoredProcedureParameter[]
     */
    private array $parameters = [];
    private string $function;
    private bool $contains_sql;

    public function __construct(string $name, array $parameters, string $function, bool $contains_sql = false) {
        $this->name = $name;
        $this->parameters = $parameters;
        $this->function = $function;
        $this->contains_sql = $contains_sql;
    }

    /**
     * @return string
     */
    public function getName(): string
    {
        return $this->name;
    }

    /**
     * @return array
     */
    public function getParameters(): array
    {
        return $this->parameters;
    }

    /**
     * @return string
     */
    public function getFunction(): string
    {
        return $this->function;
    }

    /**
     * @return string[]
     */
    public function getParamCreation(): array {
        return array_map(function(PDOMimerTestStoredProcedureParameter $param): string {
            return $param->asParam();
        }, $this->parameters);
    }

    public function create(): string {
        return "CREATE PROCEDURE $this->name (" . implode(", ", $this->getParamCreation())
            . ") " . ($this->contains_sql ? "CONTAINS SQL ": '') . "BEGIN $this->function END;";
    }

    public function drop(): string {
        $params = array_map(function (PDOMimerTestStoredProcedureParameter $param): string {
            return $param->getDataType()->name;
        }, $this->parameters);

        return "DROP PROCEDURE $this->name(" . implode(', ', $params) . ");";
    }
}

/**
 * Represents a column in a database table.
 */
class PDOMimerTestColumn {
    private string $name;
    private PDOMimerDataType $type;
    private ?array $precision; // array because DEC takes 2 values
    private bool $notNull;
    private bool $unique;
    private bool $primaryKey;
    private mixed $default;
    private array $values = [];

    function __construct(string $name, 
                        PDOMimerDataType $type, 
                        bool $primKey = false,
                        ?array $precision = null, 
                        bool $notNull = false,
                        bool $unique = false,
                        $default = false) {
        $this->name = $name;
        $this->type = $type;
        $this->precision = $precision;
        $this->notNull = $notNull;
        $this->unique = $unique;
        $this->primaryKey = $primKey;
        $this->default = $default;

        if($default && !$this->validatePHPVal($default))
            die("Incorrect value type for default value in column $this->name");
    }

    /**
     * Adds one value to the column. 
     * Must be of the expected PHP data type.
     */
    public function addValue(mixed $val): void {
        if(!$this->validatePHPVal($val))
            die("Could not insert value in column $this->name: " .
                "Value has wrong type");
        
        $this->values[] = $val;
    }

    public function getValues(): array {
        return $this->values;
    }

    public function getPrecision(): ?array{
        return $this->precision;
    }

    public function isPrimary(): bool {
        return $this->primaryKey;
    }

    public function isUnique(): bool {
        return $this->unique;
    }

    public function isNotNull(): bool {
        return $this->notNull;
    }

    public function getName(): string {
        return $this->name;
    }

    public function getType(): PDOMimerDataType {
        return $this->type;
    }

    public function getMimerType(): string {
        return PDOMimerDataTypeMap::getMimerTypeName($this->type);
    }

    public function getPDOType(){
        return PDOMimerDataTypeMap::getPDOParamConst($this->getType());
    }

    public function getDefault(): mixed {
        return $this->default;
    }

    /**
     * Returns true of the given value has a type that is
     * compatible with the column type.
     * 
     * TODO: enforce other string formats, TIME etc.?
     */
    private function validatePHPVal($val): bool {
        if (!in_array(gettype($val), PDOMimerDataTypeMap::getPHPTypeName($this->type)))
            return false;
        if ($this->type === PDOMimerDataType::DATE) // enforces date string format
            return preg_match("/^[0-9]{4}-(0?[1-9]|1[012])-([012]?[1-9]|3[01])$/", $val);
        
        return true;
    }
}
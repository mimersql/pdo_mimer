<?php require 'config.inc';

const NAME       = 'name';
const TABLE      = 'table';
const ROWS       = 'rows';
const COLUMNS    = 'columns';
const COLUMN     = 'column';
const DEFINITION = 'definition';
const TYPE       = 'type';
const PDO_TYPE   = 'pdo_type';
const NOT_NULL   = 'not_null';
const KEY        = 'key';
const PRIMARY    = 'PRIMARY KEY';
const UNIQUE     = 'UNIQUE';
const _DEFAULT   = 'default';
const DATA       = 'data';
const PARAM      = 'param';
const VALUES     = 'values';
const NAMED      = 'named';
const VALUE      = 'value';
const TEST_TABLE = 'test_table';

abstract class Struct extends ArrayObject {
    protected function __construct(array $array) {
        parent::__construct($array, /*ArrayIterator::ARRAY_AS_PROPS*/);
    }
    abstract function definition(): string;
    public function extract(): array {
        return (array)$this;
    }
    final protected function hasKey(string ...$keys): bool {
        foreach ($keys as $key)
            if (!array_key_exists($key, (array)$this))
                return false;
        return true;
    }
    final protected function getValue(string ...$keys) {
        $current = $this;
        foreach ($keys as $key) {
            if (!array_key_exists($key, (array)$current))
                return null;
            $current = $current[$key];
        }
        return $current;
    }
    final protected function getValues(string ...$keys): array {
        $array = [];
        foreach ($keys as $key) {
            if ($this->hasKey($key))
                $array += [$key => $this[$key]];
        }
        return $array;
    }
    final protected function getValuesExcept(string ...$keys): array {
        $values = [];
        foreach ($this as $key => $value)
            if (!in_array($key, $keys))
                $values += [$key => $value];

        return $values;
    }
    final protected function getAll(): array {
        $array = [];
        foreach ((array)$this as $key => $value)
            array_push($array, $key, $value);
        return $array;
    }
    public function __get($key) {
        switch ($key) {
            case DEFINITION:
                return $this->definition();
            default:
                return $this->getValue($key);
        }
    }
}

class Definition extends Struct {
    public function __construct($type_or_array, bool $not_null = null, string $key = null, $default = null) {
        $type = $type_or_array;
        if ($is_array = is_array($type_or_array))
            extract($type_or_array);

        $array = [TYPE => $type];
        if (null !== $not_null)
            $array += [NOT_NULL => $not_null];
        if (null !== $key)
            $array += [KEY => $key];

        if (4 <= func_num_args() || ($is_array && array_key_exists(_DEFAULT, $type_or_array))) {
            if (is_string($default))
                $default = "'$default'";
            if (is_null($default))
                $default = 'NULL';
            $array += [_DEFAULT => $default];
        }

        parent::__construct($array);
    }
    public function definition(): string {
        $definition = $this->type;
        if ($this->not_null)
            $definition .= ' NOT NULL';
        if ($this->key)
            $definition .= " $this->key";
        if ($this->hasKey(_DEFAULT))
            $definition .= " DEFAULT $this->default";
        return $definition;
    }
    public function __toString(): string {
        return $this->definition();
    }
}
class Column extends Struct {
    public function __construct($name_or_array, $definition = null, array $data = null) {
        $name = $name_or_array;
        if (is_array($name_or_array)) {
            $name = $name_or_array[NAME];
            if (is_array($definition = $name_or_array[DEFINITION]))
                $definition = new Definition($definition);
            if (array_key_exists(DATA, $name_or_array) and is_array($name_or_array[DATA]))
                $data = $name_or_array[DATA];
        }

        $array = [NAME => $name, DEFINITION => is_array($definition) ? new Definition($definition) : $definition];
        if ($data) {
            if (!array_key_exists(PARAM, $data))
                $data[PARAM] = ":$name";
            
            $array += [DATA => $data];
        }
        parent::__construct($array);
    }
    public function definition(): string {
        return (string)$this[DEFINITION];
    }
    public function extract(): array {
        $array = [COLUMN => $this] + $this->getValuesExcept(NAME);
        foreach ($this->getValue(DEFINITION) as $key => $value)
            $array[$key] = $value;
        foreach ($this->getValue(DATA)as $key => $value) {
            if ($key === TYPE)
                $key = PDO_TYPE;
            $array[$key] = $value;
        }
        return $array;
    }
    public function value(int $row) {
        if ($value = $this->getValue(DATA, VALUES))
            return $value[$row-1];
        return null;
    }
    public function __get($key) {
        switch ($key) {
            case TYPE:
                return $this->getValue(DEFINITION, TYPE);
            case PDO_TYPE:
                return $this->getValue(DATA, TYPE) ?: $this->getValue(DATA, PDO_TYPE);
            case VALUES:
                return $this->getValue(DATA, VALUES);
            case PARAM:
                return $this->getValue(DATA, PARAM) ?: ":$this";
            default:
                return parent::__get($key);
        }
    }
    public function __toString(): string {
        return $this[NAME];
    }
}
class Table extends Struct {
    public function __construct($name_or_array, array $columns = null, int $rows = null) {
        $name = $name_or_array;
        if (is_array($name_or_array)) {
            extract($name_or_array);
            if (isset($table))
                $name = $table;
        }

        foreach ($columns as $i => $column)
            if (is_array($column))
                $columns[$i] = new Column($column);

        if (null === $rows and ($data = $columns[0]->data))
            $rows = count($columns[0]->data[VALUES]);

        parent::__construct([NAME => $name, COLUMNS => $columns, ROWS => $rows]);
    }
    public function definition(): string {
        return implode(',', array_map(fn(Column $column): string => "$column $column->definition", $this[COLUMNS]));
    }
    public function array_column(string $key): array {
        $array_column = [];
        foreach ($this[COLUMNS] as $column)
            $array_column[] = $column[$key];
        return $array_column;
    }
    public function extract(): array {
        $array = [TABLE => $this] + $this->getValuesExcept(NAME);
        foreach ($this[COLUMNS] as $column)
            $array[$column[NAME]] = $column;
        return $array;
    }
    public function __toString(): string {
        return $this[NAME];
    }
}

class PDOMimerTest extends PDO {
    private static Table $table;
    public function __construct(?bool $create_and_insert = true, bool $drop_tables = true) {
        $dsn    = PDO_MIMER_TEST_DSN;
        $user   = PDO_MIMER_TEST_USER;
        $pass   = PDO_MIMER_TEST_PASS;
        $attr   = unserialize(PDO_MIMER_TEST_ATTR ?: '') ?: null;
        parent::__construct($dsn, $user, $pass, $attr);

        if (null !== $create_and_insert) {
            $this->dropTables();
            $this->createTables();
            if ($create_and_insert)
                $this->insertData();
        } else if ($drop_tables) {
            $this->dropTables();
        }
    }
    public static function getDBName(): string {
        $dbname_str = "dbname=";
        $dbname_len = strlen($dbname_str);

        if (false === ($dbname_pos = stripos(PDO_MIMER_TEST_DSN, $dbname_str)))
            return '';

        $dbname_end_pos = strpos(PDO_MIMER_TEST_DSN, ';', $dbname_pos + $dbname_len);
        if (false === $dbname_end_pos)
            return substr(PDO_MIMER_TEST_DSN, $dbname_pos + $dbname_len);
        return substr(PDO_MIMER_TEST_DSN, $dbname_pos + $dbname_len, $dbname_end_pos);
    }
    public static function makeExTableStd(): void {
        self::$table = new Table([
             NAME    => 'pdo_mimer_test_std',
             ROWS    => 5,
             COLUMNS => [[
                 NAME       => 'id',
                 DEFINITION => [
                     TYPE     => 'integer',
                     NOT_NULL => true,
                     KEY      => PRIMARY,
                 ], DATA => [
                 TYPE   => PDO::PARAM_INT,
                 VALUES => [1, 2, 3, 4, 5,],
             ],],[
                 NAME       => 'val',
                 DEFINITION => [
                     TYPE     => 'varchar(20)',
                     NOT_NULL => true,
                 ], DATA     => [
                 TYPE   => PDO::PARAM_STR,
                 VALUES => ['A', 'B', 'C', 'D', 'E',],
             ],]
            ],
        ]);
    }
    public static function makeExTablePerson(): void {
        self::$table = new Table([
             NAME    => 'pdo_mimer_test_person',
             ROWS    => 3,
             COLUMNS => [[
                 NAME       => 'id',
                 DEFINITION => [
                     TYPE     => 'integer',
                     NOT_NULL => true,
                     KEY      => PRIMARY,
                 ], DATA => [
                 TYPE   => PDO::PARAM_INT,
                 VALUES => [1, 2, 3,],
             ],],[
                 NAME       => 'name',
                 DEFINITION => [
                     TYPE     => 'varchar(20)',
                     NOT_NULL => true,
                 ], DATA     => [
                 TYPE   => PDO::PARAM_STR,
                 VALUES => ['Alice', 'Bob', 'Clarice',],
             ],]
            ],
        ]);
    }
    public static function error(PDOException $e): string {
        if (2 >= count($e->errorInfo)) {
            print $msg = $e->getMessage();
            return $msg;
        }

        [, $code, $err_msg] = $e->errorInfo;
        if (0 <= $code) {
            $substr = substr($e->getMessage(), strlen("SQLSTATE[HYXXX][-"));
            $code = (int)substr($substr, 0, strpos($substr, ']'));
        }
        print $msg = "[$code]: $err_msg";
        return $msg;
    }
    public static function table(): Table {
        return self::$table;
    }
    public function hasTable($table): bool {
        $name = $table[NAME];
        if ($this->query("SELECT * FROM $name FETCH 1"))
            return true;
        return false;
    }
    public function createTables(...$tables): self {
        if (!$tables)
            $tables = [self::$table];

        foreach ($tables as $table) {
            $this->dropTables($table);
            $this->exec("CREATE TABLE $table ($table->definition)");
        }
        return $this;
    }
    public function dropTables(...$tables): int {
        if (!$tables)
            $tables = [self::$table];

        $this->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_SILENT);

        $num_dropped_tables = 0;
        foreach ($tables as $table) {
            if (is_array($table))
                $table = $table[NAME];
            if ($this->hasTable($table)) {
                $this->exec("DROP TABLE $table");
                $num_dropped_tables++;
            }
        }

        $this->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        return $num_dropped_tables;
    }
    public function insertData($table = null, $columns = null): void {
        if (null === $table)
            $table = self::$table;
        else if (is_array($table))
            $table = array_key_exists(NAME, $table) ? $table[NAME] : $table[TABLE];
        if (null === $columns)
            $columns = $table[COLUMNS];

        $columns = array_map(function($col): array {
            $name = '';
            extract((array)$col);
            if (isset($column))
                $name = $column;
            if (isset($data)) {
                $type = '';
                extract($data);
                if (isset($pdo_type))
                    $type = $pdo_type;
                foreach ($values as &$val){
                    if (is_string($val))
                        $val = "'$val'";
                }
                return [NAME => $name, TYPE => $type, VALUES => $values];
            }
            return $column;
        }, (array)$columns);
        $array_column = function(string $key) use ($columns): array {
            $result = [];
            foreach ($columns as $column)
                $result[] = $column[$key];
            return $result;
        };
        $names = implode(',',$array_column(NAME));
        $values_column = array_map(null, ...$array_column(VALUES));
        foreach ($values_column as $values) {
            $values = implode(',',$values);
            $this->exec("INSERT INTO $table ($names) VALUES ($values)");
        }
    }
    public function bindValue(PDOStatement $stmt, Column $column, int $row): bool {
        return $stmt->bindValue($column->param, $column->value($row), $column->pdo_type);
    }
    public function silent(callable $function, array $params): void {
        $this->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_SILENT);
        $this->$function(...$params);
        $this->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_SILENT);
    }
    public static function extract(int $column = 0): array {
        return self::$table->extract() + self::$table->columns[$column]->extract();
    }
    public static function limitMemory(): int {
        $tot_mem_usage = memory_get_usage(true);
        ini_set('memory_limit',$tot_mem_usage);
        $used_mem_usage = memory_get_usage();
        return $tot_mem_usage - $used_mem_usage;
    }
    public static function createFile(int $req_repetitions, $contents, int $length = null) {
        $fp = tmpfile();
        for ($i = 0; $i < $req_repetitions; $i++)
            fwrite($fp, $contents, $length);
        rewind($fp);
        return $fp;
    }
    public function __get($name) {
        switch ($name) {
            case 'extract':
                return self::$table->extract();
            case 'table':
                return self::$table;
            default:
                return null;
        }
    }
    public static function skipif(bool $failed, string $msg = ''): void {
        if (!$failed)
            return;

        if (!$msg)
            if (PDO_MIMER_TEST_SKIP_OSUSER) $msg = 'os_user';

        die('skip ' . ($msg ?: 'no reason given'));
    }
    public static function skip(): void {
        try {
            new self(false, false);
        } catch (PDOException $e) {
            die("skip $e");
        }
    }

}
PDOMimerTest::makeExTableStd();
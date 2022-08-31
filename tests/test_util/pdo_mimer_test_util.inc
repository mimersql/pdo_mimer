<?php
require_once("pdo_mimer_test_util_conf.inc");
require_once("pdo_mimer_test_util_catalog.inc");
require_once("pdo_mimer_test_util_setup.inc");
require_once("pdo_mimer_test_util_db_info.inc");

/**
 * Tester's interface to the testing utilities.
 *  
 * Provides the tester with convenience functions for
 * creating and controlling the test environment, as well
 * as comparing test output with values in the example 
 * environment.
 */
class PDOMimerTestUtil {
    private ?PDOMimerTestPreset $selected = null;     // currently selected test DB preset

    /**
     * Util objects can be created with a preset name in order for the preset
     * to be set up in one line of code. Presets can also be chosen and executed 
     * at a later stage using executePreset(). 
     */
    function __construct(?string $presetName = null, bool $executeDML = true) {
        if($presetName)
            $this->executePreset($presetName, $executeDML);
    } 

    /**
     * Prints the SQL statements which are generated for a 
     * given preset.
     */
    public function printPreset(string $presetName): void {
        $preset = PDOMimerTestCatalog::getPreset($presetName);
        PDOMimerTestSQLGenerator::printPreset($preset);
    }

    /**
     * Returns a table in the currently selected preset.
     */
    public function getTable(string $table): PDOMimerTestTable {
        return $this->selected->getTable($table);
    }

    /**
     * Returns all the tables of the currently selected 
     * preset except those specified in the argument,
     * which is an array of strings.
     */
    public function getTablesExcept(array $excludes): array {
        $allTables = $this->selected->getTables();
        foreach($excludes as $ex)
            unset($allTables[$ex]);
        return $allTables;
    }

    /**
     * Returns the value for a field in the test configuration file.
     * To access nested fields, seperate the keys with a '->'.
     * For example: to get the password, use 'connection->dsn->password'. 
     * To get the entire DSN as an assoc. array, use 'connection->dsn'.
     */
    public function getConfigValue(string $key): mixed {
        return PDOMimerTestConfig::getPropertyValue($key);
    }

    /**
     * Returns a DSN string with every property in the connection->dsn 
     * object in the test configuration file. 
     * Example: 'mimer:dbname=test_db;user=sysadm;password=sysadm;'
     */
    public function getFullDSN(): string {
        return PDOMimerTestConfig::getDSN();
    }

    /**
     * Returns an array of the DML statements 
     * for a certain table in the currently selected preset. 
     */
    public function getTableDML(string $tblName): array {
        $tbl = $this->selected->getTable($tblName);
        $stmts = PDOMimerTestSQLGenerator::getTableDML($tbl);
        return $stmts;
    } 

    /**
     * Returns the name for the OS user that is running the 
     * PHP script. 
     * TODO: Not tested on windows yet.
     */
    public function getCurrentOsUser(): string {
        if (PHP_OS_FAMILY === "Windows") {
            return get_current_user();
        } elseif (PHP_OS_FAMILY === "Linux") {
            return exec("whoami");
        } else {
            die("Error getting OS user: Could not determine OS\n");
        }
    }

    /**
     * Returns the next ID of the given table in the 
     * currently selected preset.
     */
    public function getNextTableID(string $tblName): int {
        $tbl = $this->selected->getTable($tblName);
        return $tbl->getNextID();
    }

    /**
     * For checking object existance in DB. Just wraps the method 
     * from the DBInfo class to keep interface non-fragmented.
     */
    public function objectExists(string $objectType, string $objectId) {
        return PDOMimerTestDBInfo::objectExists($objectType, $objectId);
    } 

    /**
     * Selects and sets up preset.
     * @param executeDML controls whether to run the DML statements.
     * If false, DML statements can be run later using executeDML().
     */
    public function executePreset(string $dbname, bool $executeDML = true){
        $this->selected = PDOMimerTestCatalog::getPreset($dbname);
        PDOMimerTestSetup::resetDatabank();
        PDOMimerTestSetup::executeDDL($this->selected);

        if($executeDML)
            PDOMimerTestSetup::executeDML($this->selected);
    } 

    /**
     * Runs all the DML statements for the preset.
     * Useful when data should be inserted later than DB creation.   
     */
    public function executeDML(){
        if (!$this->selected)
            die("Could not execute DML statements: no preset currently selected");
        PDOMimerTestSetup::executeDML($this->selected);
    }

    /**
     * Convenience function for skip checks, mostly just improves 
     * readability. Should only be called in --SKIPIF-- section.
     */
    public static function skipIf(bool $condition, string $skipMsg) {
        if($condition)
            die("skip " . $skipMsg);
    }

    /**
     * Gathers all the general checks that determines whether a 
     * test should be skipped or not. Called in the --SKIPIF-- 
     * section of every test.
     */
    public static function commonSkipChecks(): void {
        // TODO: 
        // $this->skipIf(...);
        return;
    }

    /**
     * Locks the PHP process's memory limit and 
     * returns the amount of memory (in bytes) still
     * available before reaching that limit.
     */
    public static function limitMemory(): int {
        $tot_mem_usage = memory_get_usage(true);
        ini_set('memory_limit',$tot_mem_usage);
        $used_mem_usage = memory_get_usage();
        return $tot_mem_usage - $used_mem_usage;
    }

    /**
     * Creates a file with the given amount of repetitions 
     * of the given string and returns a file pointer.
     */
    public static function createFile(int $req_reps, string $content) {
        $fp = tmpfile();
        $strBytes = strlen($content);
        for ($i = 0; $i < $req_reps; $i++)
            fwrite($fp, $content, $strBytes);
        rewind($fp);
        return $fp;
    }

    /**
     * For running a test repeatedly for different tables.
     * Prints test results of each table.
     * 
     * NOTE: 
     * -func must have the following signature:
     *      function test($table, $dsn): ?string
     * -func must return error message string upon failure,
     *      and return null on success.
     */
    public function testEachTable($tables, $func){ 
        $dsn = $this->getFullDSN();
        foreach($tables as $tbl){
            echo "Testing table " . $tbl->getName() . "... ";
            if ($msg = $func($tbl, $dsn)){
                echo "FAILURE\n";
                echo "\t" . $msg . "\n";
            } else
                echo "OK\n";
        }
    }    
}



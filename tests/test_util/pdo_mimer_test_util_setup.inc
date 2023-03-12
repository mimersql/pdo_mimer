<?php
require_once("pdo_mimer_test_util_sqlgen.inc");
/**
 * Responsible for setting up a given database.
 * Uses PDO to connect to DB and execute SQL statements.
 */
class PDOMimerTestSetup {

    public static function executeDDL(PDOMimerTestPreset $testdb){
        $script = PDOMimerTestSQLGenerator::getPresetDDL($testdb);
        self::executeStatements($script);
    }

    public static function executeDML(PDOMimerTestPreset $testdb){
        $script = PDOMimerTestSQLGenerator::getPresetDML($testdb);
        self::executeStatements($script);
    }

    public static function resetDatabank(): void {
        $db = new PDO(PDOMimerTestConfig::getDSN());
        if (self::testBankExists($db))
            self::dropTestBank($db);
        self::createTestBank($db);
    }

    public static function tearDown(): void {
        $db = new PDO(PDOMimerTestConfig::getDSN());
        if (self::testBankExists($db))
            self::dropTestBank($db);
    }

    /**
     * Takes an array of SQL statements and tries executing
     * them using PDO::exec()
     */
    private static function executeStatements(array $script): void {
        $db = new PDO(PDOMimerTestConfig::getDSN());

        // Execute generated script
        foreach($script as $statement){
            @$db->exec($statement);
        }
    }

    private static function testBankExists(PDO $conn): bool {
        $databank = PDOMimerTestConfig::getDatabank();
        $stmt = $conn->query("SELECT COUNT(*) FROM INFORMATION_SCHEMA.EXT_DATABANKS " . 
            "WHERE DATABANK_NAME = '$databank'");
        if (!$stmt)
            die("Error when checking test databank existance");
        $rowCnt = $stmt->fetch(PDO::FETCH_NUM)[0];
        return $rowCnt == 1;
    }

    private static function createTestBank(PDO $conn): void {
        $databank = PDOMimerTestConfig::getDatabank();
        if ($conn->exec("CREATE DATABANK $databank") === false)
            die("Could not create test databank");
    }

    private static function dropTestBank(PDO $conn): void {
        $databank = PDOMimerTestConfig::getDatabank();
        if ($conn->exec("DROP DATABANK $databank CASCADE") === false)
            die("Could not drop test databank. Other connection open?");
    }
}
?>
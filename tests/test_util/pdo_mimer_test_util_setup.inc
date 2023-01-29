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

    /**
     * Takes an array of SQL statements and tries executing
     * them using PDO::exec()
     */
    private static function executeStatements(array $script): void {
        $db = new PDO(PDOMimerTestConfig::getDSN());

        // Execute generated script
        foreach($script as $statement){
            try {
                if ($db->exec($statement) === false) // docs on PDO::exec suggest === here
                    die("Could not execute statement:\n'$statement'");
            } catch (PDOException $e){
//                die($e->getMessage());
                print $e->getMessage() . "\n";
            }
        }
    }

    private static function testBankExists(PDO $conn): bool {
        $bankName = PDOMimerTestConfig::getPropertyValue('databankName');
        $stmt = $conn->query("SELECT COUNT(*) FROM INFORMATION_SCHEMA.EXT_DATABANKS " . 
            "WHERE DATABANK_NAME = '$bankName'");
        if (!$stmt)
            die("Error when checking test databank existance");
        $rowCnt = $stmt->fetch(PDO::FETCH_NUM)[0];
        return $rowCnt == 1;
    }

    private static function createTestBank(PDO $conn): void {
        $bankName = PDOMimerTestConfig::getPropertyValue('databankName');
        if ($conn->exec("CREATE DATABANK $bankName") === false)
            die("Could not create test databank");
    }

    private static function dropTestBank(PDO $conn): void {
        $bankName = PDOMimerTestConfig::getPropertyValue('databankName');
        if ($conn->exec("DROP DATABANK $bankName CASCADE") === false)
            die("Could not drop test databank. Other connection open?");
    }
}
?>
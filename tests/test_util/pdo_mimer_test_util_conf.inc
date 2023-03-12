<?php
/**
*  Represents a test configuration. Acts as a mediator 
*  between the tests and the test configuration file.
*/
class PDOMimerTestConfig {
    private static string $dsn = "mimer:";
    private static string $user;
    private static string $pass;
    private static string $dbname = "";
    private static string $databank = "pdo_tests";

    /**
     * Finds, reads and converts the JSON file into an
     * associative array which is saved in $confDict.
     */
    public static function init()
    {
        if (false !== ($dsn = getenv('PDOMIMER_TEST_DSN'))) {
            if (str_contains($dsn, 'dbname=')) {
                $dbname = substr($dsn, strpos($dsn, 'dbname=') + strlen('dbname='));
                if (str_contains($dbname, ';'))
                    self::$dbname = substr($dbname, 0, strpos($dbname, ';'));

                self::$dbname = $dbname;
            }

            self::$dsn = $dsn;
        }

        if (false !== ($databank = getenv('PDOMIMER_TEST_DATABANK')))
            self::$databank = $databank;

        self::$user = getenv('PDOMIMER_TEST_USER');
        if (false !== self::$user and !str_contains(self::$dsn, 'user'))
            self::$dsn .= ";user=" . self::$user;

        self::$pass = getenv('PDOMIMER_TEST_PASS');
        if (false !== self::$pass and !str_contains(self::$dsn, 'password'))
            self::$dsn .= ";password=" . self::$pass;
    }

    /**
     * @return string
     */
    public static function getUser(): string
    {
        return self::$user;
    }

    /**
     * @return string
     */
    public static function getPassword(): string
    {
        return self::$pass;
    }

    /**
     * @return string
     */
    public static function getDBName(): string
    {
        return self::$dbname;
    }

    /**
     * @return string
     */
    public static function getDatabank(): string
    {
        return self::$databank;
    }



    /**
     * Returns a string based on the content of the DSN JSON object
     * on the form: 
     * mimer:<propertyname1>=<propertyvalue2>;<propertyname1>=<propertyvalue2>...;
     */
    public static function getDSN(): string {
        return self::$dsn;
    }
}

PDOMimerTestConfig::init();
?>
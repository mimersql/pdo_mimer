<?php
/**
*  Represents a test configuration. Acts as a mediator 
*  between the tests and the test configuration file.
*/
class PDOMimerTestConfig {
    private static string $confFileName = "test_config.json";
    private static string $filePath;
    private static $confDict;

    /**
     * Finds, reads and converts the JSON file into an
     * associative array which is saved in $confDict.
     */
    public static function init() {
        self::$filePath = dirname(__FILE__) . "/" .  self::$confFileName;

        if (!file_exists(self::$filePath))
            die("Could not find test configuration file\n"); 
            
        $file_as_str = file_get_contents(self::$filePath);
        if (!$file_as_str)
            die("Could not read test configuration file\n");

        self::$confDict = json_decode($file_as_str, true);
        if (self::$confDict === null)
            die("Test configuration file could not be interpreted as JSON\n");
    }

    /**
     * Returns the value of a property from the configuration.
     * Access nested properties by using '->'.
     * If property value is in itself an object with multiple
     * properties, this is returned as an associative array.
     */
    public static function getPropertyValue($property): mixed {
        $val = self::$confDict;
        foreach(explode("->", $property) as $prop){
            if (!array_key_exists($prop, $val))
                die("Could not access configuration property $prop\n");
            $val = $val[$prop];
        }
        return $val;
    }

    /**
     * Returns a string based on the content of the DSN JSON object
     * on the form: 
     * mimer:<propertyname1>=<propertyvalue2>;<propertyname1>=<propertyvalue2>...;
     */
    public static function getDSN(): string {
        $dsnData = self::$confDict['connection']['dsn'];
        $dsn = "mimer:";
        foreach($dsnData as $pname => $pval){
            $dsn .= $pname . "=" . $pval . ";";
        }
        return $dsn;
    }
}

PDOMimerTestConfig::init();
?>
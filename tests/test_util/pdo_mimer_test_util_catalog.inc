<?php
require_once("pdo_mimer_test_util_db_entities.inc");
require_once("pdo_mimer_test_util_preset.inc");
require_once("pdo_mimer_test_util_types.inc");

/**
 * The catalog of available test DBs. 
 * To add a new preset, make a function called makeDB<name>
 * according to existing examples and then add it to init().
 */
class PDOMimerTestCatalog {
    private static array $presets = []; // Assoc. array where keys are preset names. 

    /**
     * Adds all presets to the catalog, thus making them available
     * from the tests.
     */
    public static function init(){
        self::addPreset(self::makeDBAllDataTypes());
        self::addPreset(self::makeDBPerson());
        self::addPreset(self::makeDBLOBs());
        self::addPreset(self::makeDBBasic());
    }

    /**
     * Searches the catalog for a databank preset with the correct name
     * and returns it if found. Returns null if nothing was found.
     */
    public static function getPreset(string $presetName): ?PDOMimerTestPreset {
        foreach (self::$presets as $preset)
            if ($preset->getName() === $presetName)
                return $preset;
        return null;
    }

    /**
     * Adds the test database to the catalog array, indexed by the 
     * DB name.
     */
    private static function addPreset(PDOMimerTestPreset $db): void {
        $dbname = $db->getName();
        if (array_key_exists($dbname, self::$presets))
            die("Could not create test database $dbname: " . 
                "A database with that name already exists in catalog.");

        self::$presets[$dbname] = $db;
    }

    ##################### PRESETS #######################

    /**
     * Multiple tables divided according to data types.
     * One row each with test data.
     * 
     * TODO: *Add interval types
     * 
     * LOBs are separated into own table. 
     */
    private static function makeDBAllDataTypes() {
        $tstDb = new PDOMimerTestPreset("db_allTypes");
        $tblInt = new PDOMimerTestTable("integer");
        $tblFloats = new PDOMimerTestTable("floating_point");
        $tblStr = new PDOMimerTestTable("string");
        $tblNstr = new PDOMimerTestTable("national_string");
        $tblBin = new PDOMimerTestTable("binary");
        $tblDate = new PDOMimerTestTable("datetime");
        $tblInter = new PDOMimerTestTable("interval");
        $tblBool = new PDOMimerTestTable("boolean");
        $tblLobs = new PDOMimerTestTable("lob");

        $colInt = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("smallintc", PDOMimerDataType::SMALLINT),
            new PDOMimerTestColumn("bigintc", PDOMimerDataType::BIGINT),
            new PDOMimerTestColumn("intc", PDOMimerDataType::INT)
        ];

        $valsInt = [
            ["id", "smallintc", "bigintc", "intc"],
            [1,     12,         5000,       302  ]
        ];

        $colFloats = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("decc", PDOMimerDataType::DEC, false, [5, 3]),
            new PDOMimerTestColumn("doublec", PDOMimerDataType::DOUBLE_PRECISION),
            new PDOMimerTestColumn("floatc", PDOMimerDataType::FLOAT),
            new PDOMimerTestColumn("realc", PDOMimerDataType::REAL)
        ];

        /* double and float have values that can be perfectly represented with 
            double precision but not with single */
        $valsFloats = [
            ["id", "decc",      "doublec",              "floatc",               "realc"],
            [1,    "30.125",    "2.9802322387695E-8",   "2.9802322387695E-8",   "345.0625"]
        ];

        $colStr = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("charc", PDOMimerDataType::CHAR, false, [5]),
            new PDOMimerTestColumn("varcharc", PDOMimerDataType::VARCHAR, false, [50])
        ];

        $valsStr = [
            ["id", "charc", "varcharc"],
            [1,    "Lorem",  "ipsum"  ]
        ];

        $colNstr = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("ncharc", PDOMimerDataType::NCHAR, false, [6]),
            new PDOMimerTestColumn("nvarcharc", PDOMimerDataType::NVARCHAR, false, [50])
        ];

        $valsNstr = [
            ["id", "ncharc",        "nvarcharc"        ],
            [1,    "绝不会放弃你",   "永远不会让你失望"  ]
        ];

        $colLobs = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("blobc", PDOMimerDataType::BLOB),
            new PDOMimerTestColumn("clobc", PDOMimerDataType::CLOB),
            new PDOMimerTestColumn("nclobc", PDOMimerDataType::NCLOB)
        ];

        $valsLobs = [
            ["id",  "blobc",     "clobc",   "nclobc"                ],
            [1,     "xyz",      "dolor",   "永远不会跑来跑去抛弃你"  ]
        ];

        $colBin = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("binaryc", PDOMimerDataType::BINARY, false, [3]),
            new PDOMimerTestColumn("varbinaryc", PDOMimerDataType::VARBINARY, false, [50])
        ];

        $valsBin = [
            ["id",  "binaryc",  "varbinaryc"    ],
            [1,     "xyz",      "xyz"          ]
        ];

        $colDate = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("datec", PDOMimerDataType::DATE),
            new PDOMimerTestColumn("timec", PDOMimerDataType::TIME),
            new PDOMimerTestColumn("timestampc", PDOMimerDataType::TIMESTAMP, false, [0])
        ];

        $valsDate = [
            ["id",  "datec",        "timec",    "timestampc"            ],
            [1,     "1995-02-09",   "09:14:00", "2021-12-31 23:59:30"   ]
        ];

        $colBool = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("booleanc", PDOMimerDataType::BOOLEAN)
        ];

        $valsBool = [
            ["id", "booleanc"],
            [1,     true]
        ];

        /** TODO 
        $colInter = [
            
        ];
        $valsInter = [
            
        ];
        */

        $names = ["Int", "Floats", "Str", "Nstr", "Lobs", 
                    "Bin", "Date", "Bool"];

        foreach($names as $name){
            foreach(${"col" . $name} as $col)
                ${"tbl" . $name}->addColumn($col);
                
            ${"tbl" . $name}->addValues(${"vals" . $name});
            $tstDb->addTable(${"tbl" . $name});
        }
        
        return $tstDb;
    }

    /**
     * Simple table representing a person.
     */
    private static function makeDBPerson() {
        $tstDb = new PDOMimerTestPreset("db_person");
        $tbl = new PDOMimerTestTable("person");
        $id = new PDOMimerTestColumn("id", PDOMimerDataType::INT, true);
        $fname = new PDOMimerTestColumn("firstname", PDOMimerDataType::VARCHAR, false, [50]);
        $lname = new PDOMimerTestColumn("lastname", PDOMimerDataType::VARCHAR, false, [50]);
        $birthday = new PDOMimerTestColumn("birthday", PDOMimerDataType::DATE);
        $tbl->addColumn($id);
        $tbl->addColumn($fname);
        $tbl->addColumn($lname);
        $tbl->addColumn($birthday);
        $vals = [
            ["id",  "firstname",    "lastname",     "birthday"],
            [1,     "Hugo",         "Johnson",      "1993-12-09"],
            [2,     "Alice",        "Miller",       "1945-04-01"],
            [3,     "Ricky",        "Hull",         "2001-01-14"]
        ];
        $tbl->addValues($vals);
        $tstDb->addTable($tbl);
        return $tstDb;
    }

    /**
     * For LOB specific tests.
     */
    private static function makeDBLOBs() {
        $tstDb = new PDOMimerTestPreset("db_lobs");
        $tbl = new PDOMimerTestTable("lobs");
        $id = new PDOMimerTestColumn("id", PDOMimerDataType::INT, true);
        $clob = new PDOMimerTestColumn("clobcol", PDOMimerDataType::CLOB);
        $nclob = new PDOMimerTestColumn("nclobcol", PDOMimerDataType::NCLOB);
        $blob = new PDOMimerTestColumn("blobcol", PDOMimerDataType::BLOB);
        $tbl->addColumn($id);
        $tbl->addColumn($clob);
        $tbl->addColumn($nclob);
        $tbl->addColumn($blob);
        $vals = [
            ["id",  "clobcol",    "nclobcol",     "blobcol"],
            [1,     "abcdefg",    "绝不会放弃你",  "xyz"]
        ];
        $tbl->addValues($vals);
        $tstDb->addTable($tbl);
        return $tstDb;
    }

    /**
     * Basic table useful for tests focused on behaviour
     * rather than e.g. data types.
     */
    private static function makeDBBasic() {
        $tstDb = new PDOMimerTestPreset("db_basic");
        $tbl = new PDOMimerTestTable("basic");
        $id = new PDOMimerTestColumn("id", PDOMimerDataType::INT, true);
        $text = new PDOMimerTestColumn("text", PDOMimerDataType::VARCHAR, false, [100]);
        $tbl->addColumn($id);
        $tbl->addColumn($text);

        $vals = [
            ["id",  "text"],
            [1,     "lorem"],
            [2,     "ipsum"],
        ];
        $tbl->addValues($vals);
        $tstDb->addTable($tbl);
        return $tstDb;
    }
}
PDOMimerTestCatalog::init();
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
        self::addPreset(self::makeDBSimpleDataTypes());
        self::addPreset(self::makeDBComplexDataTypes());
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
            new PDOMimerTestColumn("decc", PDOMimerDataType::DEC, false, [15, 3]),
            new PDOMimerTestColumn("doublec", PDOMimerDataType::DOUBLE_PRECISION),
            new PDOMimerTestColumn("floatc", PDOMimerDataType::FLOAT),
            new PDOMimerTestColumn("realc", PDOMimerDataType::REAL)
        ];

        $valsFloats = [
            ["id", "decc", "doublec", "floatc", "realc"],
            [1,     30.5,   34.125,   0.25,   345.0625 ]
        ];

        $colStr = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("charc", PDOMimerDataType::CHAR, false, [50]),
            new PDOMimerTestColumn("varcharc", PDOMimerDataType::VARCHAR, false, [50])
        ];

        $valsStr = [
            ["id", "charc", "varcharc"],
            [1,    "Lorem",  "ipsum"  ]
        ];

        $colNstr = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("ncharc", PDOMimerDataType::NCHAR, false, [50]),
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
            [1,     0xDEADC0DE,  "dolor",   "永远不会跑来跑去抛弃你"  ]
        ];

        $colBin = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("binaryc", PDOMimerDataType::BINARY),
            new PDOMimerTestColumn("varbinaryc", PDOMimerDataType::VARBINARY, false, [50])
        ];

        $valsBin = [
            ["id",  "binaryc",  "varbinaryc"    ],
            [1,      0xF0,      0xDEADC0DE      ]
        ];

        $colDate = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("datec", PDOMimerDataType::DATE),
            new PDOMimerTestColumn("timec", PDOMimerDataType::TIME),
            new PDOMimerTestColumn("timestampc", PDOMimerDataType::TIMESTAMP)
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
     * Table with each datatype that is supported by the Mimer C API's
     * input/output routines.
     */
    private static function makeDBSimpleDataTypes() {
        $tstDb = new PDOMimerTestPreset("db_simpleDatatypes");
        $tbl = new PDOMimerTestTable("simpleDatatypes"); 

        $columns = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("smallintc", PDOMimerDataType::SMALLINT),
            new PDOMimerTestColumn("bigintc", PDOMimerDataType::BIGINT),
            new PDOMimerTestColumn("intc", PDOMimerDataType::INT),
            new PDOMimerTestColumn("floatc", PDOMimerDataType::FLOAT),
            new PDOMimerTestColumn("double_precisionc", PDOMimerDataType::DOUBLE_PRECISION),
            new PDOMimerTestColumn("realc", PDOMimerDataType::REAL),
            new PDOMimerTestColumn("charc", PDOMimerDataType::CHAR, false, [50]),
            new PDOMimerTestColumn("varcharc", PDOMimerDataType::VARCHAR, false, [50]),
            new PDOMimerTestColumn("clobc", PDOMimerDataType::CLOB),
            new PDOMimerTestColumn("ncharc", PDOMimerDataType::NCHAR, false, [50]),
            new PDOMimerTestColumn("nvarcharc", PDOMimerDataType::NVARCHAR, false, [50]),
            new PDOMimerTestColumn("nclobc", PDOMimerDataType::NCLOB),
            new PDOMimerTestColumn("binaryc", PDOMimerDataType::BINARY),
            new PDOMimerTestColumn("varbinaryc", PDOMimerDataType::VARBINARY, false, [50]),
            new PDOMimerTestColumn("blobc", PDOMimerDataType::BLOB),
            new PDOMimerTestColumn("booleanc", PDOMimerDataType::BOOLEAN)
        ];

        foreach($columns as $col)
            $tbl->addColumn($col);
        
        $vals = [
            [
                "id", "intc", "smallintc", "bigintc", 
                "double_precisionc", "floatc", "realc", 
                "charc", "varcharc", "clobc", "ncharc", "nvarcharc", "nclobc", 
                "binaryc", "varbinaryc", "blobc",
                "booleanc"
            ],
            [
                1, -14, 0, 200, 
                1.797, 4.940, 3.402,
                "lorem", "ipsum", "dolor", "绝不会放弃你", "永远不会让你失望", "永远不会跑来跑去抛弃你",
                0xF0, 0xDEADC0DE, 0x1A7D3A9C,
                true        
            ]
        ];

        $tbl->addValues($vals);
        $tstDb->addTable($tbl);
        return $tstDb;
    }

    /**
     * Table with each datatype that is NOT supported by the Mimer C API's
     * input/output routines.
     * 
     * TODO: Extend for all the interval types
     */
    private static function makeDBComplexDataTypes() {
        $tstDb = new PDOMimerTestPreset("db_complexDatatypes");
        $tbl = new PDOMimerTestTable("complexDatatypes");

        $columns = [
            new PDOMimerTestColumn("id", PDOMimerDataType::INT, true),
            new PDOMimerTestColumn("decc", PDOMimerDataType::DEC, false, [15, 3]),
            new PDOMimerTestColumn("datec", PDOMimerDataType::DATE),
            new PDOMimerTestColumn("timec", PDOMimerDataType::TIME),
            new PDOMimerTestColumn("timestampc", PDOMimerDataType::TIMESTAMP),
            new PDOMimerTestColumn("intervalc", PDOMimerDataType::INTERVAL),
        ];

        foreach($columns as $col)
            $tbl->addColumn($col);
        
        $vals = [
            ["id", "decc",  "datec",        "timec",    "timestampc",           "intervalc"],
            [1,     10.309, "1995-02-09",   "09:14:00", "2021-12-31 23:59:30",  "1:30"]
        ];

        $tbl->addValues($vals);
        $tstDb->addTable($tbl);
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
     * For LOB specific tests
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
            [1,     "abcdefg",    "绝不会放弃你",  0xF0F0F0F0]
        ];
        $tbl->addValues($vals);
        $tstDb->addTable($tbl);
        return $tstDb;
    }

    /**
     * Basic table for simple tests.
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
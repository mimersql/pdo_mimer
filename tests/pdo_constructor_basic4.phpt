--TEST--
PDO Mimer(Constructor): Connect to DB without dbname, user, or password

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
When there is a OS_USER login (where ident name = OS user name) 
and a default DB is setup, it should be possible to connect to DB 
using only DSN header.

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
PDOMimerTestUtil::skipIfWindows();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil();
$dsn = $util->getFullDSN();
$dbName = PDOMimerTestConfig::getDBName();

try {
    // Make sure there is a OS_USER login
    $osUserName = $util->getCurrentOsUser();
    $db = new PDO($dsn);
    try {
        $db->exec("CREATE IDENT $osUserName AS USER USING 'pw'");
        $db->exec("ALTER IDENT $osUserName ADD OS_USER '$osUserName'");
    } catch (PDOException $e) {
        // Ignore error if ident already exists
        if(!str_contains($e->getMessage(), "-12558 Ident named $osUserName already exists"))
            throw $e;
    }
    $db = null;

    // Make sure there is a default database
    putenv("MIMER_DATABASE=$dbName");
    putenv("MIMER_DATABASE_LOCAL=$dbName");
    putenv("MIMER_DATABASE_REMOTE=$dbName");

    // Try connecting with empty DSN
    $db = new PDO("mimer:");

} catch (PDOException $e) {
    print $e->getMessage();

} finally {
    // Try to clean up test ident
    if($util->objectExists("IDENT", $osUserName)){
        $db = new PDO($dsn);
        $db->exec("DROP IDENT $osUserName");
    }
}

PDOMimerTestSetup::tearDown();
?>

--EXPECT--

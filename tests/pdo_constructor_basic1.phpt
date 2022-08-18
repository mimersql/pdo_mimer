--TEST--
PDO Mimer(Constructor): Connect to DB using only DB name

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Tests that if IDENT has a OS_USER login set up - where the 
IDENT name is the same as the OS user name - the DSN must 
only contain name of DB. 

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil();
$dsn = $util->getFullDSN();
$dbName = $util->getConfigValue("connection->dsn->dbname");

try {
    // Make sure there is a OS_USER login with same name as OS user
    $osUserName = $util->getCurrentOsUser();
    $db = new PDO($dsn);
    $db->exec("CREATE IDENT $osUserName AS USER USING 'pw'");
    $db->exec("ALTER IDENT $osUserName ADD OS_USER '$osUserName'");
    $db = null;
    
    // Should now be possible to log into db with only dbname
    $db = new PDO('mimer:dbname=' . $dbName);

} catch (PDOException $e) {
    print $e->getMessage();

} finally {
    // Try to clean up test ident
    if($util->objectExists("IDENT", $osUserName)){
        $db = new PDO($dsn);
        $db->exec("DROP IDENT $osUserName");
    }
}
?>

--EXPECT--

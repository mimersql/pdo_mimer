--TEST--
PDO Mimer(Constructor): Connect to DB with only dbname and user

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Tests that if IDENT has a OS_USER login set up the DSN must 
only contain name of DB and username. 

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
PDOMimerTestUtil::skipIfWindows();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil();
$dsn = $util->getFullDSN();
$dbName = $util->getConfigValue("connection->dsn->dbname");

try {
    // Make sure there is a OS_USER login
    $osUserName = $util->getCurrentOsUser();
    $identName = "test_ident";
    $db = new PDO($dsn);
    $db->exec("CREATE IDENT $identName AS USER USING 'pw'");
    $db->exec("ALTER IDENT $identName ADD OS_USER '$osUserName'");
    $db = null;

    // Should now be possible to log into db without password
    $db = new PDO('mimer:dbname=' . $dbName . ";user=" . $identName);

} catch (PDOException $e) {
    print $e->getMessage();

} finally {
    // Try to clean up test ident
    if($util->objectExists("IDENT", $identName)){
        $db = new PDO($dsn);
        $db->exec("DROP IDENT $identName");
    }
}
?>

--EXPECT--

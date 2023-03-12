--TEST--
PDO Mimer(beginTransaction): no insertion in table before commit

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
beginTransaction should turn off autocommit mode, so no changes should 
be made to the database before a following commit. The user making the 
transaction will see the changes even before the commit, so it must check
the database from a different connection. 

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_basic", false);
$dsn = $util->getFullDSN();
$tblName = "basic";

try {
    $db_1 = new PDO($dsn);    
    $db_1->beginTransaction();

    // Fill table with values
    $dmlStmts = $util->getTableDML($tblName);
    foreach ($dmlStmts as $stmt)
        $db_1->exec($stmt);

    // Check if changes are visible from other connection
    $db_2 = new PDO($dsn);
    $result = $db_2->query("SELECT * FROM $tblName");
    $nrows = count($result->fetchAll(PDO::FETCH_COLUMN));
    if ($nrows !== 0)
        die ("DB changed before a commit was made");

} catch (PDOException $e) {
    print $e->getMessage();
}

$db_1 = null;
$result = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--

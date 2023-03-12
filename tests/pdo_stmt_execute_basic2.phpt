--TEST--
PDO Mimer(stmt-execute): indexed array as argument

--EXTENSIONS--
pdo
pdo_mimer

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_person");
$dsn = $util->getFullDSN();
$tblName = "person";
$tbl = $util->getTable($tblName);
$rows = $tbl->getRows();

try {
    $db = new PDO($dsn);
    $stmt = $db->prepare("SELECT id FROM $tblName WHERE firstname = (?) AND lastname = (?)");

    foreach($rows as $i => $row) {
        $fname = $row['firstname'];
        $lname = $row['lastname'];
        $id = $row['id'];
        $stmt->execute(array($fname, $lname));
        $res = $stmt->fetch(PDO::FETCH_NAMED);
        if($res['id'] !== $id)
            die("Fetched value ({$res['id']}) did not match test table value ($id)");
        $stmt->closeCursor();
    }
    
} catch (PDOException $e) {
    print $e->getMessage();
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--

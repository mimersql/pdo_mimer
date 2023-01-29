--TEST--
PDO Mimer(stored-procedure): inout param in a stored procedure

--EXTENSIONS--
pdo
pdo_mimer

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_stored_procedures");
$dsn = $util->getFullDSN();

$procedure_call = "CALL LEET(?);";

try {
    $db = new PDO($dsn);

    $stmt = $db->prepare($procedure_call);
    $stmt->bindParam(1, $leet, PDO::PARAM_INT|PDO::PARAM_INPUT_OUTPUT);
    $stmt->execute();
    var_dump($leet);
} catch (PDOException $e) {
    die($e->getMessage());
}
?>

--EXPECT--
int(1337)
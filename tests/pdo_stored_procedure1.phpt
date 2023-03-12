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

$procedure_call = "CALL SQUARE(?);";
$return_value = 12;

try {
    $db = new PDO($dsn);
    $stmt = $db->prepare($procedure_call);
    $stmt->bindParam(1, $return_value, PDO::PARAM_INT|PDO::PARAM_INPUT_OUTPUT);
    $stmt->execute();
    var_dump($return_value);
} catch (PDOException $e) {
    die($e->getMessage());
}

$stmt = null;
PDOMimerTestSetup::tearDown();
?>

--EXPECT--
int(144)
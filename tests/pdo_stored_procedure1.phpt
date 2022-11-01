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
$util = new PDOMimerTestUtil("db_basic");
$dsn = $util->getFullDSN();

$procedure_create = "CREATE PROCEDURE SQUARE(INOUT p_root INTEGER) BEGIN SET p_root = p_root * p_root; END";
$procedure_call = "CALL SQUARE(?);";
$return_value = 12;

try {
    $db = new PDO($dsn);
    $db->exec($procedure_create);

    $stmt = $db->prepare($procedure_call);
    $stmt->bindParam(1, $return_value, PDO::PARAM_INT|PDO::PARAM_INPUT_OUTPUT);
    $stmt->execute();
    var_dump($return_value);
} catch (PDOException $e) {
    die($e->getMessage());
}
?>

--EXPECT--
int(144)
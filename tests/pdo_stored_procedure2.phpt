--TEST--
PDO Mimer(stored-procedure):  param in a stored procedure

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

$procedure_create = "CREATE PROCEDURE SQUARE(IN p_root INT, OUT p_product INT) CONTAINS SQL BEGIN SET p_product = p_root * p_root; END";
$procedure_call = "CALL SQUARE(?, ?);";
$root = 12;

try {
    $db = new PDO($dsn);
    $db->exec($procedure_create);
    $stmt = $db->prepare($procedure_call);
    $stmt->bindValue(1, $root, PDO::PARAM_INT);
    $stmt->bindParam(2, $product, PDO::PARAM_INT);
    $stmt->execute();
    var_dump($product);
} catch (PDOException $e) {
    die($e);
}
?>

--EXPECT--
int(144)
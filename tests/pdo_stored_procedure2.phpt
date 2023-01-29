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
$util = new PDOMimerTestUtil("db_stored_procedures");
$dsn = $util->getFullDSN();

$procedure_call = "CALL SQUARE(?, ?);";
$root = 12;

try {
    $db = new PDO($dsn);

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
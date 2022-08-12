--TEST--
PDO Mimer (stmt-bindValue): bind named placeholder

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());

try {
    $db = new PDOMimerTest(true);
    $stmt = $db->prepare("SELECT $column FROM $table WHERE $column = $param");

    foreach ($values as $val){
        $stmt->bindValue($param, $val, $pdo_type);
        $stmt->execute();
        $res = $stmt->fetch(PDO::FETCH_NUM)[0];
        $stmt->closeCursor();

        if ($res !== $val)
            die("Fetched value does not match bound value");
    }
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

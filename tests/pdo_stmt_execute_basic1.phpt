--TEST--
PDO Mimer (stmt-execute): execute with multiple named placeholders

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::makeExTablePerson();
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(true);

    $stmt = $db->prepare("SELECT id FROM $table WHERE name = :name AND lastname = :lastname");

    for($i = 0; $i < $rows; $i++){
        $stmt->execute(array(':name' => $name->value($i+1), ':lastname' => $lastname->value($i+1)));
        $row = $stmt->fetch(PDO::FETCH_NAMED);
        if($row['id'] !== $id->value($i+1))
            die("Fetched value ({$row['id']}) did not match test table value ({$id->value($i+1)})");
        $stmt->closeCursor();
    }
    
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

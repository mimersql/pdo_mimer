--TEST--
Mimer SQL (mimerAddBatch): MimerAddBatch called from PDOStatement instance

--EXTENSIONS--
pdo
pdo_mimer

--SKIPIF--
<?php require('skipif.inc'); ?>

--FILE--
<?php
require('testdb.inc');

$db = new PDO(PDO_MIMER_TEST_DSN, PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS, array(
    PDO::ATTR_ERRMODE => PDO::ERRMODE_SILENT
));

$table_tst = 'tsttbl';
$col_id = 'id';
$col_name = 'name';

$var_id = ':' . $col_id . 'var';
$var_name = ':' . $col_name . 'var';

$db->exec("DROP TABLE $table_tst");
$db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

try {
    $db->exec("CREATE TABLE $table_tst ($col_id INT NOT NULL PRIMARY KEY, $col_name VARCHAR(1))");

    $stmt = $db->prepare("INSERT INTO $table_tst ($col_id, $col_name) VALUES ($var_id, $var_name)");

    for ($i = 1; $i <= $max = 5; $i++) {
        $stmt->bindValue($var_id, $i, PDO::PARAM_INT);
        $stmt->bindValue($var_name, chr($i-1 + ord('A')));
        $i < $max ? $stmt->mimerAddBatch() : $stmt->execute();
    }

    foreach($db->query("SELECT $col_id, $col_name FROM $table_tst") as $row) {
        print_r($row);
    }

} catch (PDOException $e) {
    print $e->getMessage();
}

?>

--EXPECT--
Array
(
    [id] => 1
    [0] => 1
    [name] => A
    [1] => A
)
Array
(
    [id] => 2
    [0] => 2
    [name] => B
    [1] => B
)
Array
(
    [id] => 3
    [0] => 3
    [name] => C
    [1] => C
)
Array
(
    [id] => 4
    [0] => 4
    [name] => D
    [1] => D
)
Array
(
    [id] => 5
    [0] => 5
    [name] => E
    [1] => E
)

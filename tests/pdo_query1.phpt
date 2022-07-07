--TEST--
Mimer SQL (query): fetching a result set 

--EXTENSIONS--
pdo_mimer

--SKIPIF--
<?php require('skipif.inc'); ?>

--FILE--
<?php
require("testdb.inc");
try {
    $dbh = new PDO(PDO_MIMER_TEST_DSN, PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS);
    @$dbh->exec('DROP TABLE tsttbl');
    $dbh->exec('CREATE TABLE tsttbl(id INT NOT NULL PRIMARY KEY, name VARCHAR(10))');
    $dbh->exec("INSERT INTO tsttbl VALUES(1, 'A')");
    $dbh->exec("INSERT INTO tsttbl VALUES(2, 'B')");
    $dbh->exec("INSERT INTO tsttbl VALUES(3, 'C')");

    foreach($dbh->query('SELECT * from tsttbl') as $row) {
        print_r($row);
    }
    

} catch (PDOException $e) {
    print "Error!: " . $e->getMessage();
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

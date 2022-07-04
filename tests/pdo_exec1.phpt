--TEST--
Mimer SQL (Exec): Row count 

--DESCRIPTION--
Tests that PDO::exec returns the number of affected rows.

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
    $dbh->exec('CREATE TABLE tsttbl(id INT NOT NULL PRIMARY KEY)');
    $dbh->exec('INSERT INTO tsttbl VALUES(1)');
    $dbh->exec('INSERT INTO tsttbl VALUES(2)');
    $dbh->exec('INSERT INTO tsttbl VALUES(3)');
    $count = $dbh->exec('DELETE FROM tsttbl');
    var_dump($count);

} catch (PDOException $e) {
    print "Error!: " . $e->getMessage();
}

?>
--EXPECT--
int(3)

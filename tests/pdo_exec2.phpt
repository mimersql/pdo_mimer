--TEST--
Mimer SQL (Exec): exec row count with result set statement 

--DESCRIPTION--
Tests that PDO::exec returns zero count on a SELECT statement

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
    $res = $dbh->exec('SELECT * FROM tsttbl');
    var_dump($res);

} catch (PDOException $e) {
    [$_, $code, $msg] = $e->errorInfo;
    print "[$code]: $msg";
}

?>
--EXPECTF--
[-24101]: An illegal sequence of API calls was detected

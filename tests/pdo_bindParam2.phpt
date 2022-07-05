--TEST--
Mimer SQL (bindParam): bind positional placeholders

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
    $stmt = $dbh->prepare("INSERT INTO tsttbl (id, name) VALUES(?, ?)");
    
    $stmt->bindParam(1, $idvar, PDO::PARAM_INT);
    $stmt->bindParam(2, $namevar, PDO::PARAM_STR);
    $idvar = 1;
    $namevar = 'A';
    $stmt->execute();

    foreach($dbh->query('SELECT * from tsttbl') as $row) {
        print_r($row);
    }
    

} catch (PDOException $e) {
    print "Error!: " . $e->getMessage();
}

?>
--EXPECT--
TBD

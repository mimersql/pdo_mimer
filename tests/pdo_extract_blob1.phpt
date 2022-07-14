--TEST--
Mimer SQL (LOB): extracting a small blob from database

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
    $dbh->exec('CREATE TABLE tsttbl(id INT NOT NULL PRIMARY KEY, bindata BLOB)');

    $bin_str = pack('C*', 0, 255, 47);
    $fp = tmpfile();
    fwrite($fp, $bin_str);
    rewind($fp);

    $insert_stmt = $dbh->prepare("insert into tsttbl (id, bindata) values (1, :blob)");
    $insert_stmt->bindValue(':blob', $fp, PDO::PARAM_LOB);
    $insert_stmt->execute();
    fclose($fp);

    $extract_stmt = $dbh->prepare("select bindata from tsttbl where id=1");
    $extract_stmt->execute();
    $extract_stmt->bindColumn(1, $lob, PDO::PARAM_LOB);
    $extract_stmt->fetch(PDO::FETCH_BOUND);
    
    $bin_str = fread($lob, 3);
    $binvals = unpack("C*", $bin_str);
    var_dump($binvals);
    
} catch (PDOException $e) {
    print "Error!: " . $e->getMessage();
}

?>
--EXPECT--
array(3) {
  [1]=>
  int(0)
  [2]=>
  int(255)
  [3]=>
  int(47)
}
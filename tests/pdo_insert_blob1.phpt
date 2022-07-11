--TEST--
Mimer SQL (LOB): inserting a blob from file into the database

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

    $tstnum = 0x61626364; //0x61-64 = a-d ASCII
    $bin_str = pack('i', $tstnum);
    $fp = fopen("./tstfile_blob", "w+");
    fwrite($fp, $bin_str);
    rewind($fp);

    $insert_stmt = $dbh->prepare("insert into tsttbl (id, bindata) values (1, :blob)");
    $insert_stmt->bindValue(':blob', $fp, PDO::PARAM_LOB);
    $insert_stmt->execute();
    fclose($fp);
    print "done";

    
} catch (PDOException $e) {
    print "Error!: " . $e->getMessage();
}

?>
--EXPECT--
done
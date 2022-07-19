--TEST--
Mimer SQL (LOB): inserting a small blob from file into the database

--DESCRIPTION--
Inserts a blob of just a few bytes and validates that the correct number
of bytes was inserted. Does not validate content of inserted bytes.

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
    $nbytes = strlen($bin_str);
    $fp = tmpfile();
    fwrite($fp, $bin_str);
    rewind($fp);

    $insert_stmt = $dbh->prepare("insert into tsttbl (id, bindata) values (1, :blob)");
    $insert_stmt->bindValue(':blob', $fp, PDO::PARAM_LOB);
    $insert_stmt->execute();
    fclose($fp);
    
    // Verify number of inserted bytes
    $stm = $dbh->query("select OCTET_LENGTH(bindata) as ol from tsttbl where id=1;");
    $res = $stm->fetch();
    if ($res['ol'] != $nbytes){
        print "Number of bytes in DB differ from number of bytes in input file.\n";
        print "Bytes in DB: " . $res['ol'] . "\n";
        print "Bytes in input file: " . $nbytes . "\n";
    } else {
        print "done";
    }
    
} catch (PDOException $e) {
    print "Error!: " . $e->getMessage();
}

?>
--EXPECT--
done
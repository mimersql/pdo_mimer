--TEST--
Mimer SQL (LOB): inserting a nclob from UTF8 encoded file

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
    $dbh->exec('CREATE TABLE tsttbl(id INT NOT NULL PRIMARY KEY, nclobdata NCLOB(20))');

    $tsttext = array(0xc3, 0x85, 0xc3, 0x84, 0xc3, 0x96); // ="ÅÄÖ" in UTF8
    $fp = tmpfile();
    foreach($tsttext as $chr){
        fwrite($fp, pack('C', $chr));
    }
    
    rewind($fp);
    $insert_stmt = $dbh->prepare("insert into tsttbl (id, nclobdata) values (1, :nclob)");
    $insert_stmt->bindValue(':nclob', $fp, PDO::PARAM_LOB);
    $insert_stmt->execute();
    fclose($fp);
    print "done";

} catch (PDOException $e) {
    print "Error!: " . $e->getMessage();
}

?>
--EXPECT--
done
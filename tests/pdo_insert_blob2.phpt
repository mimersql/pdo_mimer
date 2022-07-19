--TEST--
Mimer SQL (LOB): inserting a blob larger than available process memory

--DESCRIPTION--
The purpose of this test is to verify that the PDO driver is reading in
the LOB data in chunks, instead of trying to allocate a buffer to hold
the entire data before inserting it into DB. 

Validates the number but not content of inserted bytes.

--EXTENSIONS--
pdo_mimer

--SKIPIF--
<?php require('skipif.inc'); ?>

--FILE--
<?php
require("testdb.inc");
try {
    
    // Limit memory of script process 
    $tot_mem_usage = memory_get_usage(true);    
    ini_set('memory_limit',$tot_mem_usage);     
    $used_mem_usage = memory_get_usage(false);  
    $mem_available = $tot_mem_usage - $used_mem_usage;
    
    // Generate binary data larger than available memory
    $filesize = $mem_available + 10000;
    $fp = tmpfile();
    for ($x = 0; $x < $filesize; $x++){
        fwrite($fp, pack("C", 0xFF), 1); 
    }
    rewind($fp);

    // insert the blob data
    $dbh = new PDO(PDO_MIMER_TEST_DSN, PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS);
    @$dbh->exec('DROP TABLE tsttbl');
    $dbh->exec('CREATE TABLE tsttbl(id INT NOT NULL PRIMARY KEY, bindata BLOB(10M))'); //blob size not dynamic atm
    $insert_stmt = $dbh->prepare("insert into tsttbl (id, bindata) values (1, :blob)");
    $insert_stmt->bindValue(':blob', $fp, PDO::PARAM_LOB);
    $insert_stmt->execute();
    fclose($fp);

    // Verify number of inserted bytes
    $stm = $dbh->query("select OCTET_LENGTH(bindata) as ol from tsttbl where id=1;");
    $res = $stm->fetch();
    if ($res['ol'] != $filesize){
        print "Number of bytes in DB differ from number of bytes in input file.\n";
        print "Bytes in DB: " . $res['ol'] . "\n";
        print "Bytes in input file: " . $filesize . "\n";
    } else {
        print "done";
    }

} catch (PDOException $e) {
    print "Error!: " . $e->getMessage();
}

?>
--EXPECT--
done
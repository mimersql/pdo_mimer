--TEST--
Mimer SQL (LOB): inserting a blob larger than available process memory

--DESCRIPTION--
The purpose of this test is to verify that the PDO driver is reading in
the LOB data in chunks, instead of trying to allocate a buffer to hold
the entire data before inserting it into DB. 

--EXTENSIONS--
pdo_mimer

--SKIPIF--
<?php require('skipif.inc'); ?>

--FILE--
<?php
require("testdb.inc");
try {
    
    // Limit memory of script process 
    $tot_mem_usage = memory_get_usage(true);    // includes unused pages
    ini_set('memory_limit',$tot_mem_usage);    // lock script memory usage (can't go lower than already allocated)
    $used_mem_usage = memory_get_usage(false);  // only in-use memory
    $mem_available = $tot_mem_usage - $used_mem_usage;
    
    // Generate binary data larger than available memory
    $fp = tmpfile();
    for ($x = 0; $x < $mem_available; $x++){
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
    print "done";

    
} catch (PDOException $e) {
    print "Error!: " . $e->getMessage();
}

?>
--EXPECT--
done
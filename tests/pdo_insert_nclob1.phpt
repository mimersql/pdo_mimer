--TEST--
Mimer SQL (LOB): inserting a memory-exceeding nclob from UTF8 encoded file

--DESCRIPTION--
Inserts the content of a UTF-8 encoded file larger than available process memory
and verifies the number of inserted characters (but not the content). 

Test characters were chosen as to have one of each 1-byte, 2-byte, 3-byte and 4-byte
code points. 

Note: Make sure the length of the repeating characters (in bytes) is not a divisor 
of the chunk size. This is to check the handling of chunks which would otherwise 
contain partial characters. 

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

    $nchars = 4; // number of codepoints in array
    $tsttext = array(0x61, 0xc3, 0x85, 0xe0, 0xae, 0x87, 0xf0, 0x92, 0x80, 0xbd); // ="aÅஇ𒀽" in UTF8
    $binstr = pack("C*", ...$tsttext);

    // Create a file larger than available memory
    $req_repetitions = ceil(($mem_available + 10000) / count($tsttext));
    $fp = tmpfile();
    for ($x = 0; $x < $req_repetitions; $x++) {
        fwrite($fp, $binstr);
    }
    rewind($fp);

    // Insert into DB
    $dbh = new PDO(PDO_MIMER_TEST_DSN, PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS);
    @$dbh->exec('DROP TABLE tsttbl');
    $dbh->exec('CREATE TABLE tsttbl(id INT NOT NULL PRIMARY KEY, nclobdata NCLOB(10M))');
    $insert_stmt = $dbh->prepare("insert into tsttbl (id, nclobdata) values (1, :nclob)");
    $insert_stmt->bindValue(':nclob', $fp, PDO::PARAM_LOB);
    $insert_stmt->execute();
    fclose($fp);

    // Verify number of inserted characters
    $stm = $dbh->query("select CHAR_LENGTH(nclobdata) as cl from tsttbl where id=1;");
    $res = $stm->fetch();
    if ($res['cl'] != $req_repetitions * $nchars){
        print "Number of chars in DB differ from number of chars in input file.\n";
        print "Chars in DB: " . $res['cl'] . "\n";
        print "Chars in input file: " . $req_repetitions * $nchars . "\n";
    } else {
        print "done";
    }
    
} catch (PDOException $e) {
    print "Error!: " . $e->getMessage();
}

?>
--EXPECT--
done
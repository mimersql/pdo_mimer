--TEST--
Mimer SQL (LOB): inserting a memory-exceeding clob from UTF8 encoded file

--DESCRIPTION--
Inserts the content of a UTF-8 encoded file larger than available process memory
and verifies the number of inserted characters (but not the content). 

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
    $filesize = $mem_available + 10000;

    $nchars = 5; // number of codepoints in array
    $tsttext = array(0x40, 0x61, 0x24, 0x3C, 0x3F); // ="@a$<?" in UTF-8
    $binstr = pack("C*", ...$tsttext);

    // Create a file larger than available memory
    $req_repetitions = ceil($filesize / count($tsttext));
    $fp = tmpfile();
    for ($x = 0; $x < $req_repetitions; $x++) {
        fwrite($fp, $binstr);
    }
    rewind($fp);

    // Insert into DB
    $dbh = new PDO(PDO_MIMER_TEST_DSN, PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS);
    @$dbh->exec('DROP TABLE tsttbl');
    $dbh->exec('CREATE TABLE tsttbl(id INT NOT NULL PRIMARY KEY, clobdata CLOB(10M))');
    $insert_stmt = $dbh->prepare("insert into tsttbl (id, clobdata) values (1, :clob)");
    $insert_stmt->bindValue(':clob', $fp, PDO::PARAM_LOB);
    $insert_stmt->execute();
    fclose($fp);

    // Verify number of inserted characters
    $stm = $dbh->query("select CHAR_LENGTH(clobdata) as cl from tsttbl where id=1;");
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
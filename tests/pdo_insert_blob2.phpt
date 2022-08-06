--TEST--
PDO Mimer (LOB): inserting a blob larger than available process memory
--DESCRIPTION--
The purpose of this test is to verify that the PDO driver is reading in
the LOB data in chunks, instead of trying to allocate a buffer to hold
the entire data before inserting it into DB.

Validates the number but not content of inserted bytes.
--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>
--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    // Generate binary data larger than available memory
    $fp = PDOMimerTest::createFile($filesize = PDOMimertest::limitMemory() + 10000, pack("C", 0xFF), 1);

    // insert the blob data
    $db = new PDOMimerTest(false);
    $blob = new Column('blob', [TYPE => 'BLOB(10M)'], [TYPE => PDO::PARAM_LOB, VALUES => [$fp]]);
    $db->createTables(new Table($table, [$id, $blob]));

    $stmt = $db->prepare("INSERT INTO $table ($id, $blob) VALUES (1, $blob->param)");
    $blob->bindValue($stmt)->execute();
    fclose($fp);

    // Verify number of inserted bytes
    $res = $db->query("SELECT OCTET_LENGTH($blob) as ol FROM $table FETCH 1;")->fetch();
    if ($res['ol'] != $filesize){
        print "Number of bytes in DB differ from number of bytes in input file.\n";
        print "Bytes in DB: " . $res['ol'] . "\n";
        print "Bytes in input file: " . $filesize . "\n";
    }
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>
--EXPECT--

--TEST--
PDO Mimer (LOB): inserting a memory-exceeding nclob from UTF8 encoded file

--DESCRIPTION--
Inserts the content of a UTF-8 encoded file larger than available process memory
and verifies the number of inserted characters (but not the content).

Test characters were chosen as to have one of each 1-byte, 2-byte, 3-byte and 4-byte
code points.

Note: Make sure the length of the repeating characters (in bytes) is not a divisor
of the chunk size. This is to check the handling of chunks which would otherwise
contain partial characters.

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
$nclob = new Column('nclob', [TYPE => 'NCLOB(10M)']);

$chars = 4; // number of codepoints in array
$text = array(0x61, 0xc3, 0x85, 0xe0, 0xae, 0x87, 0xf0, 0x92, 0x80, 0xbd); // ="aÅஇ𒀽" in UTF8
$binstr = pack("C*", ...$text);

// Create a file larger than available memory
$filesize = PDOMimertest::limitMemory() + 10000;
$fp = PDOMimerTest::createFile($req_repetitions = ceil($filesize / count($text)), $binstr);
try {
    // insert the blob data
    $db = new PDOMimerTest(false);
    $db->createTables(new Table($table, [$id, $nclob]));

    $stmt = $db->prepare("INSERT INTO $table ($id, $nclob) VALUES (1, :$nclob)");
    $stmt->bindValue(":$nclob", $fp, PDO::PARAM_LOB);
    $stmt->execute();
    fclose($fp);

    // Verify number of inserted characters
    $res = $db->query("SELECT CHAR_LENGTH($nclob) AS cl FROM $table")->fetch();
    if ($res['cl'] != $req_repetitions * $chars) {
        print "Number of chars in DB differ from number of chars in input file.\n";
        print "Chars in DB: " . $res['cl'] . "\n";
        print "Chars in input file: " . $req_repetitions * $chars . "\n";
    }
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

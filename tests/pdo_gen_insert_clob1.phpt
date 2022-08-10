--TEST--
PDO Mimer (LOB): inserting a memory-exceeding clob from UTF8 encoded file

--DESCRIPTION--
Inserts the content of a UTF-8 encoded file larger than available process memory
and verifies the number of inserted characters (but not the content).

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
$clob = new Column('clob', [TYPE => 'CLOB(10M)']);

$chars = 5; // number of codepoints in array
$text = array(0x40, 0x61, 0x24, 0x3C, 0x3F); // ="@a$<?" in UTF-8
$binstr = pack("C*", ...$text);

// Create a file larger than available memory
$filesize = PDOMimertest::limitMemory() + 10000;
$fp = PDOMimerTest::createFile($req_repetitions = ceil($filesize / count($text)), $binstr);
try {
    // insert the blob data
    $db = new PDOMimerTest(false);
    $db->createTables(new Table($table, [$id, $clob]));

    $stmt = $db->prepare("INSERT INTO $table ($id, $clob) VALUES (1, :$clob)");
    $stmt->bindValue(":$clob", $fp, PDO::PARAM_LOB);
    $stmt->execute();
    fclose($fp);

    // Verify number of inserted characters
    $res = $db->query("SELECT CHAR_LENGTH($clob) AS cl FROM $table")->fetch();
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

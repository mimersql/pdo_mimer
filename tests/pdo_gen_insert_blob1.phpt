--TEST--
PDO Mimer (LOB): inserting a small blob from file into the database

--DESCRIPTION--
Inserts a blob of just a few bytes and validates that the correct number
of bytes was inserted. Does not validate content of inserted bytes.

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
$blob = new Column('blob', [TYPE => 'BLOB']);

$tstnum = 0x61626364; //0x61-64 = a-d ASCII
$bin_str = pack('i', $tstnum);
$nbytes = strlen($bin_str);

fwrite($fp = tmpfile(), $bin_str);
rewind($fp);
try {
    $db = new PDOMimerTest(null);
    $db->createTables(new Table($table, [$id, $blob]));

    $stmt = $db->prepare("insert into $table ($id, $blob) values (1, :$blob)");
    $stmt->bindValue(":$blob", $fp, PDO::PARAM_LOB);
    $stmt->execute();
    fclose($fp);

    // Verify number of inserted bytes
    $res = $db->query("SELECT OCTET_LENGTH($blob) AS ol FROM $table FETCH 1")->fetch();
    if ($res['ol'] != $nbytes) {
        print "Number of bytes in DB differ from number of bytes in input file.\n";
        print "Bytes in DB: " . $res['ol'] . "\n";
        print "Bytes in input file: " . $nbytes . "\n";
    }
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

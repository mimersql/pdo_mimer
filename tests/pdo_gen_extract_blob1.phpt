--TEST--
PDO Mimer(LOB): extracting a small blob from database

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
$blob = new Column('blob', [TYPE => 'BLOB']);

$fp = tmpfile();
$bin_str = pack('C*', 0, 255, 47);
fwrite($fp, $bin_str);
rewind($fp);
try {
    $db = new PDOMimerTest(null);
    $db->createTables(new Table($table, [$id, $blob]));

    $stmt = $db->prepare("insert into $table ($id, $blob) values (1, :$blob)");
    $stmt->bindValue(":$blob", $fp, PDO::PARAM_LOB);
    $stmt->execute();
    fclose($fp);

    $stmt = $db->query("SELECT $blob FROM $table FETCH 1");
    $stmt->bindColumn("$blob", $lob, PDO::PARAM_LOB);
    $stmt->fetch(PDO::FETCH_BOUND);

    $bin_str = fread($lob, 3);
    var_dump(unpack("C*", $bin_str));
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--
array(3) {
  [1]=>
  int(0)
  [2]=>
  int(255)
  [3]=>
  int(47)
}

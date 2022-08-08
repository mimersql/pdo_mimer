--TEST--
PDO Mimer(doer): Num affected rows
--DESCRIPTION--
Tests that PDO::exec returns the correct number of affected rows.
--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>
--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract(false));

try {
    $db = new PDOMimerTest(false);
    if (($dropped = $db->exec("DELETE FROM $table")) !== $rows)
        die("num dropped rows doesn't match num inserted rows: [dropped => $dropped, inserted => $rows]");
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>
--EXPECT--

--XFAIL--
Number of affected rows not (yet?) supported by DDL statements
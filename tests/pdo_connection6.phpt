--TEST--
PDO Mimer(DSN2): Connect to DB using only DB name
--DESCRIPTION--
Requires local, default database with OS_USER ident setup.
--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skipif(PDO_MIMER_TEST_SKIP_OSUSER);
?>
--FILE--
<?php require_once 'pdo_mimer_test.inc';
try {
    $db = new PDOMimerTest('dbname=' . PDOMimerTest::getDBName());
} catch (PDOException $e) {
    print PDOMimerTest::error($e);
}
?>
--EXPECT--

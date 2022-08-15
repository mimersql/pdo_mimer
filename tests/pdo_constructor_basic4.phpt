--TEST--
PDO Mimer(Constructor): Connect to DB without dbname, user, or pass

--DESCRIPTION--
Requires local, default database with OS_USER ident setup

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skipif(PDO_MIMER_TEST_SKIP_OSUSER);
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
try {
    $db = new PDO(PDO_MIMER_EMPTY_DSN);
} catch (PDOException $e) {
    print PDOMimerTest::error($e);
}
?>

--EXPECT--

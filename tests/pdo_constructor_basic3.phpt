--TEST--
PDO Mimer(OS_USER): Connect to DB with only dbname and user

--DESCRIPTION--
Requires local, default database with OS_USER ident setup

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skipif(PDO_MIMER_TEST_SKIP_OSUSER);
PDOMimerTest::skipif(!get_current_user(), "can't stat whoami");
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
try {
    $db = new PDO(PDO_MIMER_EMPTY_DSN . "dbname=" . PDOMimerTest::getDBName(), get_current_user());
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

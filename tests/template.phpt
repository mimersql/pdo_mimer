--TEST--
PDO Mimer(TODO function): TODO Subject
--DESCRIPTION--
TODO description
--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>
--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
// TODO setup
try {
    $db = new PDOMimerTest();
    // TODO tests
} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>
--EXPECT--

--TEST--
PDO Mimer(quote): quote not supported 

--DESCRIPTION--
Driver does not currently support quote. This test 
verifies the behaviour when user tries calling it. 

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php include 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());
try {
    $db = new PDOMimerTest(true);
    $str = 'Test \' string';
    $res = $db->quote($str);
    
    if ($res)
        die("Quote did not return false");

} catch (PDOException $e) {
    PDOMimerTest::error($e);
}
?>

--EXPECT--

--XFAIL-- 
Expected behaviour is TBD

--TEST--
PDO Mimer(getAttribute): getting the generic attributes

--DESCRIPTION--
Test currently throws exception when trying to get value for an
unsupported attribute. 

TBD:
1. Which of the generic attributes should be supported. 
2. Should getAttribute raise exception or return null for 
    non-supported attributes. 

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());

$gen_attributes = array(  
    PDO::ATTR_AUTOCOMMIT,
    PDO::ATTR_CASE,
    PDO::ATTR_CLIENT_VERSION,
    PDO::ATTR_CONNECTION_STATUS,
    PDO::ATTR_DRIVER_NAME,
    PDO::ATTR_ERRMODE,
    PDO::ATTR_ORACLE_NULLS,
    PDO::ATTR_PERSISTENT,
    PDO::ATTR_PREFETCH,
    PDO::ATTR_SERVER_INFO,
    PDO::ATTR_SERVER_VERSION,
    PDO::ATTR_TIMEOUT
);

try {
    $db = new PDOMimerTest(null);
   
    foreach ($gen_attributes as $atr)
        if ($db->getAttribute($atr) === null)
            die("Could not get attribute");

} catch (PDOException $e) {
    die($e->getMessage());
}
?>
--EXPECT--

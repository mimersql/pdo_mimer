--TEST--
PDO Mimer(setAttribute): setting generic attributes

--SKIPIF--
<?php require_once 'pdo_mimer_test.inc';
PDOMimerTest::skip();
?>

--FILE--
<?php require_once 'pdo_mimer_test.inc';
extract(PDOMimerTest::extract());

$attributes = array(
    "PDO::ATTR_CASE"                => PDO::CASE_LOWER,
    "PDO::ATTR_ERRMODE"             => PDO::ERRMODE_WARNING,
    "PDO::ATTR_ORACLE_NULLS"        => PDO::NULL_TO_STRING,
    "PDO::ATTR_STRINGIFY_FETCHES"   => true,
    "PDO::ATTR_TIMEOUT"             => 10,
    "PDO::ATTR_DEFAULT_FETCH_MODE"  => PDO::FETCH_LAZY
);

try {
    $db = new PDOMimerTest(null);

    foreach ($attributes as $attr => $val) {
        if(!$db->setAttribute(constant($attr), $val))
            print("Could not set attribute $attr" . "\n");
    }

    foreach ($attributes as $attr => $val){
        if(!$db->getAttribute(constant($attr)) !== $val)
            print("$attr did not register new value" . "\n");
    }

} catch (PDOException $e) {
    die($e->getMessage());
}
?>

--EXPECT--

--XFAIL--
Must discuss which attributes we can support setting, see issue #66. 
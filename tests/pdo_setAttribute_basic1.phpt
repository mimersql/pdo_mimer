--TEST--
PDO Mimer(setAttribute): setting generic attributes

--EXTENSIONS--
pdo
pdo_mimer

--SKIPIF--
<?php require_once 'pdo_tests_util.inc';
PDOMimerTestUtil::commonSkipChecks();
?>

--FILE--
<?php require_once 'pdo_tests_util.inc';
$util = new PDOMimerTestUtil("db_basic", false);
$dsn = $util->getFullDSN();

$attributes = array(
    "PDO::ATTR_CASE"                => PDO::CASE_LOWER,
    "PDO::ATTR_ERRMODE"             => PDO::ERRMODE_WARNING,
    "PDO::ATTR_ORACLE_NULLS"        => PDO::NULL_TO_STRING,
    "PDO::ATTR_STRINGIFY_FETCHES"   => true,
    "PDO::ATTR_TIMEOUT"             => 10,
    "PDO::ATTR_DEFAULT_FETCH_MODE"  => PDO::FETCH_LAZY
);

try {
    $db = new PDO($dsn);

    foreach ($attributes as $attr => $val) {
        if(!$db->setAttribute(constant($attr), $val))
            print("Could not set attribute $attr" . "\n");
    }

    foreach ($attributes as $attr => $val){
        if(!$db->getAttribute(constant($attr)) !== $val)
            print("$attr did not register new value" . "\n");
    }

} catch (PDOException $e) {
    print $e->getMessage();
}
?>

--EXPECT--

--XFAIL--
Must discuss which attributes we can support setting, see issue #66. 
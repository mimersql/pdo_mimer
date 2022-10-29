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

--EXPECTF--
Could not set attribute PDO::ATTR_TIMEOUT
PDO::ATTR_CASE did not register new value
PDO::ATTR_ERRMODE did not register new value
PDO::ATTR_ORACLE_NULLS did not register new value

Warning: PDO::getAttribute(): SQLSTATE[IM001]: Driver does not support this function: driver does not support that attribute in %stests%epdo_setAttribute_basic1.php on line 23

Warning: PDO::getAttribute(): SQLSTATE[IM001]: Driver does not support this function: driver does not support that attribute in %stests%epdo_setAttribute_basic1.php on line 23
PDO::ATTR_TIMEOUT did not register new value
PDO::ATTR_DEFAULT_FETCH_MODE did not register new value

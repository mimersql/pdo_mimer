--TEST--
Mimer SQL (Connection): Connect to DB with invalid DSN option

--EXTENSIONS--
pdo_mimer

--DESCRIPTION--
Intended behaviour by PDO seems to be to ignore invalid DSN options. 

--SKIPIF--
<?php require('skipif.inc'); ?>

--FILE--
<?php
require("testdb.inc");
try {
    $dbh = new PDO("mimer:non-existing-option=val", PDO_MIMER_TEST_USER, PDO_MIMER_TEST_PASS);
    print("done");
} catch (PDOException $e) {
    print "PDOException was thrown";
}

?>
--EXPECT--
done

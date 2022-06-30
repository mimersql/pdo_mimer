<?php

// Test defaults, defaults to the following:
//
// Username: 	SYSADM
// Password:	SYSADM
// Database:	<nothing>
//
// A DSN must be specified by using PDO_MIMER_TEST_DSN

define('PDO_MIMER_TEST_USER', getenv('PDO_MIMER_TEST_USER') ?: 'SYSADM');
define('PDO_MIMER_TEST_PASS', getenv('PDO_MIMER_TEST_PASS') ?: 'SYSADM');
define('PDO_MIMER_TEST_DSN', getenv('PDO_MIMER_TEST_DSN') ?: '');

if(!PDO_MIMER_TEST_DSN)
{
        die('Error: PDO_MIMER_TEST_DSN must be set');
}

?>

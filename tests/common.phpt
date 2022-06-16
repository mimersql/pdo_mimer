--TEST--
MIMER
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded('pdo_mimer')) print 'skip'; ?>
--REDIRECTTEST--
if (false !== getenv('PDO_MIMER_TEST_DSN')) {
# user set them from their shell
   $config['ENV']['PDOTEST_DSN'] = getenv('PDO_MIMER_TEST_DSN');
   $config['ENV']['PDOTEST_USER'] = getenv('PDO_MIMER_TEST_USER');
   $config['ENV']['PDOTEST_PASS'] = getenv('PDO_MIMER_TEST_PASS');
   if (false !== getenv('PDO_MIMER_TEST_ATTR')) {
      $config['ENV']['PDOTEST_ATTR'] = getenv('PDO_MIMER_TEST_ATTR');
   }
   return $config;
}
return array(
   'ENV' => array(
           'PDOTEST_DSN' => 'mimer:dsn',
           'PDOTEST_USER' => 'username',  # change to ident
           'PDOTEST_PASS' => 'password'   # change to ident password
       ),
   'TESTS' => 'ext/pdo/tests'
   );
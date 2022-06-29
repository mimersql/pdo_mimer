--TEST--
Mimer SQL
--SKIPIF--
<?php
if (!extension_loaded('pdo') || !extension_loaded('pdo_mimer')) print 'skip not loaded';
?>
--REDIRECTTEST--
# magic auto-configuration

$config = array(
  'TESTS' => getenv('TEST_DIR') ?: 'ext/pdo/tests'
);

if (false !== getenv('PDO_MIMER_TEST_DSN')) {
  # user set them from their shell
  $config['ENV']['PDOTEST_DSN'] = getenv('PDO_MIMER_TEST_DSN');
  $config['ENV']['PDOTEST_USER'] = getenv('PDO_MIMER_TEST_USER');
  $config['ENV']['PDOTEST_PASS'] = getenv('PDO_MIMER_TEST_PASS');
  if (false !== getenv('PDO_MIMER_TEST_ATTR')) {
    $config['ENV']['PDOTEST_ATTR'] = getenv('PDO_MIMER_TEST_ATTR');
  }
} else {
  $config['ENV']['PDOTEST_DSN'] = 'mimer:';
  $config['ENV']['PDOTEST_USER'] = getenv('USER');
  $config['ENV']['PDOTEST_PASS'] = '';
}

return $config;

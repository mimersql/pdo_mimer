#!/usr/bin/env php
<?php
const PDO_MIMER_EMPTY_DSN = 'mimer:';

$pdo_mimer_env_list = [
    'DSN'         => PDO_MIMER_EMPTY_DSN,
    'USER'        => null,
    'PASS'        => null,
    'ATTR'        => null,
];

foreach ($pdo_mimer_env_list as $key => &$value) {
    if ($env = getenv("PDO_MIMER_TEST_$key"))
        $value = $env;
    $config['ENV']["PDOTEST_$key"] = $value;
}
unset($value); unset($env);
foreach ($config['ENV'] as $key => $value)
    putenv("$key=$value");
unset($value); unset($key);

define('PDO_MIMER_TEST_DSN',          $pdo_mimer_env_list['DSN']);
define('PDO_MIMER_TEST_USER',         $pdo_mimer_env_list['USER']);
define('PDO_MIMER_TEST_PASS',         $pdo_mimer_env_list['PASS']);
define('PDO_MIMER_TEST_ATTR',         $pdo_mimer_env_list['ATTR']);
define('PDO_MIMER_TEST_SKIP_OSUSER', (bool)getenv('PDO_MIMER_TEST_SKIP_OSUSER'));

?>

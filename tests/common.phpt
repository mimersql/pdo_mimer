--TEST--
Mimer SQL
--EXTENSIONS--
pdo
pdo_mimer
--SKIPIF--
<?php !is_dir('ext/pdo/tests') && die('skip PDO tests directory not found'); ?>
--REDIRECTTEST--
# magic auto-configuration

return ['ENV' => [
            'PDOTEST_DSN'  => getenv('PDO_MIMER_TEST_DSN')  ?: 'mimer:',
            'PDOTEST_USER' => getenv('PDO_MIMER_TEST_USER') ?: '',
            'PDOTEST_PASS' => getenv('PDO_MIMER_TEST_PASS') ?: '',
        ], 'TESTS'   => 'ext/pdo/tests',
];

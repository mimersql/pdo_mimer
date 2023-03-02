--TEST--
Mimer SQL

--EXTENSIONS--
pdo
pdo_mimer

--DESCRIPTION--
Redirects the test execution to the tests in ext/pdo/tests,
i.e. the common PDO tests. Returns some information 
needed for the common tests to connect to the test DB.

--REDIRECTTEST--
$test_settings = ['ENV' => [
            'PDOTEST_DSN'  => getenv('PDOMIMER_TEST_DSN') ?: 'mimer:',
            'PDOTEST_USER' => getenv('PDOMIMER_TEST_USER') ?: 'MIMER_ADM',
            'PDOTEST_PASS' => getenv('PDOMIMER_TEST_PASS') ?: 'adm',
        ], 'TESTS'   => is_dir('ext/pdo/tests') ? 'ext/pdo/tests' : getenv('REDIR_TEST_DIR'),
];

return $test_settings;

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
require_once __DIR__ . '/tests/pdo_tests_util.inc';

$util = new PDOMimerTestUtil();

$test_settings = ['ENV' => [
            'PDOTEST_DSN'  => $util->getFullDSN(),
            'PDOTEST_USER' => $util->getConfigValue("connection->dsn->user"),
            'PDOTEST_PASS' => $util->getConfigValue("connection->dsn->password"),
        ], 'TESTS'   => is_dir('ext/pdo/tests') ? 'ext/pdo/tests' : getenv('REDIR_TEST_DIR'),
];

return $test_settings;

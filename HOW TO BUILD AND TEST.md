# How to build PDO Mimer

Steps for building PDO Mimer

### Prerequisites

* PHP (Only tested with v8.1.7)
* PDO extension (should be included with your PHP installation)
* Mimer Library
* PDO Mimer source code (developement branch as of 16/06-2022)

## Standalone

###### Build
```shell
cd pdo_mimer
phpize  # Generates the 'configure' script
./configure --with-pdo-mimer
make
make install
```

###### Test
```shell
php -d extension=pdo_mimer.so -r "echo 'PDO Mimer is enabled';"
```

###### Other tests
You can run `make test` but no tests are available as of 16/06-2022.

## With PHP Source

##### Developement essentials
```shell
sudo apt update && sudo apt install build-essential autoconf automake bison \
    flex re2c gdb libtool make pkgconf valgrind git libxml2-dev libsqlite3-dev
```

##### Clone the repo and checkout branch v8.1.7
```shell
git clone https://github.com/php/php-src && cd php-src
git checkout tags/php-8.1.7 -b 8.1.7-dev
```

##### Copy PDO Mimer to PHP source code
```shell
cp -r /path/to/pdo_mimer /path/to/php-src/ext/
cd /path/to/php-src/ext/pdo_mimer && phpize --clean  # only do this step if you built PDO Mimer standalone
```

##### Build
```shell
cd /path/to/php-src
./buildconf --force
./configure --with-pdo-mimer
make -j$(nproc)
```

##### The test file

The test file `ext/pdo_mimer/tests/common.phpt` redirects the test execution to the test scripts in the `ext/pdo/tests` directory, which contains tests common for all PDO drivers. Before redirection it sets some values in an array, in order to allow the common tests to use a mimer database.  

###### ext/pdo_mimer/tests/common.phpt
```phpt
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
```

###### Set environment variables
```shell
export PDO_MIMER_TEST_DSN=mimer:dsn  # leave as is
export PDO_MIMER_TEST_USER=username  # change to ident
export PDO_MIMER_TEST_PASS=password  # change to ident password
```
Change the environment variable values to a database with valid credentials or change them in the file.


##### Test
```shell
make TESTS=ext/pdo_mimer/tests test  # runs only pdo_mimer tests
make test  # runs all tests
```
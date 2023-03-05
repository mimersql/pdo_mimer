# How to build PDO Mimer

Steps for building PDO Mimer

### Prerequisites

* PHP (Only tested with v8.1.7)
* PDO extension (should be included with your PHP installation)
* Mimer Library
* PDO Mimer source code (development branch as of 16/06-2022)

## Standalone

###### Build
```shell
cd pdo_mimer
phpize  # Generates the 'configure' script
./configure
make
sudo make install # installs to /usr/lib/php/<VERSION>/
```

###### Test
```shell
php -d extension=pdo_mimer.so -r "echo 'PDO Mimer is enabled';"
```

###### Other tests
No PDO Mimer-specific tests are available as of 23/06-2022.

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
cd /path/to/php-src/ext/pdo_mimer
phpize --clean  # you only need to do this step if you built PDO Mimer standalone
```

##### Build
```shell
cd /path/to/php-src
./buildconf --force
./configure --with-pdo-mimer --enable-debug  # --enable-debug is optional but is a necessity for debugging
make -j$(nproc)
```

##### The test file

The test file `pdo_mimer/tests/common.phpt` tells `make test` to run the common PDO tests in the `ext/pdo/tests` directory.

###### Set environment variables
```shell
export PDO_MIMER_TEST_DSN=mimer:dbname=testdb  # change `testdb` to a valid database for testing purposes
export PDO_MIMER_TEST_USER=username  # change to ident
export PDO_MIMER_TEST_PASS=password  # change to ident password
export NO_INTERACTION=1  # to suppress the interactive prompts
export TEST_PHP_DETAILED=1  # to get a more detailed look at the test results
```

**A tip:** make a `.env` file with the above variables and source it in your `.bashrc` file or add it to your run 
configurations for testing.

##### Test
```shell
make test TESTS=ext/pdo_mimer/tests test  # runs only common PDO tests on PDO Mimer
make test  # runs all PHP tests
```

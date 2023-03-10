# PDO driver for Mimer SQL

The official PHP PDO extension for Mimer SQL

## Requirements
- PHP 8.1+
- Mimer SQL v11.0+
- `apt install php-dev` or equivalent

## Resources
- [PHP](https://www.php.net/manual/en/)
- [Installation on Unix Systems](https://www.php.net/manual/en/install.unix.php)
- [PHP for Windows](https://windows.php.net/)
- [Manual PHP Installation on Windows](https://www.php.net/manual/en/install.windows.manual.php)
- [Building for Windows](https://wiki.php.net/internals/windows/stepbystepbuild)
- [PDO](https://www.php.net/manual/en/class.pdostatement.php)
- [pickle](https://github.com/FriendsOfPHP/pickle) 


## Installation

### Using [pickle](https://github.com/FriendsOfPHP/pickle/releases/latest)
```sh
php pickle.phar install --source https://github.com/mimersql/pdo_mimer
```

### Releases for Windows
Check out the [releases](https://github.com/mimersql/pdo_mimer/releases/latest) to download library files for Windows (64-bit only). Releases include files for both NTS and TS versions for PHP versions 8.1+.

### Building from source
#### Unix
```sh
git clone https://github.com/mimersql/pdo_mimer && cd pdo_mimer
phpize
./configure
make
make install
```

#### Windows

##### Requirements
- [Visual Studio Build Tools 2019](https://my.visualstudio.com/Downloads?q=visual%20studio%202019&wt.mc_id=o~msft~vscom~older-downloads)
- [PHP SDK Development Pack](https://windows.php.net/download/) (PHP 8.1+)
- [PHP SDK Binary Tools](https://github.com/php/php-sdk-binary-tools)

##### 1. Visual Studio Build Tools 2019
Install Visual Studio 2019. Use the image below as a reference. 

![](https://iili.io/HXoBhrl.png)

##### 2. PHP SDK Development Pack
Unpack the PHP SDK development pack. This guide will use `C:\php-8.2.3-devel-vs16-x64`, replace this with the path to the directory of the unpacked files.

##### 3. PHP SDK Binary Tools
The PHP SDK binary tools enable us to build the PDO Mimer extension.
```sh
git clone https://github.com/php/php-sdk-binary-tools C:\php-sdk-binary-tools && cd C:\php-sdk-binary-tools
git checkout php-sdk-2.2.0 -b php-sdk-2.2.0
.\phpsdk-vs16-x64.bat
phpsdk_buildtree pdo_mimer
git clone https://github.com/mimersql/pdo_mimer && cd pdo_mimer
C:\php-8.2.3-devel-vs16-x64\phpize.bat
configure --with-pdo-mimer
nmake
nmake install
```

> ⚠️ **Note**
> 
> - The developers of the PHP SDK Binary Tools recommend having the `php-sdk-binary-tools` directory as close to `C:\` as possible, with no spaces in the path name.
> - This method only seems to work with non-thread-safe versions of PHP. For thread-safe versions, one must build PDO Mimer with the PHP source code simulataneously.
> - Use `--with-prefix` and the path to a PHP installation to install the `pdo_mimer` library directly to the extensions directory.
> 
> If you get any errors indicating `mimapi64.lib` or `mimerapi.h` not found, add the following options while configuring:
> - `--with-extra-libs` and the path to a Mimer SQL installation lib directory (eg. `C:\Program Files\Mimer SQL Experience 11.0\dev\lib\amd64`)
> - `--with-extra-includes` and the path to a Mimer SQL installation include directory (eg. `C:\Program Files\Mimer SQL Experience 11.0\dev\include`)

## Enable the extension
Ensure the `pdo_mimer` shared library is installed in the extensions folder, `/path/to/php/ext`. The library file is named differently depending on the platform:
|    OS   |       Filename      |
| :------ | :------------------ |
| Windows | `php_pdo_mimer.dll` |
| Linux   | `pdo_mimer.so`      |
| MacOS   | `pdo_mimer.dylib`   |

Enabling the extension can be done in different ways:
- add `extension=pdo_mimer` to your `php.ini` config
- use `php -dextension=pdo_mimer`

## Verify PDO Mimer installation
```sh
$ php -m | grep mimer
pdo_mimer
```

### With the `readline` extension installed
```sh
$ php -a
Interactive shell

php > $db = new PDO('mimer:dbname=db', 'user', 'pass');
php > var_dump($db->query('SELECT * FROM SYSTEM.ONEROW')->fetchAll());
array(1) {
  [0]=>
  array(2) {
    ["M"]=>
    string(1) "M"
    [0]=>
    string(1) "M"
  }
}
php > 
```

## PDO Mimer API

### [PDOStatement](https://www.php.net/manual/en/class.pdostatement.php)

#### `mimerAddBatch`

```php
void PDOStatement::mimerAddBatch();
```

- Used on a prepared statement
- When using placeholders, all parameters must be bound with values before calling `mimerAddBatch`
- Should not be used after the **last** `bindValue()` or `bindParam()`, see example below

##### Example
```php
$users = [
    [1, 'John Smith'],
    [2, 'Jane Doe'],
];

$stmt = $db->prepare('INSERT INTO customers (id, name) VALUES (?, ?)');

foreach ($users as $i => [$id, $name]) {
    $stmt->bindValue(1, $id, PDO::PARAM_INT);
    $stmt->bindValue(2, $name, PDO::PARAM_STR);

    if ($i != array_key_last($users)) // do not execute if on last item
        $stmt->mimerAddBatch();
}

$stmt->execute();
```

> ⚠️ **Note:**
>
> Running
> ```php
> $stmt->mimerAddBatch();
> ```
> on the last item will throw an exception:
>`-24103 Incomplete set of input parameters when executing a statement or opening a cursor`
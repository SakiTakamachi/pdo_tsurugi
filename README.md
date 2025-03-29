# PDO TSURUGI

This is a PDO driver for [Tsurugi DB](https://github.com/project-tsurugi/tsurugidb).
The minimum functionality has been implemented, but it is still in development.

# How to use the development version

1. Clone this repository

```
git clone https://github.com/SakiTakamachi/pdo_tsurugi.git
```

2. Go to the src directory and run `phpize`

```
cd pdo_tsurugi/src
phpize
```

3. Run `configure`. At this time, pass the path to the `tsubakuro-rust` directory as an option.
For instructions on how to install `tubakuro-rust`, please refer to [tsubakuro-rust repository](https://github.com/project-tsurugi/tsubakuro-rust).

```
./configure --with-pdo_tsurugi=/tsubakuro-rust
```

4. Build and install

```
make
make install
```

5. Add the following to php.ini:

```
extension=pdo_tsurugi.so
```

# Code Example

```
<?php

use Pdo\Tsurugi;
use Pdo\Tsurugi\TransactionType;

$db = new Tsurugi('tsurugi:endpoint=tcp://tsurugi:12345', [
    Tsurugi::TRANSACTION_TYPE => TransactionType::Short,
]);

$db->exec('CREATE TABLE test_1 (id INT PRIMARY KEY, name VARCHAR(255))');

$db->exec("INSERT INTO test_1 (id, name) VALUES (1, 'test name')");

$stmt = $db->prepare("INSERT INTO test_1 (id, name) VALUES (2, :name)");
$stmt->bindValue(':name', "test 'name' 2");
$stmt->execute();

$stmt = $db->query('SELECT * FROM test_1');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
```

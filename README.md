# PDO TSURUGI

[![Latest Stable Version](https://poser.pugx.org/pdo-tsurugi/pdo-tsurugi/v)](https://packagist.org/packages/pdo-tsurugi/pdo-tsurugi)
[![License](https://poser.pugx.org/pdo-tsurugi/pdo-tsurugi/license)](https://packagist.org/packages/pdo-tsurugi/pdo-tsurugi)
[![PHP Version Require](https://poser.pugx.org/pdo-tsurugi/pdo-tsurugi/require/php)](https://packagist.org/packages/pdo-tsurugi/pdo-tsurugi)

This is a PDO driver for [Tsurugi DB](https://github.com/project-tsurugi/tsurugidb).
This extension requires [tsubakuro-rust](https://github.com/project-tsurugi/tsubakuro-rust). Please refer to the [tsubakuro-rust-ffi C example](https://github.com/project-tsurugi/tsubakuro-rust/tree/master/tsubakuro-rust-ffi/example/c) for instructions on how to install tsubakuro-rust.

The minimum functionality has been implemented, but it is still in development.

## Classes

This extension includes the following classes:

```
Pdo\Tsurugi
Pdo\Tsurugi\TransactionType
Pdo\Tsurugi\NamedPlaceholders
Pdo\Tsurugi\PositionalPlaceholders
```

## How to use the development version using PIE

```
pie install pdo-tsurugi/pdo-tsurugi:dev-master --with-lib-dir=YOUR_TSUBAKURO_RUST_DIR
```

## Code Example

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

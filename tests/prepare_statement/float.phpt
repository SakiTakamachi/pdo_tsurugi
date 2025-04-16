--TEST--
prepare statement REAL, FLOAT and DOUBLE
--FILE--
<?php

use Pdo\Tsurugi;
use Pdo\Tsurugi\NamedPlaceholders;
use Pdo\Tsurugi\PositionalPlaceholders;

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_prepare_statement_float (a REAL, b FLOAT, c DOUBLE)');

$stmt = $db->prepare("INSERT INTO test_prepare_statement_float (a, b, c) VALUES (:a, :b, :c)", [
    Tsurugi::PLACEHOLDERS => new NamedPlaceholders()
        ->addFloat(':a')
        ->addFloat(':b')
        ->addDouble(':c'),
]);

$stmt->bindValue(':a', null, PDO::PARAM_INT);
$stmt->bindValue(':b', 0, PDO::PARAM_INT);
$stmt->bindValue(':c', null, PDO::PARAM_STR);
$stmt->execute();

$stmt->bindValue(':a', 1.1, PDO::PARAM_STR);
$stmt->bindValue(':b', 2.1, PDO::PARAM_STR);
$stmt->bindValue(':c', 3.1, PDO::PARAM_STR);
$stmt->execute();

$a = -1.1;
$b = -2.1;
$c = -3.1;

$stmt->bindParam(':a', $a, PDO::PARAM_STR);
$stmt->bindParam(':b', $b, PDO::PARAM_STR);
$stmt->bindParam(':c', $c, PDO::PARAM_STR);
$stmt->execute();

$stmt = $db->prepare("INSERT INTO test_prepare_statement_float (a, b, c) VALUES (?, ?, ?)", [
    Tsurugi::PLACEHOLDERS => new PositionalPlaceholders()
        ->addDouble(3)
        ->addFloat(1)
        ->addFloat(2),
]);

$a = 1.23E-3;
$b = 1.23456E-9;
$c = 1.23456789123456789E-9;

$stmt->bindParam(1, $a, PDO::PARAM_STR);
$stmt->bindParam(2, $b, PDO::PARAM_STR);
$stmt->bindParam(3, $c, PDO::PARAM_STR);
$stmt->execute();

$stmt = $db->query('SELECT * FROM test_prepare_statement_float');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_prepare_statement_float');
?>
--EXPECT--
array(4) {
  [0]=>
  array(3) {
    ["a"]=>
    NULL
    ["b"]=>
    float(0)
    ["c"]=>
    NULL
  }
  [1]=>
  array(3) {
    ["a"]=>
    float(1.100000023841858)
    ["b"]=>
    float(2.0999999046325684)
    ["c"]=>
    float(3.1)
  }
  [2]=>
  array(3) {
    ["a"]=>
    float(-1.100000023841858)
    ["b"]=>
    float(-2.0999999046325684)
    ["c"]=>
    float(-3.1)
  }
  [3]=>
  array(3) {
    ["a"]=>
    float(0.001230000052601099)
    ["b"]=>
    float(1.2345600097773968E-9)
    ["c"]=>
    float(1.2345678912346E-9)
  }
}

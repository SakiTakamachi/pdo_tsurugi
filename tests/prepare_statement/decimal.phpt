--TEST--
prepare statement DECIMAL
--FILE--
<?php

use Pdo\Tsurugi;
use Pdo\Tsurugi\NamedPlaceholders;

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_prepare_statement_decimal (a DECIMAL(5, 0), b DECIMAL(36, 0), c DECIMAL(30, 28))');

$stmt = $db->prepare("INSERT INTO test_prepare_statement_decimal (a, b, c) VALUES (:a, :b, :c)", [
    Tsurugi::PLACEHOLDERS => new NamedPlaceholders()
        ->addDecimal(':a')
        ->addDecimal(':b')
        ->addDecimal(':c'),
]);

$stmt->bindValue(':a', 15, PDO::PARAM_STR);
$stmt->bindValue(':b', '123456789012345678901234567890123456', PDO::PARAM_STR);
$stmt->bindValue(':c', '1.234567890123456789012345', PDO::PARAM_STR);
$stmt->execute();

$stmt->bindValue(':a', '00', PDO::PARAM_STR);
$stmt->bindValue(':b', '00', PDO::PARAM_STR);
$stmt->bindValue(':c', '0.234567890123456789012345', PDO::PARAM_STR);
$stmt->execute();

$a = -15;
$b = '-123456789012345678901234567890123456';
$c = '-1.234567890123456789012345';

$stmt->bindParam(':a', $a, PDO::PARAM_STR);
$stmt->bindParam(':b', $b, PDO::PARAM_STR);
$stmt->bindParam(':c', $c, PDO::PARAM_STR);
$stmt->execute();

$stmt->bindValue(':a', null, PDO::PARAM_STR);
$stmt->bindValue(':b', null, PDO::PARAM_STR);
$stmt->bindValue(':c', null, PDO::PARAM_NULL);
$stmt->execute();

$stmt->bindValue(':a', 0, PDO::PARAM_STR);
$stmt->bindValue(':b', 0, PDO::PARAM_STR);
$stmt->bindValue(':c', 0, PDO::PARAM_STR);
$stmt->execute();

$stmt = $db->query('SELECT * FROM test_prepare_statement_decimal');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_prepare_statement_decimal');
?>
--EXPECT--
array(5) {
  [0]=>
  array(3) {
    ["a"]=>
    string(2) "15"
    ["b"]=>
    string(36) "123456789012345678901234567890123456"
    ["c"]=>
    string(30) "1.2345678901234567890123450000"
  }
  [1]=>
  array(3) {
    ["a"]=>
    string(1) "0"
    ["b"]=>
    string(1) "0"
    ["c"]=>
    string(30) "0.2345678901234567890123450000"
  }
  [2]=>
  array(3) {
    ["a"]=>
    string(3) "-15"
    ["b"]=>
    string(37) "-123456789012345678901234567890123456"
    ["c"]=>
    string(31) "-1.2345678901234567890123450000"
  }
  [3]=>
  array(3) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
    ["c"]=>
    NULL
  }
  [4]=>
  array(3) {
    ["a"]=>
    string(1) "0"
    ["b"]=>
    string(1) "0"
    ["c"]=>
    string(30) "0.0000000000000000000000000000"
  }
}

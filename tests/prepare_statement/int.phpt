--TEST--
prepare statement INT and BIGINT
--FILE--
<?php

use Pdo\Tsurugi;
use Pdo\Tsurugi\NamedPlaceholders;
use Pdo\Tsurugi\PositionalPlaceholders;

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_prepare_statement_int (a INT, b BIGINT)');

$stmt = $db->prepare("INSERT INTO test_prepare_statement_int (a, b) VALUES (:a, :b)", [
    Tsurugi::PLACEHOLDERS => new NamedPlaceholders()
        ->addInteger(':a')
        ->addBigInteger(':b'),
]);

$stmt->bindValue(':a', 0, PDO::PARAM_INT);
$stmt->bindValue(':b', null, PDO::PARAM_STR);
$stmt->execute();

$stmt->bindValue(':a', 1, PDO::PARAM_INT);
$stmt->bindValue(':b', 2, PDO::PARAM_INT);
$stmt->execute();

$stmt = $db->prepare("INSERT INTO test_prepare_statement_int (a, b) VALUES (:a, :b)", [
    Tsurugi::PLACEHOLDERS => new PositionalPlaceholders()
        ->addInteger(1)
        ->addInteger(2),
]);
$stmt->bindValue(':a', null, PDO::PARAM_INT);
$stmt->bindValue(':b', 0, PDO::PARAM_STR);
$stmt->execute();

$a = -1;
$b = -2;

$stmt->bindParam(':a', $a, PDO::PARAM_INT);
$stmt->bindParam(':b', $b, PDO::PARAM_INT);
$stmt->execute();

$stmt = $db->query('SELECT * FROM test_prepare_statement_int');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_prepare_statement_int');
?>
--EXPECT--
array(4) {
  [0]=>
  array(2) {
    ["a"]=>
    int(0)
    ["b"]=>
    NULL
  }
  [1]=>
  array(2) {
    ["a"]=>
    int(1)
    ["b"]=>
    int(2)
  }
  [2]=>
  array(2) {
    ["a"]=>
    NULL
    ["b"]=>
    int(0)
  }
  [3]=>
  array(2) {
    ["a"]=>
    int(-1)
    ["b"]=>
    int(-2)
  }
}

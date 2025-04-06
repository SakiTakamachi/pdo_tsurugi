--TEST--
prepare statement BINARY, VARBINARY, BINARY VARYING
--FILE--
<?php

use Pdo\Tsurugi;
use Pdo\Tsurugi\NamedPlaceholders;

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_prepare_statement_binary (a BINARY(8), b VARBINARY(8), c BINARY VARYING(8))');

$stmt = $db->prepare("INSERT INTO test_prepare_statement_binary (a, b, c) VALUES (:a, :b, :c)", [
    Tsurugi::PLACEHOLDERS => new NamedPlaceholders()
        ->addBinary(':a')
        ->addBinary(':b')
        ->addBinary(':c'),
]);

$stmt->bindValue(':a', null, PDO::PARAM_BOOL);
$stmt->bindValue(':b', null, PDO::PARAM_INT);
$stmt->bindValue(':c', null, PDO::PARAM_STR);
$stmt->execute();

$stmt->bindValue(':a', '0x0f1a', PDO::PARAM_STR);
$stmt->bindValue(':b', 'be1e', PDO::PARAM_STR);
$stmt->bindValue(':c', '003C94', PDO::PARAM_STR);
$stmt->execute();

$stmt = $db->query('SELECT * FROM test_prepare_statement_binary');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_prepare_statement_binary');
?>
--EXPECT--
array(2) {
  [0]=>
  array(3) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
    ["c"]=>
    NULL
  }
  [1]=>
  array(3) {
    ["a"]=>
    string(16) "0f1a000000000000"
    ["b"]=>
    string(4) "be1e"
    ["c"]=>
    string(6) "003c94"
  }
}

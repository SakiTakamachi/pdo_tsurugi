--TEST--
rowCount
--FILE--
<?php

use Pdo\Tsurugi;
use Pdo\Tsurugi\NamedPlaceholders;

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_row_count (a INT)');

$db->exec('INSERT INTO test_row_count (a) VALUES (1)');

$stmt = $db->prepare('INSERT INTO test_row_count (a) VALUES (:a)');
$stmt->bindValue(':a', 5, PDO::PARAM_INT);
$stmt->execute();
var_dump($stmt->rowCount());

$stmt = $db->prepare('UPDATE test_row_count SET a=:a', [
    Tsurugi::PLACEHOLDERS => new NamedPlaceholders()
        ->addInteger(':a'),
]);
$stmt->bindValue(':a', 10, PDO::PARAM_INT);
$stmt->execute();
var_dump($stmt->rowCount());
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_row_count');
?>
--EXPECT--
int(1)
int(2)

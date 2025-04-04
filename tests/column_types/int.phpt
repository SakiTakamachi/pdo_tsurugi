--TEST--
column type INT and BIGINT
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_column_types_int (a INT, b BIGINT)');

$db->exec("INSERT INTO test_column_types_int (a, b) VALUES (0, 0)");
$db->exec("INSERT INTO test_column_types_int (a, b) VALUES (null, null)");

$stmt = $db->prepare("INSERT INTO test_column_types_int (a, b) VALUES (:a, :b)");
$stmt->bindValue(':a', 1, PDO::PARAM_INT);
$stmt->bindValue(':b', 2, PDO::PARAM_INT);
$stmt->execute();

$a = -1;
$b = -2;
$stmt = $db->prepare("INSERT INTO test_column_types_int (a, b) VALUES (:a, :b)");
$stmt->bindParam(':a', $a, PDO::PARAM_INT);
$stmt->bindParam(':b', $b, PDO::PARAM_INT);
$stmt->execute();

$stmt = $db->prepare("INSERT INTO test_column_types_int (a, b) VALUES (:a, :b)");
$stmt->bindValue(':a', null, PDO::PARAM_INT);
$stmt->bindValue(':b', null, PDO::PARAM_NULL);
$stmt->execute();

$a = null;
$b = null;
$stmt = $db->prepare("INSERT INTO test_column_types_int (a, b) VALUES (:a, :b)");
$stmt->bindParam(':a', $a, PDO::PARAM_NULL);
$stmt->bindParam(':b', $b, PDO::PARAM_INT);
$stmt->execute();

$stmt = $db->query('SELECT * FROM test_column_types_int');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_column_types_int');
?>
--EXPECT--
array(6) {
  [0]=>
  array(2) {
    ["a"]=>
    int(0)
    ["b"]=>
    int(0)
  }
  [1]=>
  array(2) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
  }
  [2]=>
  array(2) {
    ["a"]=>
    int(1)
    ["b"]=>
    int(2)
  }
  [3]=>
  array(2) {
    ["a"]=>
    int(-1)
    ["b"]=>
    int(-2)
  }
  [4]=>
  array(2) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
  }
  [5]=>
  array(2) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
  }
}

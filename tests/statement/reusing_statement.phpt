--TEST--
Reusing statement
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_reusing_statement (a INT, b VARCHAR(10))');

$db->exec("INSERT INTO test_reusing_statement  (a, b) VALUES (1, 'one')");
$db->exec("INSERT INTO test_reusing_statement  (a, b) VALUES (2, 'two')");

$stmt = $db->prepare('SELECT * FROM test_reusing_statement');
$stmt->execute();
$stmt->execute();

var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_reusing_statement');
?>
--EXPECT--
array(2) {
  [0]=>
  array(2) {
    ["a"]=>
    int(1)
    ["b"]=>
    string(3) "one"
  }
  [1]=>
  array(2) {
    ["a"]=>
    int(2)
    ["b"]=>
    string(3) "two"
  }
}

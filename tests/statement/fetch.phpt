--TEST--
fetch
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_fetch (a INT, b VARCHAR(10))');

$db->exec("INSERT INTO test_fetch (a, b) VALUES (1, 'one')");
$db->exec("INSERT INTO test_fetch (a, b) VALUES (2, 'two')");
$db->exec("INSERT INTO test_fetch (a, b) VALUES (3, 'three')");

$stmt = $db->query('SELECT * FROM test_fetch');

var_dump(
    $stmt->fetch(PDO::FETCH_BOTH),
    $stmt->fetch(PDO::FETCH_NUM),
    $stmt->fetch(PDO::FETCH_ASSOC),
    $stmt->fetch(),
);
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_fetch');
?>
--EXPECT--
array(4) {
  ["a"]=>
  int(1)
  [0]=>
  int(1)
  ["b"]=>
  string(3) "one"
  [1]=>
  string(3) "one"
}
array(2) {
  [0]=>
  int(2)
  [1]=>
  string(3) "two"
}
array(2) {
  ["a"]=>
  int(3)
  ["b"]=>
  string(5) "three"
}
bool(false)

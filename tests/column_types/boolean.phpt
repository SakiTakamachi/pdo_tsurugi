--TEST--
column type BOOLEAN (provisional)
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

// Since the BOOLEAN type cannot be specified in the CREATE statement,
// the value is obtained using a comparison expression.
$db->exec('CREATE TABLE IF NOT EXISTS test_column_types_boolean (a INT)');

$db->exec("INSERT INTO test_column_types_boolean (a) VALUES (5)");
$db->exec("INSERT INTO test_column_types_boolean (a) VALUES (10)");
$db->exec("INSERT INTO test_column_types_boolean (a) VALUES (NULL)");

$stmt = $db->query('SELECT a=5 AS bool_column FROM test_column_types_boolean');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_column_types_boolean');
?>
--EXPECT--
array(3) {
  [0]=>
  array(1) {
    ["bool_column"]=>
    bool(true)
  }
  [1]=>
  array(1) {
    ["bool_column"]=>
    bool(false)
  }
  [2]=>
  array(1) {
    ["bool_column"]=>
    NULL
  }
}

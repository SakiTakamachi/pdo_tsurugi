--TEST--
column type BINARY, VARBINARY, BINARY VARYING
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

// Due to the format of the literal, you cannot bind parameters.
$db->exec('CREATE TABLE IF NOT EXISTS test_column_types_binary (a BINARY(8), b VARBINARY(8), c BINARY VARYING(8))');

$db->exec("INSERT INTO test_column_types_binary (a, b, c) VALUES (X'0f1a', x'be1e', x'3c94')");
$db->exec("INSERT INTO test_column_types_binary (a, b, c) VALUES (NULL, NULL, NULL)");

$stmt = $db->query('SELECT * FROM test_column_types_binary');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_column_types_binary');
?>
--EXPECT--
array(2) {
  [0]=>
  array(3) {
    ["a"]=>
    string(16) "0f1a000000000000"
    ["b"]=>
    string(4) "be1e"
    ["c"]=>
    string(4) "3c94"
  }
  [1]=>
  array(3) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
    ["c"]=>
    NULL
  }
}

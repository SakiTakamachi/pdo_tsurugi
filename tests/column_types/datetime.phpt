--TEST--
column type DATE, TIME, TIMESTAMP and TIMESTAMP WITH TIME ZONE
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

// TIMESTAMP WITHOUT TIME ZONE is not a supported literal.
// Due to the format of the literal, you cannot bind parameters.
$db->exec('CREATE TABLE IF NOT EXISTS test_column_types_datetime (a DATE, b TIME, c TIMESTAMP, d TIMESTAMP WITH TIME ZONE)');

$db->exec(<<<SQL
INSERT INTO test_column_types_datetime (a, b, c, d)
VALUES (DATE '2024-10-01', TIME '16:54:03', TIMESTAMP '2025-03-01 12:34:48', TIMESTAMP WITH TIME ZONE '2025-03-01 12:34:48+09:00')
SQL);
$db->exec("INSERT INTO test_column_types_datetime (a, b, c, d) VALUES (null, null, null, null)");

$stmt = $db->query('SELECT * FROM test_column_types_datetime');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_column_types_datetime');
?>
--EXPECT--
array(2) {
  [0]=>
  array(4) {
    ["a"]=>
    string(10) "2024-10-01"
    ["b"]=>
    string(8) "16:54:03"
    ["c"]=>
    string(19) "2025-03-01 12:34:48"
    ["d"]=>
    string(25) "2025-03-01 03:34:48+00:00"
  }
  [1]=>
  array(4) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
    ["c"]=>
    NULL
    ["d"]=>
    NULL
  }
}

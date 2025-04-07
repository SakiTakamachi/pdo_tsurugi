--TEST--
fetchColumn
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_fetch_column (a INT, b VARCHAR(10))');

$db->exec("INSERT INTO test_fetch_column (a, b) VALUES (1, 'one')");
$db->exec("INSERT INTO test_fetch_column (a, b) VALUES (2, 'two')");
$db->exec("INSERT INTO test_fetch_column (a, b) VALUES (3, 'three')");

$stmt = $db->query('SELECT * FROM test_fetch_column');

var_dump(
    $stmt->fetchColumn(0),
    $stmt->fetchColumn(1),
    $stmt->fetchColumn(0),
    $stmt->fetchColumn(1),
);
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_fetch_column');
?>
--EXPECT--
int(1)
string(3) "two"
int(3)
bool(false)

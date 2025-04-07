--TEST--
columnCount
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_column_count (a INT, b INT, c INT)');

echo "Before insert\n";

$stmt1 = $db->query('SELECT * FROM test_column_count');
$stmt2 = $db->prepare('SELECT * FROM test_column_count');
var_dump($stmt1->columnCount(), $stmt2->columnCount());

$stmt2->execute();
var_dump($stmt2->columnCount());

$db->exec("INSERT INTO test_column_count (a, b, c) VALUES (0, 0, 0)");

echo "\n";
echo "After insert\n";

$stmt1 = $db->query('SELECT * FROM test_column_count');
$stmt2 = $db->prepare('SELECT * FROM test_column_count');
var_dump($stmt1->columnCount(), $stmt2->columnCount());

$stmt2->execute();
var_dump($stmt2->columnCount());
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_column_count');
?>
--EXPECT--
Before insert
int(3)
int(0)
int(3)

After insert
int(3)
int(0)
int(3)

--TEST--
getColumnMeta
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_get_column_meta (a INT, b VARCHAR(10))');
$db->exec("INSERT INTO test_get_column_meta (a, b) VALUES (1, 'one')");

$stmt = $db->query('SELECT * FROM test_get_column_meta');

try {
    $stmt->getColumnMeta(0);
} catch (PDOException $e) {
    echo $e->getMessage();
}
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_get_column_meta');
?>
--EXPECT--
SQLSTATE[IM001]: Driver does not support this function: driver doesn't support meta data

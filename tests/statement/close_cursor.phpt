--TEST--
closeCursor
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_close_cursor (a INT)');

$stmt = $db->query('SELECT * FROM test_close_cursor');
var_dump($stmt->closeCursor());
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_close_cursor');
?>
--EXPECT--
bool(true)

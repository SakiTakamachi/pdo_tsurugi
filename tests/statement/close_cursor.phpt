--TEST--
closeCursor
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_close_cursor (a INT)');
$db->exec('INSERT INTO test_close_cursor (a) VALUES (10)');

$stmt = $db->query('SELECT * FROM test_close_cursor');
var_dump($stmt->closeCursor());
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_close_cursor');
?>
--EXPECT--
bool(true)
array(0) {
}

--TEST--
debugDumpParams
--FILE--
<?php

use Pdo\Tsurugi;
use Pdo\Tsurugi\NamedPlaceholders;

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_debug_dump_params (a INT, b INT, c INT)');

$stmt = $db->prepare('SELECT * FROM test_debug_dump_params WHERE a=:a');

$ret = $stmt->debugDumpParams();
echo $ret === null ? 'success.' : 'failed.';
echo "\n\n";

$stmt->bindValue(1, 5, PDO::PARAM_INT);
$ret = $stmt->debugDumpParams();
echo $ret === null ? 'success.' : 'failed.';
echo "\n\n";

$stmt = $db->prepare('SELECT * FROM test_debug_dump_params WHERE a=:a', [
    Tsurugi::PLACEHOLDERS => new NamedPlaceholders()
        ->addInteger(':a'),
]);

$stmt->bindValue(1, 20, PDO::PARAM_INT);
$ret = $stmt->debugDumpParams();
echo $ret === null ? 'success.' : 'failed.';
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_debug_dump_params');
?>
--EXPECT--

SQL: [47] SELECT * FROM test_debug_dump_params WHERE a=:a
Params:  0
success.

SQL: [47] SELECT * FROM test_debug_dump_params WHERE a=:a
Params:  1
Key: Position #0:
paramno=0
name=[0] ""
is_param=1
param_type=1
success.

SQL: [47] SELECT * FROM test_debug_dump_params WHERE a=:a
Params:  1
Key: Position #0:
paramno=0
name=[0] ""
is_param=1
param_type=1
success.

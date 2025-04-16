--TEST--
instant transaction
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

function inTransaction($db)
{
    echo 'in_transaction: ' . ($db->inTransaction() ? 'true' : 'false') . "\n";
}

$db->exec('CREATE TABLE test_instant_transaction (id INT PRIMARY KEY, name VARCHAR(16))');

$db->setAttribute(PDO::ATTR_AUTOCOMMIT, true);

echo "before select\n";
inTransaction($db);

echo "\n";
$stmt = $db->query('SELECT * FROM test_instant_transaction');
echo "after select\n";
inTransaction($db);

echo "\n";
$stmt->fetch(PDO::FETCH_ASSOC);
echo "after fetch 1\n";
inTransaction($db);

echo "\n";
echo "before select\n";
inTransaction($db);

echo "\n";
$stmt = $db->query('SELECT * FROM test_instant_transaction');
echo "after select\n";
inTransaction($db);

echo "\n";
$db->exec("INSERT INTO test_instant_transaction (id, name) VALUES (1, 'my name 1')");
echo "after insert\n";
inTransaction($db);

echo "\n";
echo "before select\n";
inTransaction($db);

echo "\n";
$stmt = $db->prepare('SELECT * FROM test_instant_transaction');
$stmt->execute();
echo "after select execute with prepare\n";
inTransaction($db);

echo "\n";
$stmt->execute();
echo "after another select execute\n";
inTransaction($db);

echo "\n";
$stmt->closeCursor();
echo "after closeCursor\n";
inTransaction($db);
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_instant_transaction');
?>
--EXPECT--
before select
in_transaction: false

after select
in_transaction: true

after fetch 1
in_transaction: false

before select
in_transaction: false

after select
in_transaction: true

after insert
in_transaction: false

before select
in_transaction: false

after select execute with prepare
in_transaction: true

after another select execute
in_transaction: true

after closeCursor
in_transaction: false

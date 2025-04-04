--TEST--
transaction
--FILE--
<?php

require __DIR__ . '/test.inc';
$db = getConnection();

function inTransaction($db)
{
    echo 'in_transaction: ' . ($db->inTransaction() ? 'true' : 'false') . "\n";
}

$db->exec('CREATE TABLE test_transaction (id INT PRIMARY KEY, name VARCHAR(16))');

$db->setAttribute(PDO::ATTR_AUTOCOMMIT, false);

inTransaction($db);
echo "begin\n";
$db->beginTransaction();
inTransaction($db);
echo "insert\n";
$db->exec("INSERT INTO test_transaction (id, name) VALUES (1, 'my name 1')");
$stmt = $db->query('SELECT * FROM test_transaction');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

inTransaction($db);
echo "rollback\n";
$db->rollback();
inTransaction($db);
echo "\n";

inTransaction($db);
echo "begin for select\n";
$db->beginTransaction();
inTransaction($db);
$stmt = $db->query('SELECT * FROM test_transaction');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
inTransaction($db);
echo "commit for select\n";
$db->commit();
inTransaction($db);
echo "\n";

inTransaction($db);
echo "begin\n";
$db->beginTransaction();
inTransaction($db);
echo "insert\n";
$db->exec("INSERT INTO test_transaction (id, name) VALUES (2, 'my name 2')");
$stmt = $db->query('SELECT * FROM test_transaction');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

inTransaction($db);
echo "commit\n";
$db->commit();
inTransaction($db);
echo "\n";

inTransaction($db);
echo "begin for select\n";
$db->beginTransaction();
inTransaction($db);
$stmt = $db->query('SELECT * FROM test_transaction');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
inTransaction($db);
echo "commit for select\n";
$db->commit();
inTransaction($db);
?>
--CLEAN--
<?php
require __DIR__ . '/test.inc';
dropTable('test_transaction');
?>
--EXPECT--
in_transaction: false
begin
in_transaction: true
insert
array(1) {
  [0]=>
  array(2) {
    ["id"]=>
    int(1)
    ["name"]=>
    string(9) "my name 1"
  }
}
in_transaction: true
rollback
in_transaction: false

in_transaction: false
begin for select
in_transaction: true
array(0) {
}
in_transaction: true
commit for select
in_transaction: false

in_transaction: false
begin
in_transaction: true
insert
array(1) {
  [0]=>
  array(2) {
    ["id"]=>
    int(2)
    ["name"]=>
    string(9) "my name 2"
  }
}
in_transaction: true
commit
in_transaction: false

in_transaction: false
begin for select
in_transaction: true
array(1) {
  [0]=>
  array(2) {
    ["id"]=>
    int(2)
    ["name"]=>
    string(9) "my name 2"
  }
}
in_transaction: true
commit for select
in_transaction: false

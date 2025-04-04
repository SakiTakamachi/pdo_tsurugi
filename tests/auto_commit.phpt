--TEST--
auto commit
--FILE--
<?php

require __DIR__ . '/test.inc';
$db = getConnection();

echo "auto commit true\n";
$db->setAttribute(PDO::ATTR_AUTOCOMMIT, true);

$db->exec('CREATE TABLE test_auto_commit (id INT PRIMARY KEY, name VARCHAR(16))');
$db->exec("INSERT INTO test_auto_commit (id, name) VALUES (1, 'my name')");

$stmt = $db->query('SELECT * FROM test_auto_commit');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

echo "\n";
echo "auto commit false\n";
$db->setAttribute(PDO::ATTR_AUTOCOMMIT, false);

try {
    $db->exec("INSERT INTO test_auto_commit (id, name) VALUES (2, 'my name 2')");
} catch (PDOException $e) {
    echo $e->getMessage();
}
?>
--CLEAN--
<?php
require __DIR__ . '/test.inc';
dropTable('test_auto_commit');
?>
--EXPECT--
auto commit true
array(1) {
  [0]=>
  array(2) {
    ["id"]=>
    int(1)
    ["name"]=>
    string(7) "my name"
  }
}

auto commit false
SQLSTATE[HY000]: General error: TSURUGI_CORE_CLIENT_ERROR transaction already closed

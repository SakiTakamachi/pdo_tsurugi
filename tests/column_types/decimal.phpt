--TEST--
column type DECIMAL
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_column_types_decimal (a DECIMAL(5, 0), b DECIMAL(36, 0), c DECIMAL(30, 28))');

$db->exec("INSERT INTO test_column_types_decimal (a, b, c) VALUES (0, 0, 0)");
$db->exec("INSERT INTO test_column_types_decimal (a, b, c) VALUES (-0, -0, -0)");
$db->exec("INSERT INTO test_column_types_decimal (a, b, c) VALUES (NULL, NULL, NULL)");

$stmt = $db->prepare("INSERT INTO test_column_types_decimal (a, b, c) VALUES (:a, :b, :c)");
$stmt->bindValue(':a', 15, PDO::PARAM_STR);
$stmt->bindValue(':b', '123456789012345678901234567890123456', PDO::PARAM_STR);
$stmt->bindValue(':c', '1.234567890123456789012345', PDO::PARAM_STR);
$stmt->execute();

$stmt->bindValue(':a', '00', PDO::PARAM_STR);
$stmt->bindValue(':b', '00', PDO::PARAM_STR);
$stmt->bindValue(':c', '0.234567890123456789012345', PDO::PARAM_STR);
$stmt->execute();

$a = -15;
$b = '-123456789012345678901234567890123456';
$c = '-1.234567890123456789012345';

$stmt->bindParam(':a', $a, PDO::PARAM_STR);
$stmt->bindParam(':b', $b, PDO::PARAM_STR);
$stmt->bindParam(':c', $c, PDO::PARAM_STR);
$stmt->execute();

$stmt->bindValue(':a', null, PDO::PARAM_STR);
$stmt->bindValue(':b', null, PDO::PARAM_STR);
$stmt->bindValue(':c', null, PDO::PARAM_NULL);
$stmt->execute();

$a = null;
$b = null;
$c = null;

$stmt->bindParam(':a', $a, PDO::PARAM_NULL);
$stmt->bindParam(':b', $b, PDO::PARAM_STR);
$stmt->bindParam(':c', $c, PDO::PARAM_STR);
$stmt->execute();

$stmt = $db->query('SELECT * FROM test_column_types_decimal');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_column_types_decimal');
?>
--EXPECT--
array(8) {
  [0]=>
  array(3) {
    ["a"]=>
    string(1) "0"
    ["b"]=>
    string(1) "0"
    ["c"]=>
    string(30) "0.0000000000000000000000000000"
  }
  [1]=>
  array(3) {
    ["a"]=>
    string(1) "0"
    ["b"]=>
    string(1) "0"
    ["c"]=>
    string(30) "0.0000000000000000000000000000"
  }
  [2]=>
  array(3) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
    ["c"]=>
    NULL
  }
  [3]=>
  array(3) {
    ["a"]=>
    string(2) "15"
    ["b"]=>
    string(36) "123456789012345678901234567890123456"
    ["c"]=>
    string(30) "1.2345678901234567890123450000"
  }
  [4]=>
  array(3) {
    ["a"]=>
    string(1) "0"
    ["b"]=>
    string(1) "0"
    ["c"]=>
    string(30) "0.2345678901234567890123450000"
  }
  [5]=>
  array(3) {
    ["a"]=>
    string(3) "-15"
    ["b"]=>
    string(37) "-123456789012345678901234567890123456"
    ["c"]=>
    string(31) "-1.2345678901234567890123450000"
  }
  [6]=>
  array(3) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
    ["c"]=>
    NULL
  }
  [7]=>
  array(3) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
    ["c"]=>
    NULL
  }
}

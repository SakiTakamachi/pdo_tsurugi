--TEST--
column type REAL, FLOAT and DOUBLE
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_column_types_float (a REAL, b FLOAT, c DOUBLE)');

$db->exec("INSERT INTO test_column_types_float (a, b, c) VALUES (0, 0, 0)");
$db->exec("INSERT INTO test_column_types_float (a, b, c) VALUES (-0, -0, -0)");
$db->exec("INSERT INTO test_column_types_float (a, b, c) VALUES (null, null, null)");

$stmt = $db->prepare("INSERT INTO test_column_types_float (a, b, c) VALUES (:a, :b, :c)");
$stmt->bindValue(':a', 1.1, PDO::PARAM_STR);
$stmt->bindValue(':b', 2.1, PDO::PARAM_STR);
$stmt->bindValue(':c', 3.1, PDO::PARAM_STR);
$stmt->execute();

$a = -1.1;
$b = -2.1;
$c = -3.1;

$stmt->bindParam(':a', $a, PDO::PARAM_STR);
$stmt->bindParam(':b', $b, PDO::PARAM_STR);
$stmt->bindParam(':c', $c, PDO::PARAM_STR);
$stmt->execute();

$a = 1.23E-3;
$b = 1.23456E-9;
$c = 1.23456789123456789E-9;

$stmt->bindParam(':a', $a, PDO::PARAM_STR);
$stmt->bindParam(':b', $b, PDO::PARAM_STR);
$stmt->bindParam(':c', $c, PDO::PARAM_STR);
$stmt->execute();

$stmt->bindValue(':a', null, PDO::PARAM_STR);
$stmt->bindValue(':b', null, PDO::PARAM_NULL);
$stmt->bindValue(':c', null, PDO::PARAM_NULL);
$stmt->execute();

$a = null;
$b = null;
$c = null;

$stmt->bindParam(':a', $a, PDO::PARAM_STR);
$stmt->bindParam(':b', $b, PDO::PARAM_NULL);
$stmt->bindParam(':c', $c, PDO::PARAM_STR);
$stmt->execute();

$stmt = $db->query('SELECT * FROM test_column_types_float');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_column_types_float');
?>
--EXPECT--
array(8) {
  [0]=>
  array(3) {
    ["a"]=>
    float(0)
    ["b"]=>
    float(0)
    ["c"]=>
    float(0)
  }
  [1]=>
  array(3) {
    ["a"]=>
    float(0)
    ["b"]=>
    float(0)
    ["c"]=>
    float(0)
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
    float(1.100000023841858)
    ["b"]=>
    float(2.0999999046325684)
    ["c"]=>
    float(3.1)
  }
  [4]=>
  array(3) {
    ["a"]=>
    float(-1.100000023841858)
    ["b"]=>
    float(-2.0999999046325684)
    ["c"]=>
    float(-3.1)
  }
  [5]=>
  array(3) {
    ["a"]=>
    float(0.001230000052601099)
    ["b"]=>
    float(1.2345600097773968E-9)
    ["c"]=>
    float(1.2345678912346E-9)
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

--TEST--
column type CHAR, CHARACTER, VARCHAR, CHAR VARYING and CHARACTER VARYING
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_column_types_character (a CHAR(15), b CHARACTER(15), c VARCHAR(15), d CHAR VARYING(15), e CHARACTER VARYING(15))');

$db->exec("INSERT INTO test_column_types_character (a, b, c, d, e) VALUES ('a', 'b', 'c', 'd', 'e')");
$db->exec("INSERT INTO test_column_types_character (a, b, c, d, e) VALUES (NULL, NULL, NULL, NULL, NULL)");

$stmt = $db->prepare("INSERT INTO test_column_types_character (a, b, c, d, e) VALUES (:a, :b, :c, :d, :e)");
$stmt->bindValue(':a', 'aa', PDO::PARAM_STR);
$stmt->bindValue(':b', 'bb\'', PDO::PARAM_STR);
$stmt->bindValue(':c', 'c\' \'c', PDO::PARAM_STR);
$stmt->bindValue(':d', 'dD', PDO::PARAM_STR);
$stmt->bindValue(':e', 'Ee', PDO::PARAM_STR);
$stmt->execute();

$a = 'aa\'a';
$b = '\'bbb';
$c = 'ccc';
$d = 'ddd';
$e = <<<TEXT
e
e
e
TEXT;
$stmt->bindParam(':a', $a, PDO::PARAM_STR);
$stmt->bindParam(':b', $b, PDO::PARAM_STR);
$stmt->bindParam(':c', $c, PDO::PARAM_STR);
$stmt->bindParam(':d', $d, PDO::PARAM_STR);
$stmt->bindParam(':e', $e, PDO::PARAM_STR);
$stmt->execute();

$stmt = $db->prepare("INSERT INTO test_column_types_character (a, b, c, d, e) VALUES (:a, :b, :c, :d, :e)");
$stmt->bindValue(':a', null, PDO::PARAM_NULL);
$stmt->bindValue(':b', null, PDO::PARAM_STR);
$stmt->bindValue(':c', null, PDO::PARAM_NULL);
$stmt->bindValue(':d', null, PDO::PARAM_STR);
$stmt->bindValue(':e', null, PDO::PARAM_NULL);
$stmt->execute();

$a = null;
$b = null;
$c = null;
$d = null;
$e = null;
$stmt->bindParam(':a', $a, PDO::PARAM_STR);
$stmt->bindParam(':b', $b, PDO::PARAM_NULL);
$stmt->bindParam(':c', $c, PDO::PARAM_STR);
$stmt->bindParam(':d', $d, PDO::PARAM_NULL);
$stmt->bindParam(':d', $e, PDO::PARAM_STR);
$stmt->execute();

$stmt = $db->query('SELECT * FROM test_column_types_character');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_column_types_character');
?>
--EXPECT--
array(6) {
  [0]=>
  array(5) {
    ["a"]=>
    string(15) "a              "
    ["b"]=>
    string(15) "b              "
    ["c"]=>
    string(1) "c"
    ["d"]=>
    string(1) "d"
    ["e"]=>
    string(1) "e"
  }
  [1]=>
  array(5) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
    ["c"]=>
    NULL
    ["d"]=>
    NULL
    ["e"]=>
    NULL
  }
  [2]=>
  array(5) {
    ["a"]=>
    string(15) "aa             "
    ["b"]=>
    string(15) "bb'            "
    ["c"]=>
    string(5) "c' 'c"
    ["d"]=>
    string(2) "dD"
    ["e"]=>
    string(2) "Ee"
  }
  [3]=>
  array(5) {
    ["a"]=>
    string(15) "aa'a           "
    ["b"]=>
    string(15) "'bbb           "
    ["c"]=>
    string(3) "ccc"
    ["d"]=>
    string(3) "ddd"
    ["e"]=>
    string(5) "e
e
e"
  }
  [4]=>
  array(5) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
    ["c"]=>
    NULL
    ["d"]=>
    NULL
    ["e"]=>
    NULL
  }
  [5]=>
  array(5) {
    ["a"]=>
    NULL
    ["b"]=>
    NULL
    ["c"]=>
    NULL
    ["d"]=>
    NULL
    ["e"]=>
    NULL
  }
}

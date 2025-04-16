--TEST--
prepare statement CHAR, CHARACTER, VARCHAR, CHAR VARYING and CHARACTER VARYING
--FILE--
<?php

use Pdo\Tsurugi;
use Pdo\Tsurugi\NamedPlaceholders;

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_prepare_statement_character (a CHAR(15), b CHARACTER(15), c VARCHAR(15), d CHAR VARYING(15), e CHARACTER VARYING(15))');

$stmt = $db->prepare("INSERT INTO test_prepare_statement_character (a, b, c, d, e) VALUES (:a, :b, :c, :d, :e)", [
    Tsurugi::PLACEHOLDERS => new NamedPlaceholders()
        ->addString(':a')
        ->addString(':b')
        ->addString(':c')
        ->addString(':d')
        ->addString(':e'),
]);

$stmt->bindValue(':a', null, PDO::PARAM_STR);
$stmt->bindValue(':b', null, PDO::PARAM_STR);
$stmt->bindValue(':c', null, PDO::PARAM_STR);
$stmt->bindValue(':d', null, PDO::PARAM_STR);
$stmt->bindValue(':e', null, PDO::PARAM_STR);
$stmt->execute();

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

$stmt = $db->query('SELECT * FROM test_prepare_statement_character');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_prepare_statement_character');
?>
--EXPECT--
array(3) {
  [0]=>
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
  [1]=>
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
  [2]=>
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
}

--TEST--
quote
--FILE--
<?php

require __DIR__ . '/test.inc';
$db = getConnection();

var_dump(
    $db->quote("abc", PDO::PARAM_STR),
    $db->quote("abc'd", PDO::PARAM_STR),
    $db->quote("abcde'", PDO::PARAM_STR),
);
?>
--EXPECT--
string(5) "'abc'"
string(8) "'abc''d'"
string(9) "'abcde'''"

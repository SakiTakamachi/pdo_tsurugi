--TEST--
bindColumn
--FILE--
<?php

use Pdo\Tsurugi;
use Pdo\Tsurugi\NamedPlaceholders;

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec(<<<SQL
    CREATE TABLE IF NOT EXISTS test_get_bind_column (
        b BIGINT,
        c VARCHAR(10),
        d VARBINARY(8),
        e DOUBLE,
        f DECIMAL(5, 3),
        g TIME,
        h DATE,
        i TIMESTAMP
    )
SQL);

$stmt = $db->prepare(<<<SQL
    INSERT INTO test_get_bind_column (
        b, c, d, e, f, g, h, i
    ) VALUES (
        :b, :c, :d, :e, :f, :g, :h, :i
    )
SQL, [
    Tsurugi::PLACEHOLDERS => new NamedPlaceholders()
        ->addBigInteger(':b')
        ->addString(':c')
        ->addBinary(':d')
        ->addDouble(':e')
        ->addDecimal(':f')
        ->addTime(':g')
        ->addDate(':h')
        ->addTimestamp(':i')
]);

$stmt->bindValue(':b', 10, PDO::PARAM_INT);
$stmt->bindValue(':c', 'the string', PDO::PARAM_STR);
$stmt->bindValue(':d', '0xEE01FF56', PDO::PARAM_STR);
$stmt->bindValue(':e', 98.76543, PDO::PARAM_STR);
$stmt->bindValue(':f', '12.345', PDO::PARAM_STR);
$stmt->bindValue(':g', '13:23:34', PDO::PARAM_STR);
$stmt->bindValue(':h', '2025-04-15', PDO::PARAM_STR);
$stmt->bindValue(':i', '2025-02-16 11:22:33', PDO::PARAM_STR);
$stmt->execute();

$stmt->bindValue(':b', null);
$stmt->bindValue(':c', null);
$stmt->bindValue(':d', null);
$stmt->bindValue(':e', null);
$stmt->bindValue(':f', null);
$stmt->bindValue(':g', null);
$stmt->bindValue(':h', null);
$stmt->bindValue(':i', null);
$stmt->execute();

$stmt = $db->query('SELECT * FROM test_get_bind_column');

$stmt->bindColumn(1, $b, PDO::PARAM_INT);
$stmt->bindColumn(2, $c);
$stmt->bindColumn(3, $d);
$stmt->bindColumn(4, $e);
$stmt->bindColumn(5, $f);
$stmt->bindColumn(6, $g);
$stmt->bindColumn(7, $h);
$stmt->bindColumn(8, $i);

while ($stmt->fetch(PDO::FETCH_BOUND)) {
    var_dump($b, $c, $d, $e, $f, $g, $h, $i);
    echo "\n";
}
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_get_bind_column');
?>
--EXPECT--
int(10)
string(10) "the string"
string(8) "ee01ff56"
string(8) "98.76543"
string(6) "12.345"
string(8) "13:23:34"
string(10) "2025-04-15"
string(19) "2025-02-16 11:22:33"

NULL
NULL
NULL
NULL
NULL
NULL
NULL
NULL

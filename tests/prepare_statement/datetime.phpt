--TEST--
prepare statement DATE, TIME, TIME WITH TIME ZONE, TIMESTAMP and TIMESTAMP WITH TIME ZONE
--FILE--
<?php

use Pdo\Tsurugi;
use Pdo\Tsurugi\NamedPlaceholders;

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->exec('CREATE TABLE IF NOT EXISTS test_prepare_statement_datetime (a DATE, b TIME, c TIME WITH TIME ZONE, d TIMESTAMP, e TIMESTAMP WITH TIME ZONE)');

$stmt = $db->prepare('INSERT INTO test_prepare_statement_datetime (a, b, c, d, e) VALUES (:a, :b, :c, :d, :e)', [
    Tsurugi::PLACEHOLDERS => new NamedPlaceholders()
        ->addDate(':a')
        ->addTime(':b')
        ->addTimeWithTimeZone(':c')
        ->addTimestamp(':d')
        ->addTimestampWithTimeZone(':e'),
]);

$stmt->bindValue(':a', '2024-10-01', PDO::PARAM_STR);
$stmt->bindValue(':b', '16:54:03', PDO::PARAM_STR);
$stmt->bindValue(':c', '16:54:03+09:00', PDO::PARAM_STR);
$stmt->bindValue(':d', '2025-03-01 12:34:48', PDO::PARAM_STR);
$stmt->bindValue(':e', '2025-03-01 12:34:48+09:00', PDO::PARAM_STR);
$stmt->execute();

date_default_timezone_set('Asia/Tokyo');

$stmt->bindValue(':a', '2024-10-01', PDO::PARAM_STR);
$stmt->bindValue(':b', '16:54:03+09:00', PDO::PARAM_STR);
$stmt->bindValue(':c', '16:54:03+09:00', PDO::PARAM_STR);
$stmt->bindValue(':d', '2025-03-01 12:34:48+09:00', PDO::PARAM_STR);
$stmt->bindValue(':e', '2025-03-01 12:34:48+09:00', PDO::PARAM_STR);
$stmt->execute();

$stmt->bindValue(':a', null, PDO::PARAM_STR);
$stmt->bindValue(':b', null, PDO::PARAM_STR);
$stmt->bindValue(':c', null, PDO::PARAM_STR);
$stmt->bindValue(':d', null, PDO::PARAM_STR);
$stmt->bindValue(':e', null, PDO::PARAM_STR);
$stmt->execute();

$stmt->bindValue(':a', '1743952456', PDO::PARAM_STR);
$stmt->bindValue(':b', '65200', PDO::PARAM_STR);
$stmt->bindValue(':c', '65200', PDO::PARAM_STR);
$stmt->bindValue(':d', '1743952456', PDO::PARAM_STR);
$stmt->bindValue(':e', '1743952456', PDO::PARAM_STR);
$stmt->execute();

$a = 1;
$b = 1;
$c = 1;
$d = -1;
$e = -1;

$stmt->bindParam(':a', $a, PDO::PARAM_INT);
$stmt->bindParam(':b', $b, PDO::PARAM_STR);
$stmt->bindParam(':c', $c, PDO::PARAM_STR);
$stmt->bindParam(':d', $d, PDO::PARAM_INT);
$stmt->bindParam(':e', $e, PDO::PARAM_STR);
$stmt->execute();

$stmt = $db->query('SELECT * FROM test_prepare_statement_datetime');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
?>
--CLEAN--
<?php
require dirname(__DIR__, 1) . '/test.inc';
dropTable('test_prepare_statement_datetime');
?>
--EXPECT--
array(5) {
  [0]=>
  array(5) {
    ["a"]=>
    string(10) "2024-10-01"
    ["b"]=>
    string(8) "16:54:03"
    ["c"]=>
    string(14) "07:54:03+00:00"
    ["d"]=>
    string(19) "2025-03-01 12:34:48"
    ["e"]=>
    string(25) "2025-03-01 03:34:48+00:00"
  }
  [1]=>
  array(5) {
    ["a"]=>
    string(10) "2024-10-01"
    ["b"]=>
    string(8) "16:54:03"
    ["c"]=>
    string(14) "07:54:03+00:00"
    ["d"]=>
    string(19) "2025-03-01 12:34:48"
    ["e"]=>
    string(25) "2025-03-01 03:34:48+00:00"
  }
  [2]=>
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
  [3]=>
  array(5) {
    ["a"]=>
    string(10) "2025-04-06"
    ["b"]=>
    string(8) "18:06:40"
    ["c"]=>
    string(14) "18:06:40+00:00"
    ["d"]=>
    string(19) "2025-04-06 15:14:16"
    ["e"]=>
    string(25) "2025-04-06 15:14:16+00:00"
  }
  [4]=>
  array(5) {
    ["a"]=>
    string(10) "1970-01-01"
    ["b"]=>
    string(8) "00:00:01"
    ["c"]=>
    string(14) "00:00:01+00:00"
    ["d"]=>
    string(19) "1969-12-31 23:59:59"
    ["e"]=>
    string(25) "1969-12-31 23:59:59+00:00"
  }
}

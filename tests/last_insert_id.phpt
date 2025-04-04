--TEST--
last insert id
--FILE--
<?php

require __DIR__ . '/test.inc';
$db = getConnection();

try {
    $db->lastInsertId();
} catch (PDOException $e) {
    echo $e->getMessage();
}
?>
--EXPECT--
SQLSTATE[IM001]: Driver does not support this function: driver does not support lastInsertId()

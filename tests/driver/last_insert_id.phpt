--TEST--
last insert id
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

try {
    $db->lastInsertId();
} catch (PDOException $e) {
    echo $e->getMessage();
}
?>
--EXPECT--
SQLSTATE[IM001]: Driver does not support this function: driver does not support lastInsertId()

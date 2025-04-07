--TEST--
query error
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

try {
    $stmt = $db->query('SELECT * FROM test_query_error');
} catch  (PDOException $e) {
    echo $e->getMessage();
}
?>
--EXPECTF--
SQLSTATE[HY000]: General error: 3004 SYMBOL_ANALYZE_EXCEPTION[SQL-03004] SQL service error (SQL-03004 (SYMBOL_ANALYZE_EXCEPTION)) compile failed with error:table_not_found message:"table "test_query_error" is not found" %s

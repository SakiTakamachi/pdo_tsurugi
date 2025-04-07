--TEST--
error info
--FILE--
<?php

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

try {
    $stmt = $db->query('SELECT * FROM test_error_info');
} catch  (PDOException) {
    echo "error code: {$db->errorCode()}\n";
    echo "error info: \n";
    var_dump($db->errorInfo());
}
?>
--EXPECTF--
error code: HY000
error info: 
array(3) {
  [0]=>
  string(5) "HY000"
  [1]=>
  int(3004)
  [2]=>
  string(285) "SYMBOL_ANALYZE_EXCEPTION[SQL-03004] SQL service error (SQL-03004 (SYMBOL_ANALYZE_EXCEPTION)) compile failed with error:table_not_found message:"table "test_error_info" is not found" %s"
}

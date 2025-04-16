--TEST--
transaction type error
--FILE--
<?php
use Pdo\Tsurugi;
use Pdo\Tsurugi\TransactionType;

require dirname(__DIR__, 1) . '/test.inc';
$db = getConnection();

$db->beginTransaction();

try {
    $db->setAttribute(Tsurugi::TRANSACTION_TYPE, TransactionType::Long);
} catch (PDOException $e) {
    echo $e->getMessage() . "\n";
}
var_dump($db->getAttribute(Tsurugi::TRANSACTION_TYPE));
$db->commit();

echo "\n";

try {
    $db->setAttribute(Tsurugi::TRANSACTION_TYPE, 1);
} catch (ValueError $e) {
    echo $e->getMessage() . "\n";
}
var_dump($db->getAttribute(Tsurugi::TRANSACTION_TYPE));
?>
--EXPECT--
SQLSTATE[HY000]: General error: Cannot change the transaction type while a transaction is already open
enum(Pdo\Tsurugi\TransactionType::Short)

Pdo\Tsurugi::TRANSACTION_TYPE must be of type Pdo\Tsurugi\TransactionType
enum(Pdo\Tsurugi\TransactionType::Short)

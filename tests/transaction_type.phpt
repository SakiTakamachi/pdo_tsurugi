--TEST--
transaction type
--FILE--
<?php
use Pdo\Tsurugi;
use Pdo\Tsurugi\TransactionType;

require __DIR__ . '/test.inc';
$db = getConnection();

echo "default transaction type.\n";
var_dump($db->getAttribute(Tsurugi::TRANSACTION_TYPE));
echo "\n";

echo "transaction type to Long.\n";
var_dump(
    $db->setAttribute(Tsurugi::TRANSACTION_TYPE, TransactionType::Long),
    $db->getAttribute(Tsurugi::TRANSACTION_TYPE),
);
echo "\n";

echo "transaction type to ReadOnly.\n";
var_dump(
    $db->setAttribute(Tsurugi::TRANSACTION_TYPE, TransactionType::ReadOnly),
    $db->getAttribute(Tsurugi::TRANSACTION_TYPE),
);
echo "\n";

echo "transaction type to Short.\n";
var_dump(
    $db->setAttribute(Tsurugi::TRANSACTION_TYPE, TransactionType::Short),
    $db->getAttribute(Tsurugi::TRANSACTION_TYPE),
);
?>
--EXPECT--
default transaction type.
enum(Pdo\Tsurugi\TransactionType::Short)

transaction type to Long.
bool(true)
enum(Pdo\Tsurugi\TransactionType::Long)

transaction type to ReadOnly.
bool(true)
enum(Pdo\Tsurugi\TransactionType::ReadOnly)

transaction type to Short.
bool(true)
enum(Pdo\Tsurugi\TransactionType::Short)

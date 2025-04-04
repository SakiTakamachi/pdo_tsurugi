--TEST--
transaction error
--FILE--
<?php

require __DIR__ . '/test.inc';
$db = getConnection();

$db->setAttribute(PDO::ATTR_AUTOCOMMIT, false);

$premises = [
    function ($db) {
        echo "No transaction operation has been performed yet\n";
    },
    function ($db) {
        echo "\n";
        echo "commit once\n";
        $db->commit();
    },
    function ($db) {
        echo "\n";
        echo "rollback once\n";
        $db->rollback();
    },
];

foreach ($premises as $premise) {
    $premise($db);
    try {
        $db->commit();
    } catch (PDOException $e) {
        echo $e->getMessage() . "\n";
    }

    try {
        $db->rollback();
    } catch (PDOException $e) {
        echo $e->getMessage() . "\n";
    }

    $db->beginTransaction();
    try {
        $db->beginTransaction();
    } catch (PDOException $e) {
        echo $e->getMessage() . "\n";
    }
}
?>
--EXPECT--
No transaction operation has been performed yet
There is no active transaction
There is no active transaction
There is already an active transaction

commit once
There is no active transaction
There is no active transaction
There is already an active transaction

rollback once
There is no active transaction
There is no active transaction
There is already an active transaction

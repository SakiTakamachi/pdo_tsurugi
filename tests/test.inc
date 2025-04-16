<?php

define('PDO_TSURUGI_END_POINT', getenv('PDO_TSURUGI_TEST_ENDPOINT') ?: 'tcp://127.0.0.1:12345');

function getConnection()
{
    return new \Pdo\Tsurugi('tsurugi:endpoint=' . PDO_TSURUGI_END_POINT);
}

function dropTable(string $table)
{
    $db = new \Pdo\Tsurugi('tsurugi:endpoint=' . PDO_TSURUGI_END_POINT);
    $db->exec("DROP TABLE IF EXISTS {$table}");
}
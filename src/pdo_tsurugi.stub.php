<?php

/** @generate-class-entries */

namespace Pdo
{
    /**
     * @strict-properties
     * @not-serializable
     */
    class Tsurugi extends \PDO
    {
        /** @cvalue PDO_TSURUGI_TRANSACTION_TYPE */
        public const int TRANSACTION_TYPE = UNKNOWN;
    }
}

namespace Pdo\Tsurugi
{
    enum TransactionType {
        case Short;
        case Long;
        case ReadOnly;
    }
}

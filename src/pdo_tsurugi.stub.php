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
        /** @cvalue PDO_TSURUGI_PLACEHOLDERS */
        public const int PLACEHOLDERS = UNKNOWN;

        /** @cvalue PDO_TSURUGI_TRANSACTION_TYPE */
        public const int TRANSACTION_TYPE = UNKNOWN;
    }
}

namespace Pdo\Tsurugi
{
    enum TransactionType
    {
        case Short;
        case Long;
        case ReadOnly;
    }

    final class NamedPlaceholders
    {
        public function addBoolean(string $name): NamedPlaceholders {}
        public function addInteger(string $name): NamedPlaceholders {}
        public function addBigInteger(string $name): NamedPlaceholders {}
        public function addString(string $name): NamedPlaceholders {}
        public function addBinary(string $name): NamedPlaceholders {}
        public function addFloat(string $name): NamedPlaceholders {}
        public function addDouble(string $name): NamedPlaceholders {}
        public function addDecimal(string $name): NamedPlaceholders {}
        public function addTime(string $name): NamedPlaceholders {}
        public function addTimeWithTimeZone(string $name): NamedPlaceholders {}
        public function addDate(string $name): NamedPlaceholders {}
        public function addTimestamp(string $name): NamedPlaceholders {}
        public function addTimestampWithTimeZone(string $name): NamedPlaceholders {}
    }

    final class PositionalPlaceholders
    {
        public function addBoolean(int $position): PositionalPlaceholders {}
        public function addInteger(int $position): PositionalPlaceholders {}
        public function addBigInteger(int $position): PositionalPlaceholders {}
        public function addString(int $position): PositionalPlaceholders {}
        public function addBinary(int $position): PositionalPlaceholders {}
        public function addFloat(int $position): PositionalPlaceholders {}
        public function addDouble(int $position): PositionalPlaceholders {}
        public function addDecimal(int $position): PositionalPlaceholders {}
        public function addTime(int $position): PositionalPlaceholders {}
        public function addTimeWithTimeZone(int $position): PositionalPlaceholders {}
        public function addDate(int $position): PositionalPlaceholders {}
        public function addTimestamp(int $position): PositionalPlaceholders {}
        public function addTimestampWithTimeZone(int $position): PositionalPlaceholders {}
    }
}

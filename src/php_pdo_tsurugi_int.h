#include "php.h"
#include "ext/pdo/php_pdo_driver.h"
#include <tsubakuro-rust-ffi.h>

#ifndef PHP_PDO_TSURUGI_INT_H
#define PHP_PDO_TSURUGI_INT_H

extern const pdo_driver_t pdo_tsurugi_driver;
extern zend_class_entry *pdo_tsurugi_transaction_type_ce;

extern zend_class_entry *pdo_tsurugi_named_placeholders_ce;
extern zend_class_entry *pdo_tsurugi_positional_placeholders_ce;

extern const struct pdo_stmt_methods tsurugi_stmt_methods;

typedef struct {
	int code;
	char *msg;
	size_t length;
} pdo_tsurugi_error;

typedef struct {
	pdo_tsurugi_error err;
	TsurugiFfiContextHandle context;
	TsurugiFfiSessionHandle session;
	TsurugiFfiSqlClientHandle client;
	TsurugiFfiTransactionHandle transaction;
	TsurugiFfiTransactionType transaction_type;
} pdo_tsurugi_db_handle;

typedef enum {
	PDO_TSURUGI_PLACEHOLDER_TYPE_NULL = 0,
	PDO_TSURUGI_PLACEHOLDER_TYPE_BOOLEAN,
	PDO_TSURUGI_PLACEHOLDER_TYPE_INTEGER,
	PDO_TSURUGI_PLACEHOLDER_TYPE_BIG_INTEGER,
	PDO_TSURUGI_PLACEHOLDER_TYPE_STRING,
	PDO_TSURUGI_PLACEHOLDER_TYPE_BINARY,
	PDO_TSURUGI_PLACEHOLDER_TYPE_FLOAT,
	PDO_TSURUGI_PLACEHOLDER_TYPE_DOUBLE,
	PDO_TSURUGI_PLACEHOLDER_TYPE_DECIMAL,
	PDO_TSURUGI_PLACEHOLDER_TYPE_TIME,
	PDO_TSURUGI_PLACEHOLDER_TYPE_TIME_WITH_TIME_ZONE,
	PDO_TSURUGI_PLACEHOLDER_TYPE_DATE,
	PDO_TSURUGI_PLACEHOLDER_TYPE_TIMESTAMP,
	PDO_TSURUGI_PLACEHOLDER_TYPE_TIMESTAMP_WITH_TIME_ZONE
} pdo_tsurugi_data_type;

typedef struct {
	zend_string *name;
	pdo_tsurugi_data_type type;
	zval *value;
	bool is_null;
} pdo_tsurugi_parameter;

typedef struct {
	pdo_tsurugi_db_handle *H;
	TsurugiFfiSqlQueryResultHandle result;
	TsurugiFfiSqlColumnHandle *col_metadata;
	uint32_t col_count;
	TsurugiFfiSqlPreparedStatementHandle prepared_statement;
	HashTable *placeholders;
	pdo_tsurugi_parameter *parameters;
	size_t parameter_count;
} pdo_tsurugi_stmt;

typedef struct {
	HashTable *placeholders;
	zend_object std;
} pdo_tsurugi_placeholders;

void php_tsurugi_set_error(
	pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *state, const size_t state_len, const char *msg, const size_t msg_len);
#define php_tsurugi_error(dbh) php_tsurugi_set_error(dbh, NULL, NULL, 0, NULL, 0)
#define php_tsurugi_error_stmt(stmt) php_tsurugi_set_error(stmt->dbh, stmt, NULL, 0, NULL, 0)
#define php_tsurugi_error_with_info(dbh, state, msg) php_tsurugi_set_error(dbh, NULL, ZEND_STRL(state), ZEND_STRL(msg))
#define php_tsurugi_error_stmt_with_info(stmt, state, msg) php_tsurugi_set_error(stmt->dbh, stmt, ZEND_STRL(state), ZEND_STRL(msg))

void php_tsurugi_raise_impl_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, char *state);

bool php_tsurugi_begin_instant_txn(pdo_dbh_t *dbh, bool *instant_txn);
bool php_tsurugi_commit_instant_txn(pdo_dbh_t *dbh);

HashTable *pdo_tsurugi_get_placeholders_hash_table(zend_object *obj);
bool pdo_tsurugi_register_placeholders(
	pdo_dbh_t *dbh, zval *placeholder_name, TsurugiFfiSqlPlaceholderHandle *placeholder_handle, pdo_tsurugi_data_type type);
bool pdo_tsurugi_register_parameter(
	pdo_stmt_t *stmt, zend_string *parameter_name, TsurugiFfiSqlParameterHandle *parameter_handle, pdo_tsurugi_data_type type, zval *value);

static zend_always_inline bool php_tsurugi_has_error(pdo_dbh_t *dbh)
{
	TsurugiFfiRc rc;
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;
	tsurugi_ffi_context_get_return_code(H->context, &rc);
	return rc != 0;
}

enum {
	PDO_TSURUGI_PLACEHOLDERS = PDO_ATTR_DRIVER_SPECIFIC,
	PDO_TSURUGI_TRANSACTION_TYPE
};

#endif

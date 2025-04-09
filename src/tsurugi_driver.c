
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "zend_enum.h"
#include "zend_exceptions.h"
#include "ext/pdo/php_pdo.h"
#include "ext/pdo/php_pdo_driver.h"
#include "php_pdo_tsurugi.h"
#include "php_pdo_tsurugi_int.h"


void php_tsurugi_raise_impl_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, char *state)
{
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;
	if (php_tsurugi_has_error(dbh)) {
		TsurugiFfiStringHandle error_name;
		tsurugi_ffi_context_get_error_name(H->context, &error_name);

		TsurugiFfiStringHandle db_message;
		tsurugi_ffi_context_get_error_message(H->context, &db_message);

		char buf[512];
		snprintf(buf, sizeof(buf), "%s %s\n", error_name, db_message);

		pdo_raise_impl_error(dbh, stmt, state, db_message);
	}

	/* fallback */
	pdo_raise_impl_error(dbh, stmt, state, "PDO_TSURUGI: Unknown error");
}

void php_tsurugi_set_error(
	pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *state, size_t state_len, const char *msg, const size_t msg_len)
{
	pdo_error_type *error_code = stmt ? &stmt->error_code : &dbh->error_code;
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;
	pdo_tsurugi_error *err = &H->err;

	if (err->msg) {
		pefree(err->msg, dbh->is_persistent);
		err->msg = NULL;
	}

	if (php_tsurugi_has_error(dbh)) {
		TsurugiFfiStringHandle error_name;
		tsurugi_ffi_context_get_error_name(H->context, &error_name);

		TsurugiFfiStringHandle structured_code;
		tsurugi_ffi_context_get_server_error_structured_code(H->context, &structured_code);

		TsurugiFfiStringHandle db_message;
		tsurugi_ffi_context_get_error_message(H->context, &db_message);

		tsurugi_ffi_context_get_server_error_code_number(H->context, &err->code);

		char buf[512];
		if (structured_code) {
			snprintf(buf, sizeof(buf), "%s[%s] %s", error_name, structured_code, db_message);
		} else {
			snprintf(buf, sizeof(buf), "%s %s", error_name, db_message);
		}

		err->length = strlen(buf);
		err->msg = pestrndup(buf, err->length, dbh->is_persistent);
	} else if (msg && msg_len) {
		err->length = msg_len;
		err->msg = pestrndup(msg, err->length, dbh->is_persistent);
	}

	if (state && state_len && state_len < sizeof(pdo_error_type)) {
		memcpy(*error_code, state, state_len + 1);
	} else {
		memcpy(*error_code, "HY000", sizeof("HY000"));
	}

	if (!dbh->methods) {
		pdo_throw_exception(0, err->msg, error_code);
	}
}

static void tsurugi_fetch_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, zval *info)
{
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;
	if (H->err.code != IS_NULL) {
		add_next_index_long(info, H->err.code);
	}
	if (H->err.msg && H->err.length) {
		add_next_index_stringl(info, H->err.msg, H->err.length);
	}
}

static bool tsurugi_handle_begin(pdo_dbh_t *dbh);
static bool tsurugi_handle_commit(pdo_dbh_t *dbh);
static bool tsurugi_handle_rollback(pdo_dbh_t *dbh);

bool php_tsurugi_begin_instant_txn(pdo_dbh_t *dbh)
{
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;
	if (tsurugi_handle_begin(dbh)) {
		dbh->in_txn = 1;
		H->in_instant_txn = true;
		return true;
	}
	return false;
}

bool php_tsurugi_commit_instant_txn(pdo_dbh_t *dbh)
{
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;
	if (tsurugi_handle_commit(dbh)) {
		dbh->in_txn = 0;
		H->in_instant_txn = false;
		return true;
	}
	return false;
}

static void tsurugi_handle_closer(pdo_dbh_t *dbh)
{
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;

	if (!H) {
		return;
	}

	if (dbh->in_txn) {
		if (dbh->auto_commit) {
			tsurugi_handle_commit(dbh);
		} else {
			tsurugi_handle_rollback(dbh);
		}
		dbh->in_txn = false;
	}

	H->in_instant_txn = false;
	if (H->session) {
		tsurugi_ffi_session_dispose(H->session);
	}
	if (H->client) {
		tsurugi_ffi_sql_client_dispose(H->client);
	}
	if (H->context) {
		tsurugi_ffi_context_dispose(H->context);
	}

	if (H->err.msg) {
		pefree(H->err.msg, dbh->is_persistent);
		H->err.msg = NULL;
	}
	pefree(H, dbh->is_persistent);
	dbh->driver_data = NULL;
}

static bool tsurugi_handle_preparer(pdo_dbh_t *dbh, zend_string *sql, pdo_stmt_t *stmt, zval *driver_options)
{
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;
	pdo_tsurugi_stmt *S = ecalloc(1, sizeof(*S));

	S->H = H;
	S->result = NULL;
	S->col_count = 0;
	S->fetched_col_count = 0;
	S->col_metadata = NULL;
	S->prepared_statement = NULL;
	S->placeholders = NULL;
	S->parameters = NULL;

	stmt->driver_data = S;
	stmt->methods = &tsurugi_stmt_methods;

	zval *placeholders;
	if (driver_options && (placeholders = zend_hash_index_find(Z_ARRVAL_P(driver_options), PDO_TSURUGI_PLACEHOLDERS)) != NULL) {
		if (Z_TYPE_P(placeholders) != IS_OBJECT) {
			zend_value_error(
				"Pdo\\Tsurugi::PLACEHOLDERS must be of type %s or %s, %s given",
				ZSTR_VAL(pdo_tsurugi_named_placeholders_ce->name),
				ZSTR_VAL(pdo_tsurugi_positional_placeholders_ce->name),
				zend_zval_value_name(placeholders)
			);
			return false;
		}

		bool is_named_placeholders;
		if (instanceof_function(Z_OBJCE_P(placeholders), pdo_tsurugi_named_placeholders_ce)) {
			is_named_placeholders = true;
		} else if (instanceof_function(Z_OBJCE_P(placeholders), pdo_tsurugi_positional_placeholders_ce)) {
			is_named_placeholders = false;
		} else {
			zend_value_error(
				"Pdo\\Tsurugi::PLACEHOLDERS must be of type %s or %s, %s given",
				ZSTR_VAL(pdo_tsurugi_named_placeholders_ce->name),
				ZSTR_VAL(pdo_tsurugi_positional_placeholders_ce->name),
				zend_zval_value_name(placeholders)
			);
			return false;
		}

		stmt->supports_placeholders = PDO_PLACEHOLDER_NAMED;
		stmt->named_rewrite_template = ":p%d";

		zend_string *parsed_sql;
		int ret = pdo_parse_params(stmt, sql, &parsed_sql);

		if (ret == -1) {
			strcpy(dbh->error_code, stmt->error_code);
			return false;
		} else if (ret != 1) {
			parsed_sql = zend_string_copy(sql);
		}

		if (EXPECTED(stmt->bound_param_map)) {
			HashTable *placeholders_ht = pdo_tsurugi_get_placeholders_hash_table(Z_OBJ_P(placeholders));
			S->placeholders = zend_new_array(zend_hash_num_elements(placeholders_ht));
			TsurugiFfiSqlPlaceholderHandle *placeholder_handles = emalloc(zend_hash_num_elements(placeholders_ht) * sizeof(TsurugiFfiSqlPlaceholderHandle));

			zend_string *placeholder_key;
			zend_ulong placeholder_index;
			zval *placeholder_internal_key;
			TsurugiFfiRc rc;
			size_t count = 0;
			if (is_named_placeholders) {
				ZEND_HASH_FOREACH_STR_KEY_VAL(stmt->bound_param_map, placeholder_key, placeholder_internal_key) {
					if (placeholder_key != NULL) {
						zval *placeholder_type = zend_hash_find(placeholders_ht, placeholder_key);
						if (placeholder_type == NULL) {
							zend_value_error("Placeholder mismatch");
							zend_string_release(parsed_sql);
							efree(placeholder_handles);
							return false;
						}
						zend_hash_add(S->placeholders, Z_STR_P(placeholder_internal_key), placeholder_type);
						if (!pdo_tsurugi_register_placeholders(dbh, placeholder_internal_key, &placeholder_handles[count], Z_LVAL_P(placeholder_type))) {
							php_tsurugi_error(dbh);
							zend_string_release(parsed_sql);
							for (size_t i = 0; i < count; i++) {
								tsurugi_ffi_sql_placeholder_dispose(placeholder_handles[i]);
							}
							efree(placeholder_handles);
							return false;
						}
						count++;
					}
				} ZEND_HASH_FOREACH_END();
			} else {
				ZEND_HASH_FOREACH_KEY_VAL(stmt->bound_param_map, placeholder_index, placeholder_key, placeholder_internal_key) {
					if (placeholder_key == NULL) {
						zval *placeholder_type = zend_hash_index_find(placeholders_ht, placeholder_index + 1);
						if (placeholder_type == NULL) {
							zend_value_error("Placeholder mismatch");
							zend_string_release(parsed_sql);
							efree(placeholder_handles);
							return false;
						}
						zend_hash_add(S->placeholders, Z_STR_P(placeholder_internal_key), placeholder_type);
						if (!pdo_tsurugi_register_placeholders(dbh, placeholder_internal_key, &placeholder_handles[count], Z_LVAL_P(placeholder_type))) {
							php_tsurugi_error(dbh);
							zend_string_release(parsed_sql);
							for (size_t i = 0; i < count; i++) {
								tsurugi_ffi_sql_placeholder_dispose(placeholder_handles[i]);
							}
							efree(placeholder_handles);
							return false;
						}
						count++;
					}
				} ZEND_HASH_FOREACH_END();
			}

			rc = tsurugi_ffi_sql_client_prepare(H->context, H->client, ZSTR_VAL(parsed_sql), placeholder_handles, count, &S->prepared_statement);
			for (size_t i = 0; i < count; i++) {
				tsurugi_ffi_sql_placeholder_dispose(placeholder_handles[i]);
			}
			efree(placeholder_handles);
			if (rc != 0) {
				zend_string_release(parsed_sql);
				php_tsurugi_error(dbh);
				return false;
			}
		} else if (zend_hash_num_elements(Z_ARRVAL_P(placeholders)) > 0) {
			zend_value_error("Placeholder mismatch");
			zend_string_release(parsed_sql);
			return false;
		} else {
			/* No placeholders */
			stmt->supports_placeholders = PDO_PLACEHOLDER_NONE;
			stmt->named_rewrite_template = NULL;
			return true;
		}
		zend_string_release(parsed_sql);
	} else {
		stmt->supports_placeholders = PDO_PLACEHOLDER_NONE;
	}
	return true;
}

static zend_long tsurugi_handle_doer(pdo_dbh_t *dbh, const zend_string *sql)
{
	TsurugiFfiRc rc;
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;

	/* If the next query is executed before all the results of the previous query have been fetched */
	if (H->in_instant_txn && !php_tsurugi_commit_instant_txn(dbh)) {
		return -1;
	}

	if (dbh->auto_commit && !dbh->in_txn && !php_tsurugi_begin_instant_txn(dbh)) {
		return -1;
	}

	if (!H->transaction) {
		zend_throw_exception_ex(php_pdo_get_exception(), 0, "There is no active transaction");
	}

	TsurugiFfiSqlExecuteResultHandle result;
	rc = tsurugi_ffi_sql_client_execute(H->context, H->client, H->transaction, ZSTR_VAL(sql), &result);
	if (rc != 0) {
		tsurugi_ffi_sql_execute_result_dispose(result);
		php_tsurugi_error(dbh);
		return -1;
	}

	int64_t affected_rows;
	rc = tsurugi_ffi_sql_execute_result_get_rows(H->context, result, &affected_rows);
	tsurugi_ffi_sql_execute_result_dispose(result);

	if (rc != 0) {
		php_tsurugi_error(dbh);
		return -1;
	}

	if (H->in_instant_txn && !php_tsurugi_commit_instant_txn(dbh)) {
		return -1;
	}

	return affected_rows;
}

static zend_string* tsurugi_handle_quoter(pdo_dbh_t *dbh, const zend_string *unquoted, enum pdo_param_type paramtype)
{
	if (ZSTR_LEN(unquoted) == 0) {
		return ZSTR_INIT_LITERAL("''", 0);
	}

	const char *search = ZSTR_VAL(unquoted);
	size_t count = 0;
	while ((search = strchr(search,'\''))) {
		count++;
		search++;
	}

	if (UNEXPECTED(ZSTR_LEN(unquoted) + 2 > ZSTR_MAX_LEN - count)) {
		pdo_raise_impl_error(dbh, NULL, "HY000", "String too long");
		return NULL;
	}

	size_t quoted_len = ZSTR_LEN(unquoted) + count + 2;
	zend_string *quoted_str = zend_string_alloc(quoted_len, 0);
	char *ptr = ZSTR_VAL(quoted_str);
	*ptr++ = '\'';

	const char *copy_start = ZSTR_VAL(unquoted);
	for (size_t i = 0; i < count; i++) {
		const char *copy_end = strchr(copy_start,'\'');
		size_t copy_size = copy_end - copy_start + 1;
		strncpy(ptr, copy_start, copy_size);
		ptr += copy_size;
		*ptr++ = '\'';
		copy_start = copy_end + 1;
	}

	size_t copy_size = quoted_len - (ptr - ZSTR_VAL(quoted_str)) - 1;
	strncpy(ptr, copy_start, copy_size);
	ptr += copy_size;
	*ptr++ = '\'';
	*ptr = '\0';

	return quoted_str;
}

static bool tsurugi_handle_begin(pdo_dbh_t *dbh)
{
	TsurugiFfiRc rc;
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;

	TsurugiFfiTransactionOptionHandle transaction_option;
	rc = tsurugi_ffi_transaction_option_create(H->context, &transaction_option);
	if (rc != 0) {
		goto fail;
	}

	rc = tsurugi_ffi_transaction_option_set_transaction_type(H->context, transaction_option, H->transaction_type);
	if (rc != 0) {
		tsurugi_ffi_transaction_option_dispose(transaction_option);
		goto fail;
	}

	rc = tsurugi_ffi_sql_client_start_transaction(H->context, H->client, transaction_option, &H->transaction);
	tsurugi_ffi_transaction_option_dispose(transaction_option);
	if (rc != 0) {
		goto fail;
	}

	return true;

fail:
	php_tsurugi_error(dbh);
	return false;
}

static bool tsurugi_handle_commit(pdo_dbh_t *dbh)
{
	TsurugiFfiRc rc;
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;

	TsurugiFfiCommitOptionHandle commit_option;
	rc = tsurugi_ffi_commit_option_create(H->context, &commit_option);
	if (rc != 0) {
		goto fail;
	}

	rc = tsurugi_ffi_sql_client_commit(H->context, H->client, H->transaction, commit_option);
	tsurugi_ffi_commit_option_dispose(commit_option);
	tsurugi_ffi_transaction_dispose(H->transaction);
	if (rc != 0) {
		goto fail;
	}
	return true;

fail:
	php_tsurugi_error(dbh);
	return false;
}

static bool tsurugi_handle_rollback(pdo_dbh_t *dbh)
{
	TsurugiFfiRc rc;
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;

	rc = tsurugi_ffi_sql_client_rollback(H->context, H->client, H->transaction);
	tsurugi_ffi_transaction_dispose(H->transaction);
	if (rc != 0) {
		goto fail;
	}
	return true;

fail:
	php_tsurugi_error(dbh);
	return false;
}

static zend_always_inline bool php_tsurugi_get_transaction_type(const zval *val, TsurugiFfiTransactionType *txn_type)
{
	if (Z_TYPE_P(val) == IS_OBJECT && instanceof_function(Z_OBJCE_P(val), pdo_tsurugi_transaction_type_ce)) {
		const zval *case_name = zend_enum_fetch_case_name(Z_OBJ_P(val));
		const zend_string *type_name = Z_STR_P(case_name);
		switch (ZSTR_VAL(type_name)[0]) {
			case 'S':
				*txn_type = TSURUGI_FFI_TRANSACTION_TYPE_SHORT;
				break;
			case 'L':
				*txn_type = TSURUGI_FFI_TRANSACTION_TYPE_LONG;
				break;
			case 'R':
				*txn_type = TSURUGI_FFI_TRANSACTION_TYPE_READ_ONLY;
				break;
			EMPTY_SWITCH_DEFAULT_CASE();
		}
	} else {
		zend_value_error("Pdo\\Tsurugi::TRANSACTION_TYPE must be of type Pdo\\Tsurugi\\TransactionType");
		return false;
	}
	return true;
}

static bool tsurugi_set_attr(pdo_dbh_t *dbh, zend_long attr, zval *val)
{
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *)dbh->driver_data;
	bool bval;

	switch(attr) {
		case PDO_ATTR_AUTOCOMMIT:
			if (!pdo_get_bool_param(&bval, val)) {
				return false;
			}
			if (dbh->in_txn) {
				pdo_raise_impl_error(dbh, NULL, "HY000", "Cannot change autocommit mode while a transaction is already open");
				return false;
			}
			dbh->auto_commit = bval;
			return true;

		case PDO_TSURUGI_TRANSACTION_TYPE:
			if (dbh->in_txn) {
				pdo_raise_impl_error(dbh, NULL, "HY000", "Cannot change the transaction type while a transaction is already open");
				return false;
			}
			if (!php_tsurugi_get_transaction_type(val, &H->transaction_type)) {
				return false;
			}
			return true;

		default:
			return false;
	}
}

static int tsurugi_get_attribute(pdo_dbh_t *dbh, zend_long attr, zval *return_value)
{
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *)dbh->driver_data;

	switch (attr) {
		case PDO_TSURUGI_TRANSACTION_TYPE:
			zend_object *zobj = zend_objects_new(pdo_tsurugi_transaction_type_ce);
			ZVAL_OBJ(return_value, zobj);

			zend_string *case_name;
			switch (H->transaction_type) {
				case TSURUGI_FFI_TRANSACTION_TYPE_SHORT:
					case_name = zend_string_init(ZEND_STRL("Short"), 0);
					break;
				case TSURUGI_FFI_TRANSACTION_TYPE_LONG:
					case_name = zend_string_init(ZEND_STRL("Long"), 0);
					break;
				case TSURUGI_FFI_TRANSACTION_TYPE_READ_ONLY:
					case_name = zend_string_init(ZEND_STRL("ReadOnly"), 0);
					break;
				EMPTY_SWITCH_DEFAULT_CASE();
			}

			zval *zname = OBJ_PROP_NUM(zobj, 0);
			ZVAL_STR(zname, case_name);
			Z_PROP_FLAG_P(zname) = 0;
			return 1;

		default:
			return 0;
	}

	return 1;
}

static const struct pdo_dbh_methods tsurugi_methods = {
	tsurugi_handle_closer,
	tsurugi_handle_preparer,
	tsurugi_handle_doer,
	tsurugi_handle_quoter,
	tsurugi_handle_begin, /* begin */
	tsurugi_handle_commit, /* commit */
	tsurugi_handle_rollback, /* rollback */
	tsurugi_set_attr, /*set attr */
	NULL, /* last insert id */
	tsurugi_fetch_error, /* fetch error */
	tsurugi_get_attribute, /* get attr */
	NULL, /* check liveness */
	NULL, /* get driver methods */
	NULL, /* request shutdown */
	NULL, /* in transaction, use PDO's internal tracking mechanism */
	NULL, /* get gc */
	NULL /* scanner */
};

static int pdo_tsurugi_handle_factory(pdo_dbh_t *dbh, zval *driver_options)
{
	struct pdo_data_src_parser vars[] = {
		{ "endpoint", NULL, 0 },
	};

	pdo_tsurugi_db_handle *H = pecalloc(1, sizeof(pdo_tsurugi_db_handle), dbh->is_persistent);
	dbh->driver_data = H;

	php_pdo_parse_data_source(dbh->data_source, dbh->data_source_len, vars, 1);

	H->err.code = 0;
	H->err.msg = NULL;

	H->context = NULL;
	H->session = NULL;
	H->client = NULL;
	H->transaction = NULL;

	zval *option_val;
	if (driver_options && (option_val = zend_hash_index_find(Z_ARRVAL_P(driver_options), PDO_TSURUGI_TRANSACTION_TYPE))) {
		if (!php_tsurugi_get_transaction_type(option_val, &H->transaction_type)) {
			goto fail;
		}
	} else {
		H->transaction_type = TSURUGI_FFI_TRANSACTION_TYPE_SHORT;
	}

	TsurugiFfiRc rc;
	rc = tsurugi_ffi_context_create(&H->context);
	if (rc != 0) {
		goto fail;
	}

	TsurugiFfiConnectionOptionHandle connection_option;
	rc = tsurugi_ffi_connection_option_create(H->context, &connection_option);
	if (rc != 0) {
		goto fail;
	}

	rc = tsurugi_ffi_connection_option_set_endpoint_url(H->context, connection_option, vars[0].optval);
	if (rc != 0) {
		tsurugi_ffi_connection_option_dispose(connection_option);
		goto fail;
	}

	TsurugiFfiDuration timeout = 10ull * 1000 * 1000 * 1000; /* 10 sec */
	rc = tsurugi_ffi_connection_option_set_default_timeout(H->context, connection_option, timeout);
	if (rc != 0) {
		tsurugi_ffi_connection_option_dispose(connection_option);
		goto fail;
	}

	rc = tsurugi_ffi_session_connect(H->context, connection_option, &H->session);
	tsurugi_ffi_connection_option_dispose(connection_option);
	if (rc != 0) {
		goto fail;
	}

	rc = tsurugi_ffi_session_make_sql_client(H->context, H->session, &H->client);
	if (rc != 0) {
		goto fail;
	}

	for (size_t i = 0; i < sizeof(vars)/sizeof(vars[0]); ++i) {
		if (vars[i].freeme) {
			efree(vars[i].optval);
		}
	}

	dbh->methods = &tsurugi_methods;
	dbh->alloc_own_columns = 1;
	return 1;

fail:
	for (size_t i = 0; i < sizeof(vars)/sizeof(vars[0]); ++i) {
		if (vars[i].freeme) {
			efree(vars[i].optval);
		}
	}
	dbh->methods = &tsurugi_methods;
	php_tsurugi_error(dbh);
	if (H->err.msg) {
		pdo_throw_exception(H->err.code, H->err.msg, &dbh->error_code);
	}
	tsurugi_handle_closer(dbh);
	return 0;
}

const pdo_driver_t pdo_tsurugi_driver = {
	PDO_DRIVER_HEADER(tsurugi),
	pdo_tsurugi_handle_factory
};

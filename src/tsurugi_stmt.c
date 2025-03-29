
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "php.h"
#include "php_ini.h"
#include "zend_exceptions.h"
#include "ext/pdo/php_pdo.h"
#include "ext/pdo/php_pdo_driver.h"
#include "php_pdo_tsurugi.h"
#include "php_pdo_tsurugi_int.h"

static void php_tsurugi_free_col_metadata(pdo_stmt_t *stmt)
{
	pdo_tsurugi_stmt *S = (pdo_tsurugi_stmt*) stmt->driver_data;

	for (size_t i = 0; i < stmt->column_count; i++) {
		if (S->col_metadata[i]) {
			tsurugi_ffi_sql_column_dispose(S->col_metadata[i]);
		}
	}
	efree(S->col_metadata);
}

static int pdo_tsurugi_stmt_dtor(pdo_stmt_t *stmt)
{
	pdo_tsurugi_stmt *S = (pdo_tsurugi_stmt*) stmt->driver_data;

	if (S->result) {
		tsurugi_ffi_sql_query_result_dispose(S->result);
		S->result = NULL;
	}
	if (S->col_metadata) {
		php_tsurugi_free_col_metadata(stmt);
		S->col_metadata = NULL;
	}
	S->affected_rows = 0;
	efree(S);

	return 1;
}

static int pdo_tsurugi_stmt_execute(pdo_stmt_t *stmt)
{
	TsurugiFfiRc rc;
	pdo_tsurugi_stmt *S = (pdo_tsurugi_stmt*) stmt->driver_data;
	pdo_tsurugi_db_handle *H = S->H;
	bool instant_txn = false;

	S->affected_rows = 0;
	if(S->result) {
		tsurugi_ffi_sql_query_result_dispose(S->result);
		S->result = NULL;
	}

	if (stmt->dbh->auto_commit && !stmt->dbh->in_txn && !php_tsurugi_begin_instant_txn(stmt->dbh, &instant_txn)) {
		return 0;
	}
	if (!H->transaction) {
		zend_throw_exception_ex(php_pdo_get_exception(), 0, "There is no active transaction");
	}

	const char *sql = ZSTR_VAL(stmt->active_query_string);
	size_t sql_len = ZSTR_LEN(stmt->active_query_string);
	if (sql ==  php_memnistr(sql, ZEND_STRL("select"), sql + sql_len)) {
		/* select query */
		rc = tsurugi_ffi_sql_client_query(H->context, H->client, H->transaction, sql, &S->result);
		if (rc != 0) {
			if (S->result) {
				tsurugi_ffi_sql_query_result_dispose(S->result);
				S->result = NULL;
			}
			goto fail;
		}

		TsurugiFfiSqlQueryResultMetadataHandle query_result_metadata;
		rc = tsurugi_ffi_sql_query_result_get_metadata(H->context, S->result, &query_result_metadata);
		if (rc != 0) {
			tsurugi_ffi_sql_query_result_dispose(S->result);
			S->result = NULL;
			goto fail;
		}

		uint32_t col_count;
		rc = tsurugi_ffi_sql_query_result_metadata_get_columns_size(H->context, query_result_metadata, &col_count);
		if (rc != 0) {
			tsurugi_ffi_sql_query_result_dispose(S->result);
			S->result = NULL;
			tsurugi_ffi_sql_query_result_metadata_dispose(query_result_metadata);
			goto fail;
		}
		php_pdo_stmt_set_column_count(stmt, col_count);

		S->col_metadata = ecalloc(stmt->column_count, sizeof(TsurugiFfiSqlColumnHandle));
		for (size_t i = 0; i < stmt->column_count; i++) {
			rc = tsurugi_ffi_sql_query_result_metadata_get_columns_value(H->context, query_result_metadata, i, &S->col_metadata[i]);
			if (rc != 0) {
				tsurugi_ffi_sql_query_result_dispose(S->result);
				S->result = NULL;
				php_tsurugi_free_col_metadata(stmt);
				tsurugi_ffi_sql_query_result_metadata_dispose(query_result_metadata);
				goto fail;
			}
		}
		tsurugi_ffi_sql_query_result_metadata_dispose(query_result_metadata);
	} else {
		/* others */
		TsurugiFfiSqlExecuteResultHandle execute_result;
		rc = tsurugi_ffi_sql_client_execute(H->context, H->client, H->transaction, sql, &execute_result);
		if (rc != 0) {
			goto fail;
		}

		int64_t affected_rows;
		rc = tsurugi_ffi_sql_execute_result_get_rows(H->context, execute_result, &affected_rows);
		tsurugi_ffi_sql_execute_result_dispose(execute_result);
		S->affected_rows = affected_rows;
	}

	if (instant_txn && !php_tsurugi_commit_instant_txn(stmt->dbh)) {
		return 0;
	}

	return 1;

fail:
	php_tsurugi_error_stmt(stmt);
	return 0;
}

static int pdo_tsurugi_stmt_fetch(pdo_stmt_t *stmt, enum pdo_fetch_orientation ori, zend_long offset)
{
	pdo_tsurugi_stmt *S = (pdo_tsurugi_stmt*) stmt->driver_data;

	if (!stmt->executed) {
		return 0;
	}

	bool next_row;
	TsurugiFfiRc rc = tsurugi_ffi_sql_query_result_next_row(S->H->context, S->result, &next_row);
	if (rc != 0) {
		php_tsurugi_raise_impl_error(stmt->dbh, stmt, ZEND_STRL("HY000"));
		return 0;
	}
	return next_row;
}

static int pdo_tsurugi_stmt_describe(pdo_stmt_t *stmt, int colno)
{
	pdo_tsurugi_stmt *S = (pdo_tsurugi_stmt*) stmt->driver_data;
	struct pdo_column_data *col = &stmt->columns[colno];

	const char *col_name;
	TsurugiFfiRc rc = tsurugi_ffi_sql_column_get_name(S->H->context, S->col_metadata[colno], &col_name);
	if (rc != 0) {
		php_tsurugi_error_stmt(stmt);
		return 0;
	}
	col->name = zend_string_init(col_name, strlen(col_name), 0);
	return 1;
}

static int pdo_tsurugi_stmt_get_col(pdo_stmt_t *stmt, int colno, zval *result, enum pdo_param_type *type)
{
	TsurugiFfiRc rc;
	pdo_tsurugi_stmt *S = (pdo_tsurugi_stmt*) stmt->driver_data;
	pdo_tsurugi_db_handle *H = S->H;

	TsurugiFfiAtomType atom_type;
	rc = tsurugi_ffi_sql_column_get_atom_type(S->H->context, S->col_metadata[colno], &atom_type);
	if (rc != 0) {
		goto fail;
	}

	bool next_col;
	rc = tsurugi_ffi_sql_query_result_next_column(H->context, S->result, &next_col);
	if (rc != 0) {
		goto fail;
	}

	if (!next_col) {
		return 0;
	}

	bool is_null;
	rc = tsurugi_ffi_sql_query_result_is_null(H->context, S->result, &is_null);
	if (rc != 0) {
		goto fail;
	}

	if (is_null) {
		ZVAL_NULL(result);
		return 1;
	}

	switch (atom_type) {
		case TSURUGI_FFI_ATOM_TYPE_BOOLEAN:
			/* Not implemented */
			break;

		case TSURUGI_FFI_ATOM_TYPE_INT4:
			int32_t ival;
			rc = tsurugi_ffi_sql_query_result_fetch_int4(H->context, S->result, &ival);
			ZVAL_LONG(result, ival);
			break;

		case TSURUGI_FFI_ATOM_TYPE_INT8:
			int64_t lval;
			rc = tsurugi_ffi_sql_query_result_fetch_int8(H->context, S->result, &lval);
			ZVAL_LONG(result, lval);
			break;

		case TSURUGI_FFI_ATOM_TYPE_FLOAT4:
			float fval;
			tsurugi_ffi_sql_query_result_fetch_float4(H->context, S->result, &fval);
			ZVAL_DOUBLE(result, fval);
			break;

		case TSURUGI_FFI_ATOM_TYPE_FLOAT8:
			double dval;
			tsurugi_ffi_sql_query_result_fetch_float8(H->context, S->result, &dval);
			ZVAL_DOUBLE(result, dval);
			break;

		case TSURUGI_FFI_ATOM_TYPE_CHARACTER:
			const char *cval;
			tsurugi_ffi_sql_query_result_fetch_character(H->context, S->result, &cval);
			ZVAL_STRING(result, cval);
			break;
	}

	return 1;

fail:
	php_tsurugi_error_stmt(stmt);
	return 0;
}

static int pdo_tsurugi_stmt_cursor_closer(pdo_stmt_t *stmt)
{
	/* Nothing todo */
	return 1;
}


const struct pdo_stmt_methods tsurugi_stmt_methods = { /* {{{ */
	pdo_tsurugi_stmt_dtor,
	pdo_tsurugi_stmt_execute,
	pdo_tsurugi_stmt_fetch,
	pdo_tsurugi_stmt_describe,
	pdo_tsurugi_stmt_get_col,
	NULL, /* param hook */
	NULL, /* set_attribute */
	NULL, /* get_attribute */
	NULL, /* get_column_meta */
	NULL, /* next_rowset_func */
	pdo_tsurugi_stmt_cursor_closer
};
/* }}} */

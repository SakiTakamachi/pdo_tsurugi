
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "php.h"
#include "php_ini.h"
#include "zend_exceptions.h"
#include "zend_portability.h"
#include "ext/pdo/php_pdo.h"
#include "ext/pdo/php_pdo_driver.h"
#include "php_pdo_tsurugi.h"
#include "php_pdo_tsurugi_int.h"
#include <time.h>

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

	int32_t ival;
	int64_t lval;
	float fval;
	double dval;
	const char *cval;
	struct tm *t;
	uint32_t nano_sec;
	int32_t tz_offset;

	switch (atom_type) {
		case TSURUGI_FFI_ATOM_TYPE_BOOLEAN:
			/* Not implemented */
			break;

		case TSURUGI_FFI_ATOM_TYPE_INT4:
			rc = tsurugi_ffi_sql_query_result_fetch_int4(H->context, S->result, &ival);
			if (rc != 0) {
				goto fail;
			}
			ZVAL_LONG(result, ival);
			break;

		case TSURUGI_FFI_ATOM_TYPE_INT8:
			rc = tsurugi_ffi_sql_query_result_fetch_int8(H->context, S->result, &lval);
			if (rc != 0) {
				goto fail;
			}
			ZVAL_LONG(result, lval);
			break;

		case TSURUGI_FFI_ATOM_TYPE_FLOAT4:
			rc = tsurugi_ffi_sql_query_result_fetch_float4(H->context, S->result, &fval);
			if (rc != 0) {
				goto fail;
			}
			ZVAL_DOUBLE(result, fval);
			break;

		case TSURUGI_FFI_ATOM_TYPE_FLOAT8:
			rc = tsurugi_ffi_sql_query_result_fetch_float8(H->context, S->result, &dval);
			if (rc != 0) {
				goto fail;
			}
			ZVAL_DOUBLE(result, dval);
			break;

		case TSURUGI_FFI_ATOM_TYPE_DECIMAL:
			TsurugiFfiByteArrayHandle bytes;
			uint32_t bytes_size;
			int32_t exponent;
			rc = tsurugi_ffi_sql_query_result_fetch_decimal(H->context, S->result, &bytes, &bytes_size, &lval, &exponent);
			if (rc != 0) {
				goto fail;
			}

			char *buf = NULL;
			uint64_t *v = NULL;
			char *tmp_str;
			size_t len = 0;
			bool is_negative = false;
			if (bytes_size > 0) {
				is_negative = (int8_t) bytes[0] < 0;

				TsurugiFfiByteArrayHandle b_ptr = bytes;
				TsurugiFfiByteArrayHandle b_end = bytes + bytes_size;
				size_t vsize = ((bytes_size + sizeof(uint32_t) - 1) / sizeof(uint32_t));
				v = emalloc(vsize * sizeof(uint64_t));
				bool has_leftover = bytes_size % sizeof(uint32_t);
				size_t bulk_loop = has_leftover ? vsize - 1 : vsize;
				for (size_t i = 0; i < bulk_loop; i++) {
					b_end -= sizeof(uint32_t);
					memcpy(&v[i], b_end, sizeof(uint32_t));
#ifndef WORDS_BIGENDIAN
					v[i] = ZEND_BYTES_SWAP64(v[i]);
#endif
					if (is_negative) {
						v[i] = ~v[i];
					}
					v[i] >>= 32;
				}

				size_t max_vindex = vsize - 1;
				if (has_leftover) {
					v[max_vindex] = 0;
#ifdef WORDS_BIGENDIAN
					while (b_end > bytes) {
						b_end--;
						v[max_vindex] <<= 8;
						if (is_negative) {
							v[max_vindex] |= (~*b_end & 0xFF);
						} else {
							v[max_vindex] |= *b_end;
						}
					}
#else
					while (b_end > bytes) {
						v[max_vindex] <<= 8;
						if (is_negative) {
							v[max_vindex] |= (~*bytes & 0xFF);
						} else {
							v[max_vindex] |= *bytes;
						}
						bytes++;
					}
#endif
				}

				if (is_negative) {
					v[0] += 1;
				}

				while (v[max_vindex] == 0 && max_vindex > 0) {
					max_vindex--;
				}

				buf = emalloc(vsize * 10);
				char *tmp_ptr = buf;
				char *tmp_last = buf + vsize * 10 - 1;
				do {
					uint64_t carry = 0;
					for (size_t i = max_vindex; i > 0; i--) {
						carry = v[i] % 10;
						v[i - 1] += carry * 0x100000000;
						v[i] /= 10;
					}
					*tmp_last-- = (v[0] % 10) + '0';
					v[0] /= 10;
					len++;
					while (v[max_vindex] == 0 && max_vindex > 0) {
						max_vindex--;
					}
					if (max_vindex == 0 && v[0] == 0) {
						break;
					}
				} while (1);

				tmp_str = tmp_last + 1;
			} else {
				if (lval < 0) {
					is_negative = true;
					lval = -lval;
				}
				buf = emalloc(32);
				tmp_str = buf + 31;
				while (lval > 0) {
					*tmp_str-- = (lval % 10) + '0';
					lval /= 10;
					len++;
				}
				tmp_str++;
			}

			size_t leading_zeros = 0;
			size_t trailing_zeros = 0;
			ssize_t before_decimal_point_len = 0;
			bool has_decimal_point = false;
			if (exponent > 0){
				trailing_zeros = exponent;
			} else if (exponent < 0) {
				leading_zeros = -exponent >= len ? -exponent - len + 1 : 0;
				before_decimal_point_len = len + exponent;
				has_decimal_point = true;
			}

			zend_string *str;
			char *str_ptr;
			bool has_sign_str = is_negative;

			str = zend_string_alloc(len + leading_zeros + trailing_zeros + has_decimal_point + has_sign_str, 0);
			str_ptr = ZSTR_VAL(str);

			if (is_negative) {
				*str_ptr++ = '-';
			}

			if (leading_zeros > 0) {
				if (before_decimal_point_len < 0) {
					memset(str_ptr, '0', leading_zeros + before_decimal_point_len);
					str_ptr += leading_zeros + before_decimal_point_len;
					leading_zeros = -before_decimal_point_len;
					*str_ptr++ = '.';
				}
				memset(str_ptr, '0', leading_zeros);
				str_ptr += leading_zeros;
			}

			if (has_decimal_point) {
				if (before_decimal_point_len > 0) {
					memcpy(str_ptr, tmp_str, before_decimal_point_len);
					str_ptr += before_decimal_point_len;
					tmp_str += before_decimal_point_len;
					len -= before_decimal_point_len;
				}
				*str_ptr++ = '.';
			}

			if (len > 0) {
				memcpy(str_ptr, tmp_str, len);
				str_ptr += len;
			}

			if (trailing_zeros > 0) {
				memset(str_ptr, '0', trailing_zeros);
				str_ptr += trailing_zeros;
			}

			*str_ptr = '\0';
			ZVAL_STR(result, str);

			if (buf) {
				efree(buf);	
			}
			if (v) {
				efree(v);
			}
			break;

		case TSURUGI_FFI_ATOM_TYPE_CHARACTER:
			rc = tsurugi_ffi_sql_query_result_fetch_character(H->context, S->result, &cval);
			if (rc != 0) {
				goto fail;
			}
			ZVAL_STRING(result, cval);
			break;

		case TSURUGI_FFI_ATOM_TYPE_OCTET:
			TsurugiFfiByteArrayHandle octet;
			size_t octet_size;
			rc = tsurugi_ffi_sql_query_result_fetch_octet(H->context, S->result, &octet, &octet_size);
			if (rc != 0) {
				goto fail;
			}
			char *octet_buf = emalloc(octet_size * 2 + 1);
			char *octet_ptr = octet_buf;
			for (size_t i = 0; i < octet_size; i++) {
				octet_ptr += sprintf(octet_ptr, "%02x", (unsigned char) octet[i]);
			}
			*octet_ptr = '\0';
			ZVAL_STRING(result, octet_buf);
			efree(octet_buf);
			break;

		case TSURUGI_FFI_ATOM_TYPE_DATE:
			rc = tsurugi_ffi_sql_query_result_fetch_date(H->context, S->result, &lval);
			if (rc != 0) {
				goto fail;
			}
			lval *= 86400;
			t = gmtime((time_t *) &lval);
			char date_buf[16];
			size_t date_len = strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", t);
			ZVAL_STRINGL(result, date_buf, date_len);
			break;

		case TSURUGI_FFI_ATOM_TYPE_TIME_OF_DAY:
			rc = tsurugi_ffi_sql_query_result_fetch_time_of_day(H->context, S->result, &lval);
			if (rc != 0) {
				goto fail;
			}
			lval /= 1000 * 1000 * 1000;
			char time_buf[16];
			snprintf(time_buf, sizeof(time_buf), "%02ld:%02ld:%02ld", lval / 3600, (lval % 3600) / 60, lval % 60);
			ZVAL_STRINGL(result, time_buf, strlen(time_buf));
			break;

		case TSURUGI_FFI_ATOM_TYPE_TIME_POINT:
			rc = tsurugi_ffi_sql_query_result_fetch_time_point(H->context, S->result, &lval, &nano_sec);
			if (rc != 0) {
				goto fail;
			}
			t = gmtime((time_t *) &lval);
			char time_point_buf[32];
			size_t time_point_len = strftime(time_point_buf, sizeof(time_point_buf), "%Y-%m-%d %H:%M:%S", t);
			ZVAL_STRINGL(result, time_point_buf, time_point_len);
			break;

		case TSURUGI_FFI_ATOM_TYPE_TIME_POINT_WITH_TIME_ZONE:
			rc = tsurugi_ffi_sql_query_result_fetch_time_point_with_time_zone(H->context, S->result, &lval, &nano_sec, &tz_offset);
			if (rc != 0) {
				goto fail;
			}
			t = gmtime((time_t *) &lval);
			char time_point_with_tz_buf[32];
			size_t time_point_with_tz_len = strftime(time_point_with_tz_buf, sizeof(time_point_with_tz_buf), "%Y-%m-%d %H:%M:%S", t);

			char *time_point_with_tz_offset_ptr = time_point_with_tz_buf + time_point_with_tz_len;
			if (tz_offset >= 0) {
				*time_point_with_tz_offset_ptr++ = '+';
			} else {
				*time_point_with_tz_offset_ptr++ = '-';
			}
			snprintf(time_point_with_tz_offset_ptr, 6, "%02d:%02d", tz_offset / 60, tz_offset % 60);
			ZVAL_STRINGL(result, time_point_with_tz_buf, time_point_with_tz_len + 6);
			break;

		case TSURUGI_FFI_ATOM_TYPE_BIT:
		case TSURUGI_FFI_ATOM_TYPE_DATETIME_INTERVAL:
		case TSURUGI_FFI_ATOM_TYPE_TIME_OF_DAY_WITH_TIME_ZONE:
		case TSURUGI_FFI_ATOM_TYPE_CLOB:
		case TSURUGI_FFI_ATOM_TYPE_BLOB:
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "Unsupported type");
			return 0;

		default:
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "Unknown type");
			return 0;
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

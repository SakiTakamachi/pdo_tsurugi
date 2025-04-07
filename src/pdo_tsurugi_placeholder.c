#include "php.h"
#include "php_ini.h"
#include "php_pdo_tsurugi_int.h"
#include "zend_portability.h"
#include "zend_interfaces.h"
#include "ext/date/php_date.h"

HashTable *pdo_tsurugi_get_placeholders_hash_table(zend_object *obj)
{
	pdo_tsurugi_placeholders *intern = (pdo_tsurugi_placeholders *) ((char*) (obj) - XtOffsetOf(pdo_tsurugi_placeholders, std));
	return intern->placeholders;
}

static zend_always_inline void pdo_tsurugi_add_named_placeholder_method(INTERNAL_FUNCTION_PARAMETERS, pdo_tsurugi_data_type type)
{
	zend_string *str;
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(str);
	ZEND_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(str) == 0) {
		zend_value_error("Placeholder name cannot be empty string");
		RETURN_THROWS();
	}

	pdo_tsurugi_placeholders *intern = (pdo_tsurugi_placeholders *) ((char*) Z_OBJ_P(ZEND_THIS) - XtOffsetOf(pdo_tsurugi_placeholders, std));

	zval type_zv;
	ZVAL_LONG(&type_zv, type);
	zval *ret = zend_hash_add(intern->placeholders, str, &type_zv);

	if (ret = NULL) {
		zend_value_error("Duplicate placeholder");
		RETURN_THROWS();
	}

	RETURN_ZVAL(ZEND_THIS, 1, 0);
}

static zend_always_inline void pdo_tsurugi_add_positional_placeholder_method(INTERNAL_FUNCTION_PARAMETERS, pdo_tsurugi_data_type type)
{
	zend_long lval;
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(lval);
	ZEND_PARSE_PARAMETERS_END();

	if (lval < 0) {
		zend_value_error("Placeholder index cannot be negative");
		RETURN_THROWS();
	}

	pdo_tsurugi_placeholders *intern = (pdo_tsurugi_placeholders *) ((char*) Z_OBJ_P(ZEND_THIS) - XtOffsetOf(pdo_tsurugi_placeholders, std));

	zval type_zv;
	ZVAL_LONG(&type_zv, type);
	zval *ret = zend_hash_index_add(intern->placeholders, lval, &type_zv);

	if (ret = NULL) {
		zend_value_error("Duplicate placeholder");
		RETURN_THROWS();
	}

	RETURN_ZVAL(ZEND_THIS, 1, 0);
}

#define PDO_TSURUGI_PLACEHOLDER_METHOD(method, type) \
	PHP_METHOD(Pdo_Tsurugi_NamedPlaceholders, method) \
	{ \
		pdo_tsurugi_add_named_placeholder_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, (type)); \
	} \
	PHP_METHOD(Pdo_Tsurugi_PositionalPlaceholders, method) \
	{ \
		pdo_tsurugi_add_positional_placeholder_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, (type)); \
	}

PDO_TSURUGI_PLACEHOLDER_METHOD(addBoolean, PDO_TSURUGI_PLACEHOLDER_TYPE_BOOLEAN)
PDO_TSURUGI_PLACEHOLDER_METHOD(addInteger, PDO_TSURUGI_PLACEHOLDER_TYPE_INTEGER)
PDO_TSURUGI_PLACEHOLDER_METHOD(addBigInteger, PDO_TSURUGI_PLACEHOLDER_TYPE_BIG_INTEGER)
PDO_TSURUGI_PLACEHOLDER_METHOD(addString, PDO_TSURUGI_PLACEHOLDER_TYPE_STRING)
PDO_TSURUGI_PLACEHOLDER_METHOD(addBinary, PDO_TSURUGI_PLACEHOLDER_TYPE_BINARY)
PDO_TSURUGI_PLACEHOLDER_METHOD(addFloat, PDO_TSURUGI_PLACEHOLDER_TYPE_FLOAT)
PDO_TSURUGI_PLACEHOLDER_METHOD(addDouble, PDO_TSURUGI_PLACEHOLDER_TYPE_DOUBLE)
PDO_TSURUGI_PLACEHOLDER_METHOD(addDecimal, PDO_TSURUGI_PLACEHOLDER_TYPE_DECIMAL)
PDO_TSURUGI_PLACEHOLDER_METHOD(addTime, PDO_TSURUGI_PLACEHOLDER_TYPE_TIME)
PDO_TSURUGI_PLACEHOLDER_METHOD(addTimeWithTimeZone, PDO_TSURUGI_PLACEHOLDER_TYPE_TIME_WITH_TIME_ZONE)
PDO_TSURUGI_PLACEHOLDER_METHOD(addDate, PDO_TSURUGI_PLACEHOLDER_TYPE_DATE)
PDO_TSURUGI_PLACEHOLDER_METHOD(addTimestamp, PDO_TSURUGI_PLACEHOLDER_TYPE_TIMESTAMP)
PDO_TSURUGI_PLACEHOLDER_METHOD(addTimestampWithTimeZone, PDO_TSURUGI_PLACEHOLDER_TYPE_TIMESTAMP_WITH_TIME_ZONE)

bool pdo_tsurugi_register_placeholders(
	pdo_dbh_t *dbh, zval *placeholder_name, TsurugiFfiSqlPlaceholderHandle *placeholder_handle, pdo_tsurugi_data_type type)
{
	TsurugiFfiRc rc;
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) dbh->driver_data;
	TsurugiFfiAtomType atom_type;

	switch (type) {
		case PDO_TSURUGI_PLACEHOLDER_TYPE_BOOLEAN:
			atom_type = TSURUGI_FFI_ATOM_TYPE_BOOLEAN;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_INTEGER:
			atom_type = TSURUGI_FFI_ATOM_TYPE_INT4;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_BIG_INTEGER:
			atom_type = TSURUGI_FFI_ATOM_TYPE_INT8;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_STRING:
			atom_type = TSURUGI_FFI_ATOM_TYPE_CHARACTER;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_BINARY:
			atom_type = TSURUGI_FFI_ATOM_TYPE_OCTET;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_FLOAT:
			atom_type = TSURUGI_FFI_ATOM_TYPE_FLOAT4;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_DOUBLE:
			atom_type = TSURUGI_FFI_ATOM_TYPE_FLOAT8;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_DECIMAL:
			atom_type = TSURUGI_FFI_ATOM_TYPE_DECIMAL;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_TIME:
			atom_type = TSURUGI_FFI_ATOM_TYPE_TIME_OF_DAY;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_TIME_WITH_TIME_ZONE:
			atom_type = TSURUGI_FFI_ATOM_TYPE_TIME_OF_DAY_WITH_TIME_ZONE;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_DATE:
			atom_type = TSURUGI_FFI_ATOM_TYPE_DATE;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_TIMESTAMP:
			atom_type = TSURUGI_FFI_ATOM_TYPE_TIME_POINT;
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_TIMESTAMP_WITH_TIME_ZONE:
			atom_type = TSURUGI_FFI_ATOM_TYPE_TIME_POINT_WITH_TIME_ZONE;
			break;
		EMPTY_SWITCH_DEFAULT_CASE();
	}

	rc = tsurugi_ffi_sql_placeholder_of_atom_type(H->context, Z_STRVAL_P(placeholder_name) + 1, atom_type, placeholder_handle);
	return rc == 0;
}

static bool php_tsurugi_convert_decimal_to_binary(
	pdo_stmt_t *stmt, zval *value, int64_t *upper_dist, uint64_t *lower_dist, int32_t *exponent_dist)
{
	zend_string *str = zval_get_string(value);
	char *ptr = ZSTR_VAL(str);
	size_t len = ZSTR_LEN(str);

	char *num_str = emalloc(len);
	size_t num_str_len = 0;
	char *num_str_ptr = num_str;
	char *decimal_point_ptr = NULL;
	char *exponent_ptr = NULL;
	bool exponent_is_negative = false;

	bool num_is_negative = false;
	if (*ptr == '-') {
		num_is_negative = true;
		ptr++;
		len--;
	} else if (*ptr == '+') {
		ptr++;
		len--;
	}

	for (size_t i = 0; i < len; i++) {
		if (*ptr >= '0' && *ptr <= '9') {
			*num_str_ptr++ = *ptr - '0';
			num_str_len++;
		} else if (*ptr == '.') {
			if (decimal_point_ptr) {
				goto fail;
			}
			decimal_point_ptr = ptr;
		} else if (*ptr == 'e' || *ptr == 'E') {
			if (exponent_ptr) {
				goto fail;
			}
			exponent_ptr = ptr + 1;
			if (*exponent_ptr == '-') {
				exponent_is_negative = true;
				exponent_ptr++;
			}
			break;
		} else {
			goto fail;
		}
		ptr++;
	}
	char *num_end = num_str + num_str_len;
	char *end = ptr;

	size_t exponent_len = len - (exponent_ptr - ZSTR_VAL(str));
	int64_t exponent_abs_max = INT_MAX;
	if (exponent_is_negative) {
		exponent_abs_max++;
	}
	int64_t exponent = 0;
	if (exponent_ptr) {
		for (size_t i = 0; i < exponent_len; i++) {
			if (exponent_ptr[i] < '0' || exponent_ptr[i] > '9') {
				goto fail;
			}
			exponent = exponent * 10 + (exponent_ptr[i] - '0');
			if (exponent > exponent_abs_max) {
				goto exponent_out_of_range;
			}
		}
		if (exponent_is_negative) {
			exponent = -exponent;
		}
	}

	size_t after_decimal_point_len = 0;
	if (decimal_point_ptr) {
		after_decimal_point_len = end - decimal_point_ptr - 1;
	}
	if (exponent < INT_MIN + (int64_t) after_decimal_point_len) {
		goto exponent_out_of_range;
	}
	exponent -= after_decimal_point_len;

	size_t trailing_zeros = 0;
	while (num_end[-1] == 0 && num_end > num_str) {
		num_end--;
		trailing_zeros++;
	}
	if (exponent > INT_MAX - (int64_t) trailing_zeros) {
		goto exponent_out_of_range;
	}
	exponent += trailing_zeros;

	num_str_ptr = num_str;
	while (*num_str_ptr == 0 && num_str_ptr < num_end) {
		num_str_ptr++;
	}
	size_t num_len = num_end - num_str_ptr;
	if (UNEXPECTED(num_len == 0)) {
		*lower_dist = 0;
		*upper_dist = 0;
		*exponent_dist = 0;
		zend_string_release(str);
		efree(num_str);
		return true;
	}

#ifdef __SIZEOF_INT128__
	__int128_t lval = 0;
	while (num_str_ptr < num_end) {
		lval = lval * 10 + *num_str_ptr++;
	}
	if (num_is_negative) lval = -lval;

	*lower_dist = (uint64_t) (lval & 0xFFFFFFFFFFFFFFFFULL);
	*upper_dist = (int64_t) (lval >> 64);
#else
	uint32_t parts[4] = {0};

	while (num_str_ptr < num_end) {
		uint32_t digit = *num_str_ptr++;
		uint64_t carry = digit;
		for (int i = 0; i < 4; ++i) {
			uint64_t val = (uint64_t) parts[i] * 10 + carry;
			parts[i] = (uint32_t) (val & 0xFFFFFFFF);
			carry = val >> 32;
		}
	}

	if (num_is_negative) {
		for (int i = 0; i < 4; ++i) {
			parts[i] = ~parts[i];
		}
		for (int i = 0; i < 4; ++i) {
			if (++parts[i] != 0) {
				break;
			}
		}
	}

	*lower_dist = ((uint64_t) parts[1] << 32) | parts[0];
	*upper_dist = ((uint64_t) parts[3] << 32) | parts[2];
#endif

	*exponent_dist = (int32_t) exponent;
	zend_string_release(str);
	efree(num_str);
	return true;

fail:
	pdo_raise_impl_error(stmt->dbh, stmt, "22023", "Invalid decimal string");
	zend_string_release(str);
	efree(num_str);
	return false;

exponent_out_of_range:
	pdo_raise_impl_error(stmt->dbh, stmt, "22023", "Exponent out of range");
	zend_string_release(str);
	efree(num_str);
	return false;
}

static bool php_tsurugi_get_timestamp(pdo_stmt_t *stmt, zval *value, zend_long *timestamp, int32_t *timezone_offset, bool time_only)
{
	if (Z_TYPE_P(value) == IS_LONG) {
		*timestamp = Z_LVAL_P(value);
		*timezone_offset = 0;
		return true;
	} else if (Z_TYPE_P(value) != IS_STRING) {
		pdo_raise_impl_error(stmt->dbh, stmt, "HY105", "datetime must be string or integer");
		return false;
	}

	zend_long lval;
	if (is_numeric_string(Z_STRVAL_P(value), Z_STRLEN_P(value), &lval, NULL, 0)) {
		*timestamp = lval;
		*timezone_offset = 0;
		return true;
	}

	zval timezone_zv;
	zend_class_entry *tz_ce = zend_hash_str_find_ptr(CG(class_table), "datetimezone", sizeof("datetimezone") - 1);

	if (!tz_ce) {
		php_error_docref(NULL, E_ERROR, "DateTimeZone class not found");
		return FAILURE;
	}

	object_init_ex(&timezone_zv, tz_ce);

	zval timezone_name;
	ZVAL_STRING(&timezone_name, "UTC");

	zend_call_method_with_1_params(
		Z_OBJ(timezone_zv), tz_ce, NULL,
		"__construct", NULL, &timezone_name);

	zval_ptr_dtor(&timezone_name);

	zval datetime_zv;
	php_date_obj *date_obj;
	object_init_ex(&datetime_zv, php_date_get_date_ce());
	if (!php_date_initialize(Z_PHPDATE_P(&datetime_zv), Z_STRVAL_P(value), Z_STRLEN_P(value), NULL, &timezone_zv, 0)) {
		zval_ptr_dtor(&datetime_zv);
		pdo_raise_impl_error(stmt->dbh, stmt, "22007", "Could not parse as datetime");
		return false;
	}

	date_obj = Z_PHPDATE_P(&datetime_zv);
	if (time_only) {
		zend_long hours = date_obj->time->h;
		zend_long minutes = date_obj->time->i;
		zend_long seconds = date_obj->time->s;
		*timestamp = (hours * 3600) + (minutes * 60) + seconds;
	} else {
		*timestamp = (zend_long) date_obj->time->sse + date_obj->time->z;
	}
	*timezone_offset = date_obj->time->z;
	zval_ptr_dtor(&timezone_zv);
	zval_ptr_dtor(&datetime_zv);
	return true;
}

bool pdo_tsurugi_register_parameter(
	pdo_stmt_t *stmt, zend_string *parameter_name, TsurugiFfiSqlParameterHandle *parameter_handle, pdo_tsurugi_data_type type, zval *value)
{
	TsurugiFfiRc rc;
	pdo_tsurugi_db_handle *H = (pdo_tsurugi_db_handle *) stmt->dbh->driver_data;
	char *parameter_name_str = ZSTR_VAL(parameter_name) + 1;

	if (Z_TYPE_P(value) == IS_NULL) {
		rc = tsurugi_ffi_sql_parameter_null(H->context, parameter_name_str, parameter_handle);
		return rc == 0;
	}

	zend_long timestamp;
	int32_t timezone_offset;
	switch (type) {
		case PDO_TSURUGI_PLACEHOLDER_TYPE_BOOLEAN:
			/* Not implemented */
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_INTEGER:
			rc = tsurugi_ffi_sql_parameter_of_int4(H->context, parameter_name_str, (int32_t) zval_get_long(value), parameter_handle);
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_BIG_INTEGER:
			rc = tsurugi_ffi_sql_parameter_of_int8(H->context, parameter_name_str, (int64_t) zval_get_long(value), parameter_handle);
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_STRING:
			zend_string *str = zval_get_string(value);
			rc = tsurugi_ffi_sql_parameter_of_character(H->context, parameter_name_str, ZSTR_VAL(str), parameter_handle);
			zend_string_release(str);
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_BINARY:
			zend_string *binary_str = zval_get_string(value);

			size_t binary_bytes_size = (ZSTR_LEN(binary_str) + 1) / 2;
			char *binary_val_ptr = ZSTR_VAL(binary_str);

			if (ZSTR_LEN(binary_str) >= 2 && binary_val_ptr[0] == '0' && binary_val_ptr[1] == 'x') {
				binary_val_ptr += 2;
				binary_bytes_size--;
			}

			uint8_t *binary_bytes = emalloc(binary_bytes_size);

			size_t i = 0;
			char byte[2];
			char *dummy;
			if (ZSTR_LEN(binary_str) & 1) {
				byte[0] = '0';
				byte[1] = binary_val_ptr[0];
				binary_bytes[0] = (uint8_t) strtol(byte, &dummy, 16);
				i++;
			}
			for (; i < binary_bytes_size; i++) {
				byte[0] = binary_val_ptr[0];
				byte[1] = binary_val_ptr[1];
				binary_bytes[i] = (uint8_t) strtol(byte, &dummy, 16);
				binary_val_ptr += 2;
			}

			rc = tsurugi_ffi_sql_parameter_of_octet(H->context, parameter_name_str, binary_bytes, binary_bytes_size, parameter_handle);
			efree(binary_bytes);
			zend_string_release(binary_str);
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_FLOAT:
			rc = tsurugi_ffi_sql_parameter_of_float4(H->context, parameter_name_str, (float) zval_get_double(value), parameter_handle);
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_DOUBLE:
			rc = tsurugi_ffi_sql_parameter_of_float8(H->context, parameter_name_str, zval_get_double(value), parameter_handle);
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_DECIMAL:
			int64_t upper = 0;
			uint64_t lower = 0;
			int32_t exponent = 0;
			if (php_tsurugi_convert_decimal_to_binary(stmt, value, &upper, &lower, &exponent)) {
				rc = tsurugi_ffi_sql_parameter_of_decimal_i128(H->context, parameter_name_str, upper, lower, exponent, parameter_handle);
			} else {
				return false;
			}
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_TIME:
			if (php_tsurugi_get_timestamp(stmt, value, &timestamp, &timezone_offset, true)) {
				rc = tsurugi_ffi_sql_parameter_of_time_of_day(H->context, parameter_name_str, (uint64_t) timestamp * 1000 * 1000 * 1000, parameter_handle);
			} else {
				return false;
			}
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_TIME_WITH_TIME_ZONE:
			if (php_tsurugi_get_timestamp(stmt,value, &timestamp, &timezone_offset, true)) {
				rc = tsurugi_ffi_sql_parameter_of_time_of_day_with_time_zone(
					H->context, parameter_name_str, (uint64_t) timestamp * 1000 * 1000 * 1000, timezone_offset / 60, parameter_handle);
			} else {
				return false;
			}
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_DATE:
			if (php_tsurugi_get_timestamp(stmt,value, &timestamp, &timezone_offset, false)) {
				rc = tsurugi_ffi_sql_parameter_of_date(H->context, parameter_name_str, (int64_t) timestamp / 86400, parameter_handle);
			} else {
				return false;
			}
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_TIMESTAMP:
			if (php_tsurugi_get_timestamp(stmt,value, &timestamp, &timezone_offset, false)) {
				rc = tsurugi_ffi_sql_parameter_of_time_point(H->context, parameter_name_str, (int64_t) timestamp, 0, parameter_handle);
			} else {
				return false;
			}
			break;
		case PDO_TSURUGI_PLACEHOLDER_TYPE_TIMESTAMP_WITH_TIME_ZONE:
			if (php_tsurugi_get_timestamp(stmt,value, &timestamp, &timezone_offset, false)) {
				rc = tsurugi_ffi_sql_parameter_of_time_point_with_time_zone(
					H->context, parameter_name_str, (int64_t) timestamp, 0, timezone_offset / 60, parameter_handle);
			} else {
				return false;
			}
			break;
		EMPTY_SWITCH_DEFAULT_CASE();
	}

	return rc == 0;
}

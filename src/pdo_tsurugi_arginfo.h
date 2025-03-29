/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 59f6d94be9621b7ffd22fd3b2aa1cf3b598d0364 */

static zend_class_entry *register_class_Pdo_Tsurugi(zend_class_entry *class_entry_PDO)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Pdo", "Tsurugi", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_PDO, ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	zval const_TRANSACTION_TYPE_value;
	ZVAL_LONG(&const_TRANSACTION_TYPE_value, PDO_TSURUGI_TRANSACTION_TYPE);
	zend_string *const_TRANSACTION_TYPE_name = zend_string_init_interned("TRANSACTION_TYPE", sizeof("TRANSACTION_TYPE") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_TRANSACTION_TYPE_name, &const_TRANSACTION_TYPE_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_TRANSACTION_TYPE_name);

	return class_entry;
}

static zend_class_entry *register_class_Pdo_Tsurugi_TransactionType(void)
{
	zend_class_entry *class_entry = zend_register_internal_enum("Pdo\\Tsurugi\\TransactionType", IS_UNDEF, NULL);

	zend_enum_add_case_cstr(class_entry, "Short", NULL);

	zend_enum_add_case_cstr(class_entry, "Long", NULL);

	zend_enum_add_case_cstr(class_entry, "ReadOnly", NULL);

	return class_entry;
}


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_enum.h"
#include "php_pdo_tsurugi.h"
#include "php_pdo_tsurugi_int.h"
#include "pdo_tsurugi_arginfo.h"

static zend_class_entry *pdo_tsurugi_ce;
zend_class_entry *pdo_tsurugi_transaction_type_ce;

static const zend_module_dep pdo_tsurugi_deps[] = {
	ZEND_MOD_REQUIRED("pdo")
	ZEND_MOD_END
};

zend_module_entry pdo_tsurugi_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	pdo_tsurugi_deps,
	"PDO_Tsurugi",
	NULL,
	PHP_MINIT(pdo_tsurugi),
	PHP_MSHUTDOWN(pdo_tsurugi),
	NULL,
	NULL,
	PHP_MINFO(pdo_tsurugi),
	PHP_PDO_TSURUGI_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PDO_TSURUGI
ZEND_GET_MODULE(pdo_tsurugi)
#endif

PHP_MINIT_FUNCTION(pdo_tsurugi)
{
	if (FAILURE == php_pdo_register_driver(&pdo_tsurugi_driver)) {
		return FAILURE;
	}

	pdo_tsurugi_ce = register_class_Pdo_Tsurugi(pdo_dbh_ce);
	pdo_tsurugi_ce->create_object = pdo_dbh_new;

	pdo_tsurugi_transaction_type_ce = register_class_Pdo_Tsurugi_TransactionType();

	return php_pdo_register_driver_specific_ce(&pdo_tsurugi_driver, pdo_tsurugi_ce);
}

PHP_MSHUTDOWN_FUNCTION(pdo_tsurugi)
{
	php_pdo_unregister_driver(&pdo_tsurugi_driver);

	return SUCCESS;
}

PHP_MINFO_FUNCTION(pdo_tsurugi)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "PDO Driver for Tsurugi DB", "enabled");
	php_info_print_table_end();
}

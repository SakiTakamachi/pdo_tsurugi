
#ifndef PHP_PDO_TSURUGI_H
#define PHP_PDO_TSURUGI_H

extern zend_module_entry pdo_tsurugi_module_entry;
#define phpext_pdo_tsurugi_ptr &pdo_tsurugi_module_entry

#define PHP_PDO_TSURUGI_VERSION "0.0.1-dev"

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(pdo_tsurugi);
PHP_MSHUTDOWN_FUNCTION(pdo_tsurugi);
PHP_RINIT_FUNCTION(pdo_tsurugi);
PHP_RSHUTDOWN_FUNCTION(pdo_tsurugi);
PHP_MINFO_FUNCTION(pdo_tsurugi);

#endif	/* PHP_PDO_TSURUGI_H */

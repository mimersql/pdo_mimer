/*
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.php.net/license/3_01.txt                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Alexander Hedberg <alexander.hedberg@mimer.com>              |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_mimer.h"
#include "php_pdo_mimer_int.h"
/* #include "pdo_mimer_arginfo.h" */

/* TODO: check what more needs to be done here */
PHP_MINIT_FUNCTION(pdo_mimer) /* {{{ */
{
    if (FAILURE == php_pdo_register_driver(&pdo_mimer_driver)) {
        return FAILURE;
    }

    /* register custom attributes here */
    REGISTER_PDO_CLASS_CONST_LONG("PDO_MIMER_ATTR_TRANS_OPTION", (long) PDO_MIMER_ATTR_TRANS_OPTION);

    return SUCCESS;
}
/* }}} */

/* TODO: check what more needs to be done here */
PHP_MSHUTDOWN_FUNCTION(pdo_mimer) /* {{{ */
{
    php_pdo_unregister_driver(&pdo_mimer_driver);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(pdo_mimer)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "PDO Driver for Mimer", "enabled");
    php_info_print_table_row(2, "Mimer API Version", MimerAPIVersion());
    php_info_print_table_end();
}
/* }}} */

/* TODO: check what this does and if is needed */
#ifdef COMPILE_DL_PDO_MIMER
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(pdo_mimer)
#endif

/* TODO: check what this does and if is needed */
/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ pdo_firebird_deps */
static const zend_module_dep pdo_mimer_deps[] = {
        ZEND_MOD_REQUIRED("pdo")
        ZEND_MOD_END
};
/* }}} */

/* {{{ pdo_mimer_module_entry */
zend_module_entry pdo_mimer_module_entry = {
        STANDARD_MODULE_HEADER_EX,
        NULL,
        pdo_mimer_deps,
        "pdo_mimer",
        NULL,
        PHP_MINIT(pdo_mimer),
        PHP_MSHUTDOWN(pdo_mimer),
        NULL,
        NULL,
        PHP_MINFO(pdo_mimer),
        PHP_PDO_MIMER_VERSION,
        STANDARD_MODULE_PROPERTIES
};
/* }}} */
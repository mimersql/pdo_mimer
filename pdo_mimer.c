/*
   +--------------------------------------------------------------------------------+
   | MIT License                                                                    |
   +--------------------------------------------------------------------------------+
   | Copyright (c) 2023 Mimer Information Technology AB                             |
   +--------------------------------------------------------------------------------+
   | Permission is hereby granted, free of charge, to any person obtaining a copy   |
   | of this software and associated documentation files (the "Software"), to deal  |
   | in the Software without restriction, including without limitation the rights   |
   | to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      |
   | copies of the Software, and to permit persons to whom the Software is          |
   | furnished to do so, subject to the following conditions:                       |
   |                                                                                |
   | The above copyright notice and this permission notice shall be included in all |
   | copies or substantial portions of the Software.                                |
   |                                                                                |
   | THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     |
   | IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       |
   | FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    |
   | AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         |
   | LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  |
   | OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  |
   | SOFTWARE.                                                                      |
   +--------------------------------------------------------------------------------+
   | Author: Alexander Hedberg <alexander.hedberg@mimer.com>                        |
   +--------------------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_mimer.h"
#include "php_pdo_mimer_int.h"

#ifdef COMPILE_DL_PDO_MIMER
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(pdo_mimer)
#endif

#define REGISTER_ATTR(x) REGISTER_PDO_CLASS_CONST_LONG(#x, (x))

PHP_MINIT_FUNCTION(pdo_mimer) {
    if (FAILURE == php_pdo_register_driver(&pdo_mimer_driver)) {
        return FAILURE;
    }

    /* register custom attributes here */
    REGISTER_ATTR(MIMER_ATTR_TRANS_OPTION)
    REGISTER_ATTR(MIMER_TRANS_DEFAULT)
    REGISTER_ATTR(MIMER_TRANS_READWRITE)
    REGISTER_ATTR(MIMER_TRANS_READONLY)

    return SUCCESS;
}

/* TODO: check what more needs to be done here */
PHP_MSHUTDOWN_FUNCTION(pdo_mimer) {
    php_pdo_unregister_driver(&pdo_mimer_driver);

    return SUCCESS;
}

PHP_MINFO_FUNCTION(pdo_mimer) {
    php_info_print_table_start();
    php_info_print_table_header(2, "PDO Driver for Mimer SQL", "enabled");
    php_info_print_table_row(2, "Mimer API Version", MimerAPIVersion());
    php_info_print_table_end();
}

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
# define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

static const zend_module_dep pdo_mimer_deps[] = {
        ZEND_MOD_REQUIRED("pdo")
        ZEND_MOD_END
};

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

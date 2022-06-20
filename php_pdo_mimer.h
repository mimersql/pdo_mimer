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

#ifndef PHP_PDO_MIMER_H
# define PHP_PDO_MIMER_H

/* generated by ext_skel.php */
extern zend_module_entry pdo_mimer_module_entry;
# define phpext_pdo_mimer_ptr &pdo_mimer_module_entry

#include "php_version.h"
# define PHP_PDO_MIMER_VERSION PHP_VERSION

/* TODO: find out what this does */
# if defined(ZTS) && defined(COMPILE_DL_PDO_MIMER)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

/* TODO: other drivers have this, maybe instead of ^? Find out what this does.
#ifdef ZTS
#include "TSRM.h"
#endif
*/

/* declare pdo_mimer init/shutdown functions */
PHP_MINIT_FUNCTION(pdo_mimer);
PHP_MSHUTDOWN_FUNCTION(pdo_mimer);
PHP_MINFO_FUNCTION(pdo_mimer);

#endif	/* PHP_PDO_MIMER_H */

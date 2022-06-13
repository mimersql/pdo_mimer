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

extern zend_module_entry pdo_mimer_module_entry;
# define phpext_pdo_mimer_ptr &pdo_mimer_module_entry

# define PHP_PDO_MIMER_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_PDO_MIMER)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_PDO_MIMER_H */

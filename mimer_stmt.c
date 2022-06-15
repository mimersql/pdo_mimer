/*
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https:/*www.php.net/license/3_01.txt                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Alexander Hedberg <alexander.hedberg@mimer.com>              |
   +----------------------------------------------------------------------+
*/

#include "php.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_mimer.h"
#include "php_pdo_mimer_int.h"

/* called by PDO to execute a prepared query */
static int mimer_stmt_execute(pdo_stmt_t *stmt) /* {{{ */
{

}
/* }}} */

/* called by PDO to fetch the next row from a statement */
static int mimer_stmt_fetch(pdo_stmt_t *stmt,
    enum pdo_fetch_orientation ori, zend_long offset) /* {{{ */
{

}
/* }}} */


const struct pdo_stmt_methods mimer_stmt_methods = { /* {{{ */
        NULL,   /* statement destructor method */
        NULL,   /* statement executor method */
        NULL,   /* statement fetcher method */
        NULL,   /* statement describer method */
        NULL,   /* statement get column method */
        NULL,   /* statement parameter hook method */
        NULL,   /* statement set attribute method */
        NULL,   /* statement get attribute method */
        NULL,   /* statement get column data method */
        NULL,   /* next statement rowset method */
        NULL,   /* statement cursor closer method */
};
/* }}} */

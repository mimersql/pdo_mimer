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

#include "php.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_mimer.h"
#include "php_pdo_mimer_int.h"

/**
 * @brief PDO Mimer method to end a statement and free necessary memory
 * @param pdo_stmt_t* stmt
 * @return 1 for success
 */
static int pdo_mimer_stmt_dtor(pdo_stmt_t *stmt)
{
    if (stmt == NULL || !pdo_mimer_check_session(stmt->dbh)) {
        return 0;
    }

    pdo_mimer_handle *handle = stmt->dbh->driver_data;
    pdo_mimer_stmt *stmt_handle = (pdo_mimer_stmt *)stmt->driver_data;

    if (stmt_handle == NULL) {
        goto end;
    }

    if (stmt_handle->statement == NULL) {
        goto cleanup;
    }

    int32_t return_code = MimerEndStatement(&stmt_handle->statement);
    if (!MIMER_SUCCEEDED(return_code)) {
        handle->last_error = return_code;
        pdo_mimer_error(stmt->driver_data);
    }

cleanup:
    efree(stmt_handle);

end:
    return 1;
}

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
        pdo_mimer_stmt_dtor,   /* statement destructor method */
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

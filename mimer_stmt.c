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
#include "php_ini.h"
#include "ext/standard/info.h"
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

static int pdo_mimer_stmt_executer(pdo_stmt_t *stmt) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    int32_t return_code;

    /* TODO: check if statement will yield result set */

    return_code = MimerExecute(stmt_handle->statement);
    if (!MIMER_SUCCEEDED(return_code)) {
        stmt_handle->last_error = return_code;
        return 0;
    }

    return 1;
}

static int pdo_mimer_stmt_fetch(pdo_stmt_t *stmt, enum pdo_fetch_orientation ori, zend_long offset) {

}

static int pdo_mimer_describe_col(pdo_stmt_t *stmt, int colno) {

}

static int pdo_mimer_stmt_get_col_data(pdo_stmt_t *stmt, int colno, zval *result, enum pdo_param_type *type) {

}

static int pdo_mimer_stmt_param_hook(pdo_stmt_t *stmt, struct pdo_bound_param_data *param, enum pdo_param_event event_type) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    MimerStatement *statement = &stmt_handle->statement;
    int32_t return_code;

    if (stmt_handle->statement == NULL || !param->is_param) { /* nothing to do */
        return 1;
    }

    if (event_type != PDO_PARAM_EVT_EXEC_PRE) {
        return 1;
    }

    if (param->paramno >= INT16_MAX) {
        /* >= because +1 for parameter number later */
        /* TODO: better error */
        pdo_throw_exception(-25000, "Parameter number is larger than INT16_MAX",
                            (pdo_error_type *) SQLSTATE_INTERNAL_ERROR);
        return 0;
    }

    int16_t paramno = (int16_t)param->paramno + 1; /* parameter number is 0-indexed, while Mimer is not */

    switch (PDO_PARAM_TYPE(param->param_type)) {
        case PDO_PARAM_NULL:
            return_code = MimerSetNull(*statement, paramno);
            break;

        case PDO_PARAM_BOOL:
            return_code = MimerSetBoolean(*statement, paramno, Z_TYPE(param->parameter) == IS_TRUE);
            break;

        case PDO_PARAM_INT:
            return_code = MimerSetInt64(*statement, paramno, Z_LVAL(param->parameter));
            break;

        case PDO_PARAM_STR:
            return_code = MimerSetString8(*statement, paramno, Z_STRVAL(param->parameter));
            break;

#define UNSUPPORTED(x) \
        case (x): pdo_throw_exception(-25000, #x " is not yet supported",  (pdo_error_type *) SQLSTATE_INTERNAL_ERROR); \
            break;

        UNSUPPORTED(PDO_PARAM_LOB)
        UNSUPPORTED(PDO_PARAM_INPUT_OUTPUT)
        UNSUPPORTED(PDO_PARAM_STR_NATL)
        UNSUPPORTED(PDO_PARAM_STR_CHAR)
        UNSUPPORTED(PDO_PARAM_STMT)

        default:
            break;

    }

    if (!MIMER_SUCCEEDED(return_code)) {
        stmt_handle->last_error = return_code;
        return 0;
    }

    return 1;
}

static int pdo_mimer_set_attr(pdo_stmt_t *stmt, zend_long attr, zval *val) {

}

static int pdo_mimer_get_attr(pdo_stmt_t *stmt, zend_long attr, zval *val) {

}

static int pdo_mimer_get_column_meta(pdo_stmt_t *stmt, zend_long colno, zval *return_value) {

}

static int pdo_mimer_next_rowset(pdo_stmt_t *stmt) {

}

static int pdo_mimer_cursor_closer(pdo_stmt_t *stmt) {

}

const struct pdo_stmt_methods mimer_stmt_methods = {
        pdo_mimer_stmt_dtor,   /* statement destructor method */
        pdo_mimer_stmt_executer,   /* statement executor method */
        pdo_mimer_stmt_fetch,   /* statement fetcher method */
        pdo_mimer_describe_col,   /* statement describer method */
        pdo_mimer_stmt_get_col_data,   /* statement get column method */
        pdo_mimer_stmt_param_hook,   /* statement parameter hook method */
        pdo_mimer_set_attr,   /* statement set attribute method */
        pdo_mimer_get_attr,   /* statement get attribute method */
        pdo_mimer_get_column_meta,   /* statement get column data method */
        pdo_mimer_next_rowset,   /* next statement rowset method */
        pdo_mimer_cursor_closer,   /* statement cursor closer method */
};
/* }}} */

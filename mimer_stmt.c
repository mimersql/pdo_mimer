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
#include "php_pdo_mimer_errors.h"


/**
 * @brief PDO Mimer method to end a statement and free necessary memory
 * @param stmt A pointer to the PDO statement handle object.
 * @return 1 for success
 */
static int pdo_mimer_stmt_dtor(pdo_stmt_t *stmt) {
    pdo_mimer_stmt *stmt_handle = (pdo_mimer_stmt *)stmt->driver_data;

    handle_err_stmt(MimerEndStatement(&stmt_handle->statement))

    if (stmt_handle->error_info.error_msg != NULL) {
        pefree(stmt_handle->error_info.error_msg, stmt->dbh->is_persistent);
    }

    efree(stmt_handle);
    return 1;
}

/**
 * @brief Execute a prepared SQL statement.
 * @param stmt A pointer to the PDO statement handle object.
 * @return 1 for success, 0 for failure.
 */
static int pdo_mimer_stmt_executer(pdo_stmt_t *stmt) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    MimerError return_code;
    
    if(MimerStatementHasResultSet(stmt_handle->statement)) {
        int num_columns;

        pdo_mimer_open_cursor(stmt_handle, return_code)
        return_on_err_stmt(return_code, 0)

        return_on_err_stmt(num_columns = MimerColumnCount(stmt_handle->statement), 0)

        php_pdo_stmt_set_column_count(stmt, num_columns);
    } else {
        return_on_err_stmt(return_code = MimerExecute(stmt_handle->statement), 0)
    }

    return 1;
}

static int pdo_mimer_stmt_fetch(pdo_stmt_t *stmt, enum pdo_fetch_orientation ori, zend_long offset) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    MimerStatement *mimer_statement = &stmt_handle->statement;
    int32_t fetch_op_mode = MIMER_NEXT;
    MimerError return_code;

    if (!MimerStatementHasResultSet(*mimer_statement)) {
        return 0;
    }

    if (stmt_handle->cursor_type == MIMER_SCROLLABLE) {
        switch (ori) { /* map PDO fetch orientation to MimerFetchScroll operation mode */
            case PDO_FETCH_ORI_NEXT:
                fetch_op_mode = MIMER_NEXT;
                break;

            case PDO_FETCH_ORI_PRIOR:
                fetch_op_mode = MIMER_PREVIOUS;
                break;

            case PDO_FETCH_ORI_ABS:
                fetch_op_mode = MIMER_ABSOLUTE;
                break;

            case PDO_FETCH_ORI_FIRST:
                fetch_op_mode = MIMER_FIRST;
                break;

            case PDO_FETCH_ORI_LAST:
                fetch_op_mode = MIMER_LAST;
                break;

            case PDO_FETCH_ORI_REL:
            default:
                fetch_op_mode = MIMER_RELATIVE;
                break;
        }

        return_on_err_stmt(return_code = MimerFetchScroll(*mimer_statement, fetch_op_mode, (int32_t)offset), 0)
    } else { /* MIMER_FORWARD_ONLY */
        return_on_err_stmt(return_code = MimerFetch(*mimer_statement), 0)
    }

    return return_code != MIMER_NO_DATA;
}

static int pdo_mimer_describe_col(pdo_stmt_t *stmt, int colno) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    int mim_colno = colno + 1;
    MimerError return_code;

    MimerGetStr(MimerColumnName8, str_buf, return_code, stmt_handle->statement, mim_colno);
    return_on_err_stmt(return_code, 0);

	stmt->columns[colno] = (struct pdo_column_data) {
            .name = zend_string_init(str_buf, strlen(str_buf), 0),
            .maxlen = SIZE_MAX,
            .precision = 0
    };

    return 1;
}

static int pdo_mimer_stmt_get_col_data(pdo_stmt_t *stmt, int colno, zval *result, enum pdo_param_type *type) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    MimerError return_code;
    int mim_colno = colno + 1; 
    int32_t tst = MimerColumnType(stmt_handle->statement, mim_colno);
    
    /** TODO: Rewrite the test macros to include types that are missing from the MimerIsXX() checks */
    if (MimerIsInt64(tst) || tst == MIMER_NATIVE_INTEGER_NULLABLE || tst == MIMER_NATIVE_INTEGER){
        int64_t res;
        return_on_err_stmt(MimerGetInt64(stmt_handle->statement, mim_colno, &res), 0)
        ZVAL_LONG(result, res);
    }  else if (MimerIsString(tst)){
        MimerGetStr(MimerGetString8, str_buf, return_code, stmt_handle->statement, mim_colno);
        return_on_err_stmt(return_code, 0)

        ZVAL_STRING(result, str_buf);
    }

    else {
        mimer_throw_except(&stmt_handle->error_info, "Unknown column type", MIMER_FEATURE_NOT_IMPLEMENTED,
                          SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED, stmt->dbh->is_persistent, stmt->error_code)
    }
    
    return 1;
}

/**
 * @brief Handle bound parameters and columns
 * @param stmt A pointer to the PDO statement handle object.
 * @param param The structure describing either a statement parameter or a bound column.
 * @param event_type The type of event to occur for this parameter.
 * @return 1 for success, 0 for failure.
 * @see <a href="https://php-legacy-docs.zend.com/manual/php5/en/internals2.pdo.implementing">Implementing PDO</a>
 * @see <a href="https://www.php.net/manual/en/pdo.constants.php">PHP: Predefined Constants</a>
 */
static int pdo_mimer_stmt_param_hook(pdo_stmt_t *stmt, struct pdo_bound_param_data *param, enum pdo_param_event event_type) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    MimerStatement *statement = &stmt_handle->statement;
    MimerError return_code;
    zval *parameter;

    if (stmt_handle->statement == NULL || !param->is_param) { /* nothing to do */
        return 1;
    }

    if (event_type != PDO_PARAM_EVT_EXEC_PRE) {
        return 1;
    }

    if (param->paramno >= INT16_MAX) {
        /* TODO: custom error */
        strcpy(stmt->error_code, SQLSTATE_FEATURE_NOT_SUPPORTED);
        pdo_throw_exception(MIMER_VALUE_TOO_LARGE,
            "Parameter number is larger than INT16_MAX. Mimer only supports up to " QUOTE_EX(INT16_MAX) " parameters",
                            &stmt->error_code);

        return 0;
    }

    int16_t paramno = (int16_t)param->paramno + 1; /* parameter number is 0-indexed, while Mimer is not */

    /* unwrap reference if necessary */
    parameter = Z_ISREF(param->parameter) ? Z_REFVAL(param->parameter) : &param->parameter;

    switch (PDO_PARAM_TYPE(param->param_type)) {
        case PDO_PARAM_NULL:
            return_code = MimerSetNull(*statement, paramno);
            break;

        case PDO_PARAM_BOOL:
            return_code = MimerSetBoolean(*statement, paramno, Z_TYPE_P(parameter) == IS_TRUE);
            break;

        case PDO_PARAM_INT:
            return_code = MimerSetInt64(*statement, paramno, Z_LVAL_P(parameter));
            break;

        case PDO_PARAM_STR:
            return_code = MimerSetString8(*statement, paramno, Z_STRVAL_P(parameter));
            break;

        /* unimplemented */
#       define UNSUPPORTED(pdo_param) \
        case pdo_param:                \
            handle_custom_err(&stmt_handle->error_info, #pdo_param " support is not yet implemented", \
                MIMER_FEATURE_NOT_IMPLEMENTED, SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED, stmt->dbh->is_persistent, stmt->error_code) \
            return 0;

            UNSUPPORTED(PDO_PARAM_LOB)
            UNSUPPORTED(PDO_PARAM_INPUT_OUTPUT)
            UNSUPPORTED(PDO_PARAM_STR_NATL)
            UNSUPPORTED(PDO_PARAM_STR_CHAR)
            UNSUPPORTED(PDO_PARAM_STMT)
        default:
            return 0;

    }

    return_on_err_stmt(return_code, 0)

    return 1;
}

static int pdo_mimer_set_attr(pdo_stmt_t *stmt, zend_long attr, zval *val) {
    return 0;
}

static int pdo_mimer_get_attr(pdo_stmt_t *stmt, zend_long attr, zval *val) {
    return 0;
}

static int pdo_mimer_get_column_meta(pdo_stmt_t *stmt, zend_long colno, zval *return_value) {
    return 0;
}

static int pdo_mimer_next_rowset(pdo_stmt_t *stmt) {
    return 0;
}

static int pdo_mimer_cursor_closer(pdo_stmt_t *stmt) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    MimerError return_code;

    pdo_mimer_close_cursor(stmt_handle, return_code)
    return_on_err_stmt(return_code, 0)
    return 1;
}


/* the methods implemented by PDO Mimer for PDO statement handling */
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

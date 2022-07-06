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
 * @param pdo_stmt_t* stmt
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
 * @param stmt Pointer to the statement structure initialized by pdo_mimer_handle_preparer.
 * @return 1 for success, 0 for failure.
 */
static int pdo_mimer_stmt_executer(pdo_stmt_t *stmt) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;

    /* TODO: check if statement will yield result set */
    return_on_err_stmt(MimerExecute(stmt_handle->statement), 0)

    return 1;
}


/**
 * @brief This function will be called by PDO to fetch a row from a previously executed statement object.
 * @param stmt Pointer to the statement structure initialized by pdo_mimer_handle_preparer.
 * @param ori One of PDO_FETCH_ORI_xxx which will determine which row will be fetched.
 * @param offset If ori is set to PDO_FETCH_ORI_ABS or PDO_FETCH_ORI_REL, offset represents the row desired or the row
 *      relative to the current position, respectively. Otherwise, this value is ignored.
 * @return 1 for success or 0 in the event of failure.
 * @remark The results of this fetch are driver dependent and the data is usually stored in the driver_data member of
 *      the pdo_stmt_t object. The ori and offset parameters are only meaningful if the statement represents a
 *      scrollable cursor.
 * @see <a href="https://php-legacy-docs.zend.com/manual/php5/en/internals2.pdo.implementing">
 *          PDO Driver How-To: Fleshing out your skeleton (SKEL_stmt_fetch)
 *      </a>
 */
static int pdo_mimer_stmt_fetch(pdo_stmt_t *stmt, enum pdo_fetch_orientation ori, zend_long offset) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    MimerStatement *mimer_statement = &stmt_handle->statement;
    int32_t fetch_op_mode;
    MimerError return_code;

    if (!MimerStatementHasResultSet(*mimer_statement)) {
        return 0;
    }

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

    return return_code != MIMER_NO_DATA;
}

static int pdo_mimer_describe_col(pdo_stmt_t *stmt, int colno) {
    return 0;
}

static int pdo_mimer_stmt_get_col_data(pdo_stmt_t *stmt, int colno, zval *result, enum pdo_param_type *type) {
    return 0;
}

/**
 * @brief Handle bound parameters and columns
 * @param stmt stmt Pointer to the statement structure initialized by pdo_mimer_handle_preparer.
 * @param param The structure describing either a statement parameter or a bound column.
 * @param event_type The type of event to occur for this parameter.
 * @return 1 for success, 0 for failure.
 *
 * @details
 *  @code <h3>PDO_PARAM_EVT_NORMALIZE</h3> @endcode
 *      Event triggered during bound parameter registration allowing the driver to normalize the parameter name.
 *  @code <h3>PDO_PARAM_EVT_ALLOC</h3> @endcode
 *      Called when PDO allocates the binding. Occurs as part of
 *      <a href="https://php-legacy-docs.zend.com/manual/php5/en/pdostatement.bindparam.html">PDOStatement::bindParam()</a>,
 *      <a href="https://php-legacy-docs.zend.com/manual/php5/en/pdostatement.bindvalue.html">PDOStatement::bindValue()</a>
 *      or as part of an implicit bind when calling
 *      <a href="https://php-legacy-docs.zend.com/manual/php5/en/pdostatement.execute.html">PDOStatement::execute()</a>.
 *      This is your opportunity to take some action at this point; drivers that implement native prepared statements
 *      will typically want to query the parameter information, reconcile the type with that requested by the script,
 *      allocate an appropriately sized buffer and then bind the parameter to that buffer.
 *  @code <h3>PDO_PARAM_EVT_FREE</h3> @endcode
 *      Called once per parameter as part of cleanup. You should release any resources associated with that parameter now.
 *  @code <h3>PDO_PARAM_EXEC_PRE</h3> @endcode
 *      Called once for each parameter immediately before calling <code>pdo_mimer_stmt_executer</code>;  take this
 *      opportunity to make any final adjustments ready for execution. In particular, you should note that variables
 *      bound via
 *      <a href="https://php-legacy-docs.zend.com/manual/php5/en/pdostatement.bindparam.html">PDOStatement::bindParam()</a>
 *      are only legal to touch now, and not any sooner.
 *  @code <h3>PDO_PARAM_EXEC_POST</h3> @endcode
 *      Called once for each parameter immediately after calling <code>pdo_mimer_stmt_executer</code>; take this
 *      opportunity to make any post-execution actions that might be required by your driver.
 *  @code <h3>PDO_PARAM_FETCH_PRE</h3> @endcode
 *      Called once for each parameter immediately prior to calling <code>pdo_mimer_stmt_fetch</code>.
 *  @code <h3>PDO_PARAM_FETCH_POST</h3> @endcode
 *      Called once for each parameter immediately after calling <code>pdo_mimer_stmt_fetch</code>.
 *
 * @remark
 *      This hook will be called for each bound parameter and bound column in the statement. For <code>ALLOC</code> and
 *      <code>FREE</code> events, a single call will be made for each parameter or column. The param structure contains
 *      a <code>driver_data</code> field that the driver can use to store implementation specific
 *      information about each of the parameters.
 *      <br><br>
 *      For all other events, PDO may call you multiple times as the script issues
 *      <a href="https://php-legacy-docs.zend.com/manual/php5/en/pdostatement.execute.html">PDOStatement::execute()</a>
 *      and
 *      <a href="https://php-legacy-docs.zend.com/manual/php5/en/pdostatement.fetch.html">PDOStatement::fetch()</a>
 *      calls.
 *
 * @sa
 *      <a href="https://php-legacy-docs.zend.com/manual/php5/en/internals2.pdo.implementing">Implementing PDO</a>
 *      <br>
 *      <a href="https://www.php.net/manual/en/pdo.constants.php">PHP: Predefined Constants</a>
 */
static int pdo_mimer_stmt_param_hook(pdo_stmt_t *stmt, struct pdo_bound_param_data *param, enum pdo_param_event event_type) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    MimerStatement *statement = &stmt_handle->statement;
    MimerError return_code;

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
    return 0;
}

/**
 * @brief The methods implemented by PDO Mimer for statement handling
 *
 * @dtor The function called by PDO to destroy a previously constructed statement object.
 * @executer The function called by PDO to execute the prepared SQL statement in the passed statement object.
 * @fetcher The function called by PDO to fetch a row from a previously executed statement object.
 * @describer The function called by PDO to query information about a particular column.
 * @get_col The function called by PDO to retrieve data from the specified column.
 * @param_hook The function called by PDO for handling of both bound parameters and bound columns.
 * @set_attribute The function called by PDO to allow the setting of driver specific attributes for a statement object.
 * @get_attribute The function called by PDO to allow the retrieval of driver specific attributes for a statement object.
 * @get_column_meta The function called by PDO to retrieve meta data from the specified column.
 * @next_rowset The function called by PDO to advance to the next rowset of the batch.
 * @cursor_closer The function called by PDO to close the active cursor on a statement.
 */
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

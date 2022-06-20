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

/**
 * @brief Execute a prepared SQL statement.
 * @param stmt Pointer to the statement structure initialized by pdo_mimer_handle_preparer.
 * @return 1 for success, 0 for failure.
 */
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

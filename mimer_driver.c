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
 * @brief A function to handle all PDO Mimer SQL errors.
 * @param dbh A pointer to the PDO database handle object.
 * @param stmt A pointer to the PDO statement handle object.
 * @return Mimer SQL native error code
 * @remark If using a "custom" Mimer error, all error handling should be done beforehand. This function simply
 *      builds error information specific to the driver or statement handle.
 * @see https://docs.mimer.com/MimerSqlManual/v110/html/Manuals/App_Return_Codes/App_Return_Codes.htm
 */
int _pdo_mimer_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file, int line) {
    pdo_mimer_handle *mimer_db_handle = dbh->driver_data;
    MimerHandle mimer_handle;
    MimerErrorInfo *error_info;
    pdo_error_type *pdo_error;
    MimerError return_code;
    char* tmp_buf;

    /* handle either statement or session error */
    if (stmt) {
        pdo_error = &stmt->error_code;
        pdo_mimer_stmt *mimer_stmt_handle = stmt->driver_data;

        if (mimer_stmt_handle->statement == NULL) { /* default to session mimer_stmt_handle if statement unable to be created */
            mimer_handle = (MimerHandle) mimer_db_handle->session;
        } else {
            mimer_handle = (MimerHandle) mimer_stmt_handle->statement;
        }

        error_info = &mimer_stmt_handle->error_info;
    } else {
        pdo_error = &dbh->error_code;
        mimer_handle = (MimerHandle)mimer_db_handle->session;
        error_info = &mimer_db_handle->error_info;
    }

    /* free any previous error message */
    if (error_info->error_msg != NULL) {
        pefree(error_info->error_msg, dbh->is_persistent);
        error_info->error_msg = NULL;
    }

    /* handle errors and build error info */
    return_code = MimerGetError8(mimer_handle, &error_info->mimer_error, NULL, 0);
    if (return_code == MIMER_NO_ERROR) {
        tmp_buf = estrdup("Last operation completed successfully.");
    } else if (MIMER_SUCCEEDED(return_code)) { /* get error msg */
        size_t num_chars = return_code + 1; /* MimerGetError returns number of characters without null-termination */
        tmp_buf = ecalloc(num_chars, sizeof(char));
        MimerGetError8(mimer_handle, &error_info->mimer_error, tmp_buf, num_chars);
    } else { /* error getting error message */
        spprintf(&tmp_buf, 0,
                 "%s:%d Error retrieving error information. Return code(%d), Mimer error(%d)",
                 file, line, return_code, error_info->mimer_error);

        error_info->mimer_error = return_code;
    }

    strcpy(*pdo_error, MimerGetSQLState(error_info->mimer_error));
    error_info->error_msg = pestrdup(tmp_buf, dbh->is_persistent);
    efree(tmp_buf);

    if (!dbh->methods) { /* error constructing PDO object */
        pdo_throw_exception(error_info->mimer_error, error_info->error_msg, pdo_error);
    }

    return error_info->mimer_error;
}


/**
 * @brief PDO method to end a Mimer SQL session
 * @param dbh A pointer to the PDO database handle object.
 * @remark Frees any allocated memory
 */
static void mimer_handle_closer(pdo_dbh_t *dbh) {
    pdo_mimer_handle *handle = (pdo_mimer_handle*)dbh->driver_data;

    handle_err_dbh(MimerEndSession(&handle->session))
    if (handle->session != NULL) {
        handle_err_dbh(MimerEndSessionHard(&handle->session)); /* I wasn't asking */
    }

    if (handle->error_info.error_msg != NULL) {
        pefree(handle->error_info.error_msg, dbh->is_persistent);
    }

    pefree(handle, dbh->is_persistent);

    dbh->driver_data = NULL;
}


/**
 * @brief PDO method to prepare a SQL query with possible positional or named placeholders
 * @param dbh A pointer to the PDO database handle object.
 * @param sql The SQL query to be executed
 * @param stmt A pointer to the PDO statement handle object.
 * @param driver_options User-specified options to Mimer SQL
 * @return true if successfully prepared a statement, false if not
 */
static bool mimer_handle_preparer(pdo_dbh_t *dbh, zend_string *sql, pdo_stmt_t *stmt, zval *driver_options) {
    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;
    pdo_mimer_stmt *stmt_handle = ecalloc(1, sizeof(pdo_mimer_stmt));
    zend_string *new_sql = NULL;
    MimerError return_code;

    stmt_handle->handle = handle;
    stmt->driver_data = stmt_handle;
    stmt->methods = &mimer_stmt_methods;
    stmt->supports_placeholders = PDO_PLACEHOLDER_POSITIONAL; /* TODO: add named placeholders */

    if (pdo_parse_params(stmt, sql, &new_sql) == -1) { /* failed to parse */
        strcpy(dbh->error_code, stmt->error_code);
        return false;
    }

    /* if no option given, assign PDO_CURSOR_FWDONLY as default */
    int32_t cursor = (pdo_attr_lval(driver_options, PDO_ATTR_CURSOR, PDO_CURSOR_FWDONLY)
                      == PDO_CURSOR_FWDONLY) ? MIMER_FORWARD_ONLY : MIMER_SCROLLABLE;

    handle_err_stmt(return_code = MimerBeginStatement8(handle->session, ZSTR_VAL(new_sql ? new_sql : sql), cursor,
                                                       &stmt_handle->statement))

    if (new_sql != NULL) {
        zend_string_release(new_sql);
    }

    return MIMER_SUCCEEDED(return_code);
}


/**
 * @brief This function will be called by PDO to execute a raw SQL statement.
 * @param dbh A pointer to the PDO database handle object.
 * @param sql A zend_string containing the SQL statement to be prepared.
 * @return This function returns the number of rows affected or -1 upon failure.
 */
static zend_long mimer_handle_doer(pdo_dbh_t *dbh, const zend_string *sql) {
    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;
    MimerStatement statement;
    MimerError return_code;
    zend_long num_affected_rows;

    /* guard against MIMER_STATEMENT_CANNOT_BE_PREPARED so we can try to execute the statement */
    if (!MIMER_SUCCEEDED(return_code = MimerBeginStatement8(handle->session, ZSTR_VAL(sql), MIMER_FORWARD_ONLY, &statement))
            && return_code != MIMER_STATEMENT_CANNOT_BE_PREPARED) {
        pdo_mimer_error(dbh);
        return -1;
    }

    /* either a select query or a query that can't be used with MimerBeginStatement() */
    if (MimerStatementHasResultSet(statement) || return_code == MIMER_STATEMENT_CANNOT_BE_PREPARED) {
        if (statement != NULL) {
            return_on_err(MimerEndStatement(&statement), -1)
        }
        return_on_err(MimerExecuteStatement8(handle->session, ZSTR_VAL(sql)), -1) /* TODO: count affected rows */

        return 0;
    }

    /* MimerExecute() outputs number of affected rows upon success. */
    return_on_err(num_affected_rows = MimerExecute(statement), -1)

    return_on_err(MimerEndStatement(&statement), -1)

    return num_affected_rows;
}


/**
 * @brief PDO method to start a transaction
 * @param dbh A pointer to the PDO database handle object.
 * @return true if transaction was started, false if not
 */
static bool mimer_handle_begin(pdo_dbh_t *dbh) {
    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;

    return_on_err(MimerBeginTransaction(handle->session, handle->trans_option), false)

    return true;
}


/**
 * @brief This function will be called by PDO to commit a transaction.
 * @param dbh A pointer to the PDO database handle object.
 * @return true on success, false if failure
 */
static bool mimer_handle_commit(pdo_dbh_t *dbh) {
    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;

    return_on_err(MimerEndTransaction(handle->session, MIMER_COMMIT), false)

    return true;
}


/**
 * @brief This function will be called by PDO to rollback a transaction.
 * @param dbh A pointer to the PDO database handle object.
 * @return true on success, false if failure
 */
static bool mimer_handle_rollback(pdo_dbh_t *dbh) {
    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;

    return_on_err(MimerEndTransaction(handle->session, MIMER_ROLLBACK), false)

    return true;
}


/**
 * @brief PDO method to set driver attributes
 * @param dbh A pointer to the PDO database handle object
 * @param attribute A PDO or Mimer SQL attribute
 * @param value The value to set for the given attribute
 * @return true on successfully setting the attribute, false if not
 */
static bool pdo_mimer_set_attribute(pdo_dbh_t *dbh, zend_long attribute, zval *value) {
    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;

    switch (attribute) {
        /*
         * TODO: add autocommit functionality
         * (it's part of PDO's functionality)
         */

        /* custom driver attributes */
        case MIMER_ATTR_TRANS_OPTION: {
            long trans_option;
            if (!pdo_get_long_param(&trans_option, value)) { /* user didnt enter a number type */
                return false;
            }

            if (trans_option != MIMER_TRANS_READONLY && trans_option != MIMER_TRANS_READWRITE)
                return false;
            else {
                handle->trans_option = trans_option;
                break;
            }
        }

        /* TODO: add charset attribute support */

        default:
            return false;
    }

    return true;
}


/**
 * @brief PDO method to retrieve the latest error
 * @param dbh A pointer to the PDO database handle object.
 * @param stmt A pointer to the PDO statement handle object.
 * @param info The information array to add Mimer SQL error information to
 */
static void pdo_mimer_fetch_err(pdo_dbh_t *dbh, pdo_stmt_t *stmt, zval *info) {
    MimerErrorInfo *error_info;

    if (stmt) {
        pdo_mimer_stmt *handle = (pdo_mimer_stmt *)stmt->driver_data;
        error_info = &handle->error_info;
    } else {
        pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;
        error_info = &handle->error_info;
    }

    add_next_index_long(info, error_info->mimer_error);
    add_next_index_string(info, error_info->error_msg ? error_info->error_msg: "Last operation completed successfully.");
}


/**
 * @brief Get driver attributes (settings)
 * @return -1 for errors while retrieving a valid attribute
 * @return 0 for attempting to retrieve an attribute which is not supported by the driver
 * @return any other value for success, *return_value must be set to the attribute value
 */
static int pdo_mimer_get_attribute(pdo_dbh_t *dbh, zend_long attribute, zval *return_value) {
    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;

    switch (attribute) {
        /* TODO: add autocommit functionality */

        case PDO_ATTR_CLIENT_VERSION:
            ZVAL_STRING(return_value, MimerAPIVersion());
            break;

        case PDO_ATTR_DRIVER_NAME:
            ZVAL_STRING(return_value, "mimer");
            break;

        case PDO_ATTR_CONNECTION_STATUS: {
            /* TODO: find out how to do this */
            /* MySQLs status function https://dev.mysql.com/doc/refman/8.0/en/show-status.html */
            if (handle->session == NULL) {
                ZVAL_STRING(return_value, "Disconnected");
                break;
            }

            return_on_err(MimerPing(handle->session), -1)

            ZVAL_STRING(return_value, "Connected");
            break;
        }

        case PDO_ATTR_DEFAULT_STR_PARAM:
            /* TODO: charset */
            ZVAL_STRING(return_value, "UTF8");
            break;

        /* custom driver attributes */
        case MIMER_ATTR_TRANS_OPTION:
            ZVAL_STRING(return_value, handle->trans_option == MIMER_TRANS_READONLY ? "Read-only" : "Read and write");
            break;

        /* TODO: find more attributes for Mimer */

        default:
            return 0;
    }

    return 1;
}

/**
 * @brief A function to check if the user is still connected to a MimerSession
 * @param dbh A pointer to the PDO database handle object.
 * @return true or false
 */
static zend_result pdo_mimer_check_liveness(pdo_dbh_t *dbh) {
    pdo_mimer_handle *handle = dbh->driver_data;

    return_on_err(MimerPing(handle->session), FAILURE)

    return SUCCESS;
}


/* declares the methods Mimer uses and give them to the PDO driver */
static const struct pdo_dbh_methods mimer_methods = { /* {{{ */
        mimer_handle_closer,   /* handle closer method */
        mimer_handle_preparer,   /* handle preparer method */
        mimer_handle_doer,   /* handle doer method */
        NULL,   /* handle quoter method */
        mimer_handle_begin,   /* handle begin method */
        mimer_handle_commit,   /* handle commit method */
        mimer_handle_rollback,   /* handle rollback method */
        pdo_mimer_set_attribute,   /* handle set attribute method */
        NULL,   /* last_id not supported */
        pdo_mimer_fetch_err,   /* fetch error method */
        pdo_mimer_get_attribute,   /* handle get attribute method */
        pdo_mimer_check_liveness,   /* check liveness method */
        NULL,   /* get driver method */
        NULL,   /* persistent connection shutdown method */
        NULL,    /* in transaction method */
        NULL    /* get gc method */
};


/**
 * @brief This method handles PDO's construction for use with Mimer SQL
 * @param dbh A pointer to the PDO database handle object.
 * @param driver_options The user-defined driver options to be used with PDO Mimer
 * @example @code $PDO = new PDO("mimer:dbname=$dbname", $user, $pass, $attr_array); @endcode
 */
static int pdo_mimer_handle_factory(pdo_dbh_t *dbh, zval *driver_options) {
    MimerError return_code = MIMER_LOGIN_FAILED;
    int num_data_src_opts;

    /* Leave this guard here until we're ready to test persistent connections*/
    if(dbh->is_persistent){
        strcpy(dbh->error_code, SQLSTATE_FEATURE_NOT_SUPPORTED);
        pdo_throw_exception(MIMER_FEATURE_NOT_IMPLEMENTED,"Persistent connections not yet supported", &dbh->error_code);
    }

    /**
     * @brief Macro function to get a DSN option's value
     * @param optname The name of the option to get the value from
     * @return A string or NULL
     */
#   define optval(optname) data_src_opts[optname##_opt].optval

    enum { dbname_opt, user_opt, password_opt}; /* must match format `optname_opt` from data_src_opts */
    data_src_opt data_src_opts[] = {
            /* if the user does not give database name, NULL will trigger default database connection */
            { "dbname", NULL, 0 },
            { "user", "", 0 }, /* MimerBeginSession crashes on NULL user */
            { "password", NULL, 0 },

            /**
             * TODO: possible future functionality
             * { "protocol", PDO_MIMER_DEFAULT_PROTOCOL, 0},
             * { "host", PDO_MIMER_DEFAULT_HOST, 0 },
             * { "port", PDO_MIMER_DEFAULT_PORT, 0},
             */
    };

    num_data_src_opts = sizeof(data_src_opts) / sizeof(data_src_opt);

    pdo_mimer_handle *handle = pecalloc(1, sizeof(pdo_mimer_handle), dbh->is_persistent);
    handle->trans_option = pdo_attr_lval(driver_options, MIMER_ATTR_TRANS_OPTION, MIMER_TRANS_DEFAULT);

    dbh->driver_data = handle;
    dbh->skip_param_evt = 0b1111111 ^ (1 << PDO_PARAM_EVT_EXEC_PRE); /* skip all but exec_pre param events */

    /* get used data source name options */
    php_pdo_parse_data_source(dbh->data_source, dbh->data_source_len, data_src_opts,
                                  num_data_src_opts);

    /* setting user and password params in PDO constructor takes precedence over DSN options */
    if (!dbh->username && optval(user)) {
        dbh->username = pestrdup(optval(user), dbh->is_persistent);
    } if (!dbh->password && optval(password)) {
        dbh->password = pestrdup(optval(password), dbh->is_persistent);
    }

    /* TODO: add compatability for MimerBeginSession() and MimerBeginSessionC() */
    /* TODO: add session-persistence functionality */
    return_code = MimerBeginSession8(optval(dbname), dbh->username, dbh->password, &handle->session);
    if (MIMER_LOGIN_SUCCEEDED(return_code)) {
        dbh->methods = &mimer_methods;
    } else {
        pdo_mimer_error(dbh);
    }

    /* free up options given by user in dsn */
    for (int i = 0; i < num_data_src_opts; i++) {
        if (data_src_opts[i].freeme) {
            efree(data_src_opts[i].optval);
        }
    }

    return MIMER_LOGIN_SUCCEEDED(return_code);
}

/* register the driver in PDO */
const pdo_driver_t pdo_mimer_driver = {
        PDO_DRIVER_HEADER(mimer),
        pdo_mimer_handle_factory
};
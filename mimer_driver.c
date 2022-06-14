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

/* {{{ _pdo_mimer_error */
int _pdo_mimer_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file, int line)
{
    pdo_mimer_handle *handle = (pdo_mimer_handle*)dbh->driver_data;
    pdo_error_type *pdo_err;

    // TODO: add errors for statements

    // TODO: convert Mimer error codes (int32_t) to SQLSTATE (char[6])

    zend_string *err_msg = strpprintf(0, "Something went wrong -- %s:%d", file, line);
    php_printf("%s", ZSTR_VAL(err_msg));
    pdo_throw_exception(handle->last_error, ZSTR_VAL(err_msg), GENERAL_ERROR_SQLSTATE);

    return handle->last_error;
}
/* }}} */

/* {{{ _pdo_mimer_handle_checker */
bool _pdo_mimer_handle_checker(pdo_dbh_t *dbh, bool check_handle, bool check_session)
{
    if (!check_handle) {
        return dbh != NULL;
    }

    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;

    if (!check_session) {
        return handle != NULL;
    }

    return handle->session != NULL;
}
/* }}} */

/* {{{ mimer_handle_closer */
static void mimer_handle_closer(pdo_dbh_t *dbh)
{
    if (!pdo_mimer_check_handle(dbh)) {
        return;
    }

    pdo_mimer_handle *handle = (pdo_mimer_handle*)dbh->driver_data;
    if (handle->session == NULL) {
        goto cleanup;
    }

    int32_t return_code = MimerEndSession(&handle->session);

    if (!MIMER_SUCCEEDED(return_code)) {
        handle->last_error = return_code;
        pdo_mimer_error(dbh);
    }

    cleanup:
    /* FREE ANY PERSISTENT MEMORY USED BY PDO-MIMER */
    pefree(handle, dbh->is_persistent);

    dbh->driver_data = NULL;
}
/* }}} */

/* {{{ mimer_handle_doer */
static zend_long mimer_handle_doer(pdo_dbh_t *dbh, const zend_string *sql)
{
    /**
     * @brief Execute a statement that does not return a result set.
     * @return (zend_long) -1 on failure, otherwise the number of affected rows.
     *
     * @TODO Check if the Mimer API has a way to get number of affected rows
     * when calling MimerExecuteStatement[C|8]
     */

    if (!pdo_mimer_check_session(dbh)) {
        return 0L;
    }

    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;
    MimerStatement statement = NULL;

    int32_t return_code = MimerBeginStatement8(handle->session, ZSTR_VAL(sql), MIMER_FORWARD_ONLY, &statement);  /* TODO: add compatability for other charsets */

    if (!MIMER_SUCCEEDED(return_code)) {
        handle->last_error = return_code;

        if (return_code == MIMER_NO_DATA) {
            return 0;
        }

        return -1;
    }

    zend_long result_count = 0L;
    while (1) {
        return_code = MimerFetch(statement);

        if (!MIMER_SUCCEEDED(return_code)) {
            handle->last_error = return_code;
            return -1;
        }

        if (return_code == MIMER_NO_DATA) {
            break;
        }

        result_count++;
    }

    return_code = MimerEndStatement(&statement);

    if (!MIMER_SUCCEEDED(return_code)) {
        handle->last_error = return_code;
        return -1;
    }

    return result_count;
}
/* }}} */

/* {{{ mimer_handle_begin */
static bool mimer_handle_begin(pdo_dbh_t *dbh)
{
    if (!pdo_mimer_check_session(dbh)) {
        return false;
    }

    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;

    int32_t return_code = MimerBeginTransaction(handle->session, handle->trans_option);
    if (!MIMER_SUCCEEDED(return_code)) {
        handle->last_error = return_code;
        pdo_mimer_error(dbh);
        return false;
    }

    return true;
}

/* {{{ mimer_handle_commit */
static bool mimer_handle_commit(pdo_dbh_t *dbh)
{
    if (!pdo_mimer_check_session(dbh)) {
        return false;
    }

    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;
    int32_t return_code = MimerEndTransaction(handle->session, MIMER_COMMIT);

    if (!MIMER_SUCCEEDED(return_code)) {
        handle->last_error = return_code;
        pdo_mimer_error(dbh);
        return false;
    }

    return true;
}

/* {{{ mimer_handle_rollback */
static bool mimer_handle_rollback(pdo_dbh_t *dbh)
{
    if (!pdo_mimer_check_session(dbh)) {
        return false;
    }

    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;
    int32_t return_code = MimerEndTransaction(handle->session, MIMER_ROLLBACK);

    if (!MIMER_SUCCEEDED(return_code)) {
        handle->last_error = return_code;
        pdo_mimer_error(dbh);
        return false;
    }

    return true;
}
/* }}} */

/* {{{ pdo_mimer_set_attribute */
static bool pdo_mimer_set_attribute(pdo_dbh_t *dbh, zend_long attribute, zval *value)
{
    if (!pdo_mimer_check_pdo_handle(dbh)) {
        return false;
    }

    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;

    switch (attribute) {
        case PDO_ATTR_AUTOCOMMIT: {
            bool auto_commit;
            if (!pdo_get_bool_param(&auto_commit, value)) {
                return false;
            }

            dbh->auto_commit = auto_commit;
            break;
        }

        case PDO_ATTR_CURSOR: {
            long cursor_type;
            if (!pdo_get_long_param(&cursor_type, value)) {
                return false;
            }

            handle->cursor_type = cursor_type;
            break;
        }

        /* custom driver attributes */
        case PDO_MIMER_ATTR_TRANS_OPTION: {
            long trans_option;
            if (!pdo_get_long_param(&trans_option, value)) {
                return false;
            }

            handle->trans_option = (int32_t) trans_option;
            break;
        }

        /* TODO: add charset attribute support */

        default:
            return false;
    }

    return true;
}

/* {{{ pdo_mimer_fetch_err */
static void pdo_mimer_fetch_err(pdo_dbh_t *dbh, pdo_stmt_t *stmt, zval *info)
{
    if (!pdo_mimer_check_session(dbh)) {
        return;
    }

    int32_t last_error;

    if (stmt) {
        pdo_mimer_stmt *handle = (pdo_mimer_stmt *)stmt->driver_data;
        last_error = handle->last_error;
    } else {
        pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;
        last_error = handle->last_error;
    }

    if (last_error == MIMER_SUCCESS) {
        return;
    }

    zend_string *err_msg = strpprintf(0, "Error -- " __FILE__ ":%d", __LINE__);

    add_next_index_long(info, last_error);
    add_next_index_string(info, ZSTR_VAL(err_msg));
}
/* }}} */

/* {{{ pdo_mimer_get_attribute */
static int pdo_mimer_get_attribute(pdo_dbh_t *dbh, zend_long attribute, zval *return_value)
{
    /**
     * @brief Get driver attributes (settings)
     * @return -1 for errors while retrieving a valid attribute
     * @return 0 for attempting to retrieve an attribute which is not supported by the driver
     * @return any other value for success, *return_value must be set to the attribute value
     */

    if (!pdo_mimer_check_pdo_handle(dbh)) {
        return -1;
    }

    pdo_mimer_handle *handle = (pdo_mimer_handle *)dbh->driver_data;

    switch (attribute) {
        case PDO_ATTR_AUTOCOMMIT:
            ZVAL_BOOL(return_value, dbh->auto_commit);
            break;

        case PDO_ATTR_CLIENT_VERSION:
        case PDO_ATTR_SERVER_VERSION:
        case PDO_ATTR_SERVER_INFO:
            ZVAL_STRING(return_value, MimerAPIVersion());
            break;

        case PDO_ATTR_DRIVER_NAME:
            ZVAL_STRING(return_value, "mimer");
            break;

        case PDO_ATTR_CONNECTION_STATUS:
            /* TODO: find out how to do this */

            /* MySQLs status function https://dev.mysql.com/doc/refman/8.0/en/show-status.html */
            /* +--------------------------+------------+
               | Variable_name            | Value      |
               +--------------------------+------------+
               | Aborted_clients          | 0          |
               | Aborted_connects         | 0          |
               | Bytes_received           | 155372598  |
               | Bytes_sent               | 1176560426 |
               | Connections              | 30023      |
               | Created_tmp_disk_tables  | 0          |
               | Created_tmp_tables       | 8340       |
               | Created_tmp_files        | 60         |
               ...
               | Open_tables              | 1          |
               | Open_files               | 2          |
               | Open_streams             | 0          |
               | Opened_tables            | 44600      |
               | Questions                | 2026873    |
               ...
               | Table_locks_immediate    | 1920382    |
               | Table_locks_waited       | 0          |
               | Threads_cached           | 0          |
               | Threads_created          | 30022      |
               | Threads_connected        | 1          |
               | Threads_running          | 1          |
               | Uptime                   | 80380      |
               +--------------------------+------------+ */

            ZVAL_STRING(return_value, handle->session == NULL ? "Disconnected" : "Connected");
            break;

        case PDO_ATTR_DEFAULT_STR_PARAM:
            /* TODO: charset */
            ZVAL_STRING(return_value, "UTF8");
            break;

        case PDO_ATTR_CURSOR:
            ZVAL_STRING(return_value, handle->cursor_type == MIMER_FORWARD_ONLY ? "Forward-only" : "Scrollable");
            break;

        /* custom driver attributes */
        case PDO_MIMER_ATTR_TRANS_OPTION:
            ZVAL_STRING(return_value, handle->trans_option == MIMER_TRANS_READWRITE ? "Read and write" : "Read-only");
            break;

        /* TODO: find more attributes for Mimer */

        default:
            return 0;
    }

    return 1;
}
/* }}} */

/* {{{ pdo_mimer_check_liveness */
static zend_result pdo_mimer_check_liveness(pdo_dbh_t *dbh)
{
    if (!pdo_mimer_check_session(dbh)) {
        return FAILURE;
    }

    pdo_mimer_handle *handle = dbh->driver_data;
    int32_t return_code = MimerPing(handle->session);

    if (!MIMER_SUCCEEDED(return_code)) {
        handle->last_error = return_code;
        pdo_mimer_error(dbh);
        return FAILURE;
    }

    return SUCCESS;
}
/* }}} */

/* {{{ pdo_mimer_request_shutdown */
static void pdo_mimer_request_shutdown(pdo_dbh_t *dbh)
{
    /**
     * @note Memory freeing and pointer nullifying is done in mimer_handle_closer()
     */

    if (!pdo_mimer_check_session(dbh)) {
        return;
    }

    pdo_mimer_handle *handle = (pdo_mimer_handle*)dbh->driver_data;

    int32_t return_code = MimerEndSession(&handle->session);
    if (!MIMER_SUCCEEDED(return_code)) {
        handle->last_error = return_code;
        pdo_mimer_error(dbh);
    }
}
/* }}} */


/**
 * @brief Declare the methods Mimer uses and give them to the PDO driver
 */
static const struct pdo_dbh_methods mimer_methods = { /* {{{ */
        mimer_handle_closer,   /* handle closer method */
        NULL,   /* handle preparer method */
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
        pdo_mimer_request_shutdown,   /* request shutdown method */
        NULL,   /* in transaction method */
        NULL    /* get gc method */
};

/**
 * @brief This method handles PDO's construction for use with Mimer SQL
 *
 * @example @code $PDO = new PDO("mimer:host=localhost;dbname=test_db"); @endcode
 *
 * @remark DSN: <a href="https://www.php.net/manual/en/pdo.construct.php">Data Source Name</a>
 */
static int pdo_mimer_handle_factory(pdo_dbh_t *dbh, zval *driver_options) /* {{{ */
{
    /* This enum exists to add code readability */
//    typedef enum { protocol_opt, dbname_opt, host_opt, port_opt, ident_opt, /* user_opt, */ password_opt } opt_val; // TODO: possible future functionality
    typedef enum { dbname_opt } DataSourceOption;

    int num_data_src_opts;
    data_src_opt data_src_opts[] = {
            /* if the user does not give database name, NULL will trigger default database connection */
            {"dbname", NULL, 0 }

            /**
             * TODO: possible future functionality
             * { "protocol", PDO_MIMER_DEFAULT_PROTOCOL, 0},
             * { "host", PDO_MIMER_DEFAULT_HOST, 0 },
             * { "port", QUOTE(PDO_MIMER_DEFAULT_PORT), 0},
             * {"ident", PDO_MIMER_DEFAULT_IDENT, 0 }, // Mimer terminology
             * {"user", PDO_MIMER_DEFAULT_IDENT, 0 }, // PDO standard?
             */
    };

    num_data_src_opts = sizeof(data_src_opts) / sizeof(data_src_opt);

    MimerSession session = NULL;
    pdo_mimer_handle *handle = pecalloc(1, sizeof(pdo_mimer_handle), dbh->is_persistent);
    dbh->driver_data = handle;

    handle->last_error = 0;
    handle->session = session;
    handle->cursor_type = MIMER_FORWARD_ONLY;
    handle->trans_option = MIMER_TRANS_DEFAULT;

    /**
     * @brief Parsing the DSN
     *
     * This function, provided by php-src/ext/pdo/pdo.c, parses the user-provided DSN
     * during the construction of the PDO and copies each value (optval) to the respective key (optname).
     */
    php_pdo_parse_data_source(dbh->data_source, dbh->data_source_len, data_src_opts, num_data_src_opts);

    /* TODO: add compatability for MimerBeginSession() and MimerBeginSessionC() */
    int32_t return_code = MimerBeginSession8(data_src_opts[dbname_opt].optval, dbh->username, dbh->password, &session);

    if (!MIMER_SUCCEEDED(return_code) || session == NULL) {
        handle->last_error = return_code;
        pdo_mimer_error(dbh);
        goto cleanup;
    }

    php_printf("Connected to Mimer SQL (db=%s, user=%s, pwd=%s)\n", data_src_opts[dbname_opt].optval, dbh->username, dbh->password);

    /* free up memory no longer needed */
    for (int i = 0; i < num_data_src_opts; i++) {
        if (data_src_opts[i].freeme) {  // check if each option is persistent
            efree(data_src_opts[i].optval);
        }
    }

    cleanup:
    dbh->methods = &mimer_methods;
    if (!MIMER_SUCCEEDED(return_code)) {
        mimer_handle_closer(dbh);
    }

    return MIMER_SUCCEEDED(return_code);
}


/**
 * @brief Registers "mimer" as an available database in PDO's data source name
 */
const pdo_driver_t pdo_mimer_driver = {
        PDO_DRIVER_HEADER(mimer),
        pdo_mimer_handle_factory
};
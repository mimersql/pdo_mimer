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
   | Authors: Alexander Hedberg <alexander.hedberg@mimer.com>             |
   |          Ludwig von Feilitzen <ludwig.vonfeilitzen@mimer.com>        |
   +----------------------------------------------------------------------+
*/

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_mimer.h"
#include "php_pdo_mimer_int.h"
#include "pdo_mimer_error.h"
#include "mimer_stmt_arginfo.h"

/**
 * @brief Mimer PDO specific implementation of PHP streams' write. 
 * 
 * @return -1 at failure, otherwise number of bytes written 
 * @remark TODO: See if this can be adapted for use with LOB input
 * @see https://github.com/php/php-src/blob/master/docs/streams.md
 */
static ssize_t mimer_lob_write(php_stream *stream, const char *buf, size_t count)
{
    return 0;
}

/**
 * @brief Mimer PDO specific implementation of PHP streams' read. 
 * 
 * @return -1 at failure, otherwise number of bytes read 
 * @see https://github.com/php/php-src/blob/master/docs/streams.md
 */
static ssize_t mimer_lob_read(php_stream *stream, char *buf, size_t count) {
    ssize_t rc;
    pdo_mimer_lob_stream_data *stream_data = (pdo_mimer_lob_stream_data*)stream->abstract;

    if (stream->eof)
        return 0;

    if(MimerIsBlob(stream_data->lob_type))
        rc = MimerGetBlobData(&stream_data->lob_handle, buf, count);
    else if(MimerIsClob(stream_data->lob_type) || MimerIsNclob(stream_data->lob_type))
        rc = MimerGetNclobData8(&stream_data->lob_handle, buf, count);
    else         
        return -1;
    
    if (MIMER_SUCCEEDED(rc)) {
        stream->eof = rc <= count;
        return stream->eof ? rc : (ssize_t) count;
    } else 
        return -1;
}

/**
 * @brief Mimer PDO specific implementation of PHP streams' close. 
 * 
 * @remark TODO: Find out if return value matters 
 * @see https://github.com/php/php-src/blob/master/docs/streams.md
 */
static int mimer_lob_close(php_stream *stream, int close_handle) {
    pdo_mimer_lob_stream_data *self = (pdo_mimer_lob_stream_data*)stream->abstract;
    if (close_handle){
        efree(self);
    }
	return 0;
}

/**
 * @brief Mimer PDO specific implementation of PHP streams' flush. 
 * 
 * @remark Currently does nothing but is mandatory to define for 
 *          own php_stream implementations.  
 * @see https://github.com/php/php-src/blob/master/docs/streams.md
 */
static int mimer_lob_flush(php_stream *stream)
{
	return 0;
}

/**
 * @brief Collection of function pointers defining the Mimer PDO
 * specific PHP streams implementation.  
 * 
 * @see https://github.com/php/php-src/blob/master/docs/streams.md
 */
const php_stream_ops pdo_mimer_lob_stream_ops = {
	mimer_lob_write,
	mimer_lob_read,
	mimer_lob_close,
	mimer_lob_flush,
	"pdo_mimer lob stream",
	NULL,
	NULL,
	NULL,
	NULL
};
 
 /**
  * @brief Creates a PHP stream which represents a connection to a LOB in DB.
  * 
  * @param stmt [in] A pointer to the PDOStatement handle object.
  * @param colno [in] Index of LOB column in resultset (one-indexed)
  * @param lob_type [in] MIMER_BLOB | MIMER_CLOB | MIMER_NCLOB
  * @return Pointer to PHP stream if successful, null otherwise
  */
php_stream *pdo_mimer_create_lob_stream(pdo_stmt_t *stmt, int colno, int32_t lob_type) {
    php_stream *stream;
    size_t lob_size;
    pdo_mimer_lob_stream_data *stream_data = emalloc(sizeof(pdo_mimer_lob_stream_data));
    stream_data->lob_type = lob_type;

    if (!MIMER_SUCCEEDED(MimerGetLob(MIMER_STMT, colno, &lob_size, &stream_data->lob_handle))) {
        pdo_mimer_stmt_error();
        goto cleanup;
    }

    if ((stream = php_stream_alloc(&pdo_mimer_lob_stream_ops, stream_data, 0, "r+b")) == NULL) {
        goto cleanup;
    }

    return stream;

    cleanup:
    efree(stream_data);
    return NULL;
}


/**
 * @brief Safely insert @p code and @p msg into @p error_info while freeing any previously allocated memory.
 * @param error_info [out] The error info to insert @p code and @msg into.
 * @param code [in] A MimerErrorCode belonging to the error.
 * @param msg [in] The error message to display when needed.
 */
static void pdo_mimer_insert_error_info(MimerErrorInfo *error_info, MimerErrorCode code, const char *msg, const SQLState sqlstate) {
    if (error_info->msg) {
        pefree(error_info->msg, error_info->persistent);
    }

    error_info->code = code;
    error_info->msg = pestrdup(msg, error_info->persistent);
    strcpy(error_info->sqlstate, sqlstate);
}


/**
 * @brief Get error message and error code from @p handle.
 * @param error_info [out] The error info to store the error code and message in.
 * @param handle [in] Either a MimerSession or MimerStatement.
 * @return MimerReturnCode
 */
static int pdo_mimer_get_error_info(MimerErrorInfo *error_info, MimerHandle handle) {
    MimerReturnCode return_code;
    MimerErrorCode error_code;
    char *tmp_buf;
    size_t str_len;

    str_len = (return_code = MimerGetError8(handle, &error_code, NULL, 0)) + 1; /* +1 for null terminator */
    if (!MIMER_SUCCEEDED(return_code))
        return return_code; /* propagate error */

    tmp_buf = emalloc(str_len);
    if (!MIMER_SUCCEEDED(return_code = MimerGetError8(handle, &error_code, tmp_buf, str_len)))
        return return_code; /* propagate error */

    pdo_mimer_insert_error_info(error_info, error_code, tmp_buf, MimerGetSQLState(error_code));
    efree(tmp_buf);
    return MIMER_SUCCESS;
}


/**
 * @brief Error function to create custom error code and propagate @p sqlstate to @p pdo_error_code.
 * @param error_info [in|out] @p error_info should be populated with error code beforehand.
 * @param sqlstate [in] The SQLState code closest to the given error.
 * @param pdo_error_code [out] The <code>pdo_error_type</code> from either <code>pdo_dbh_t</code> or
 * <code>pdo_stmt_t</code>.
 * @param format [in] The string to be formatted with the remaining arguments for the error message.
 * @param ... [in] Extra arguments to pass to the error message to be formatted.
 */
void _pdo_mimer_custom_error(MimerErrorInfo *error_info, const char sqlstate[6], pdo_error_type pdo_error_code, const char * format, ...) {
    va_list ap;
    char *tmp_buf;

    va_start(ap, format);
    vspprintf(&tmp_buf, 0, format, ap); /* format string with the help of vspprintf using va_list */
    va_end(ap);

    pdo_mimer_insert_error_info(error_info, error_info->code, tmp_buf, sqlstate);
    strcpy(pdo_error_code, sqlstate);
}


/**
 * @brief Error handler function to be used when an API error occurs. Gets the error code and error message.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @param stmt [in] A pointer to the PDOStatement handle object.
 * @param FILE [in] The name of the file where the error occurred, to be used with <code>__FILE__</code>.
 * @param LINE [in] The line number of where the error occurred, to be used with <code>__LINE__</code>.
 * @remark  If a @p dbh error, pass <code>NULL</code> to @p stmt. If a @p stmt error, pass @p stmt<code>->dbh</code>
 * to @p dbh and @p stmt to @p stmt.
 */
void pdo_mimer_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *FILE, const int LINE) {
    MimerReturnCode return_code;

    /* grab data needed depending on if error comes from statement or session */
    MimerHandle handle = stmt ? MIMER_HANDLE(stmt) : MIMER_HANDLE(dbh);
    MimerErrorInfo *error_info = stmt ? PDO_MIMER_STMT_ERROR : PDO_MIMER_DBH_ERROR;

    if (!MIMER_SUCCEEDED(return_code = pdo_mimer_get_error_info(error_info, handle))) {
        /* unable to get error info for some reason. throw an exception */
        error_info->code = return_code;
        _pdo_mimer_custom_error(error_info, MimerGetSQLState(return_code), stmt ? stmt->error_code : dbh->error_code,
                                "%s:%d Error occurred while fetching error from %s.",
                                FILE, LINE, stmt ? "statement" : "session");
        if(stmt)
            mimer_throw_except(stmt);
        else
            mimer_throw_except(dbh);
    }
}

/**
 * @brief PDO method to end a Mimer SQL session.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @remark Frees any allocated memory.
 */
static void mimer_handle_closer(pdo_dbh_t *dbh) {
    if (!MIMER_SUCCEEDED(MimerEndSession(&MIMER_SESSION))) {
        pdo_mimer_dbh_error();
        mimer_throw_except(dbh);
    }

    if (PDO_MIMER_DBH_ERROR->msg != NULL)
        pefree(PDO_MIMER_DBH_ERROR->msg, PDO_MIMER_DBH_ERROR->persistent);

    pefree(dbh->driver_data, dbh->is_persistent);
    dbh->driver_data = NULL;
}


/**
 * @brief PDO method to prepare a SQL query with possible positional or named placeholders.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @param sql [in] The SQL query to be executed.
 * @param stmt [in|out] A pointer to the PDOStatement handle object.
 * @param driver_options [in] User-specified options to Mimer SQL.
 * @return true upon success
 * @return false upon failure
 */
static bool mimer_handle_preparer(pdo_dbh_t *dbh, zend_string *sql, pdo_stmt_t *stmt, zval *driver_options) {
    MimerReturnCode return_code;
    MimerStatement mimer_stmt;
    int32_t cursor_type;

    /* if no option given, assign PDO_CURSOR_FWDONLY as default */
    cursor_type = pdo_attr_lval(driver_options, PDO_ATTR_CURSOR, PDO_CURSOR_FWDONLY) ==
            PDO_CURSOR_SCROLL ? MIMER_SCROLLABLE : MIMER_FORWARD_ONLY;

    stmt->supports_placeholders = PDO_PLACEHOLDER_POSITIONAL;
    struct { bool freeme; zend_string *stmt; } active_query = {};
    switch (pdo_parse_params(stmt, sql, &active_query.stmt)) {
        case -1: // unable to parse
            strcpy(dbh->error_code, stmt->error_code);
            goto cleanup;

        case 0: /* no parsing needed */
            active_query.stmt = sql;
            break;

        default: /* parsing needed, newly parsed sql statement now in active_query.stmt */
            active_query.freeme = 1;
            break;
    }

    if (!MIMER_SUCCEEDED(return_code = MimerBeginStatement8(MIMER_SESSION, ZSTR_VAL(active_query.stmt), cursor_type, &mimer_stmt))) {
        pdo_mimer_dbh_error();
        goto cleanup;
    }

    stmt->driver_data = NEW_PDO_MIMER_STMT(mimer_stmt, cursor_type); /* allocate memory for new handle */
    stmt->methods = &pdo_mimer_stmt_methods;

    cleanup:
    if (active_query.freeme) /* if sql query was parsed, it's up to us to free it */
        zend_string_release(active_query.stmt);

    return MIMER_SUCCEEDED(return_code);
}


/**
 * @brief This function will be called by PDO to execute a raw SQL statement.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @param sql [in] A zend_string containing the SQL statement to be prepared.
 * @return This function returns the number of rows affected or -1 upon failure.
 * @remark Mimer SQL C API's <code>MimerExecuteStatement[8|C]</code> doesn't support
 * returning number of affected rows (yet?).
 */
static zend_long mimer_handle_doer(pdo_dbh_t *dbh, const zend_string *sql) {
    MimerReturnCode return_code;
    int num_affected_rows;

    if (!MIMER_SUCCEEDED(return_code = MimerExecuteStatement8(MIMER_SESSION, ZSTR_VAL(sql)))) {
        pdo_mimer_dbh_error();
        return -1;
    } else {
        /* doesn't actually return num affected rows.
         * this is just for possible future support, otherwise it will just return 0. */
        num_affected_rows = return_code;
    }

    return num_affected_rows;
}


/**
 * @brief PDO method to start a transaction.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @return true if transaction was started
 * @return false if unable to start transaction
 */
static bool mimer_handle_begin(pdo_dbh_t *dbh) {
    if (!MIMER_SUCCEEDED(MimerBeginTransaction(MIMER_SESSION, PDO_MIMER_TRANS_TYPE))) {
        pdo_mimer_dbh_error();
        return false;
    }

    PDO_MIMER_IN_TRANSACTION = true;
    return true;
}


/**
 * @brief This function will be called by PDO to commit a transaction.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @return true if transaction was committed
 * @return false if unable to commit transaction
 */
static bool mimer_handle_commit(pdo_dbh_t *dbh) {
    if (!MIMER_SUCCEEDED(MimerEndTransaction(MIMER_SESSION, MIMER_COMMIT))) {
        pdo_mimer_dbh_error();
        return false;
    }

    PDO_MIMER_IN_TRANSACTION = false;
    return true;
}


/**
 * @brief This function will be called by PDO to rollback a transaction.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @return true if transaction was rollbacked
 * @return false if unable to rollback transaction
 */
static bool mimer_handle_rollback(pdo_dbh_t *dbh) {
    if (!MIMER_SUCCEEDED(MimerEndTransaction(MIMER_SESSION, MIMER_ROLLBACK))) {
        pdo_mimer_dbh_error();
        return false;
    }

    PDO_MIMER_IN_TRANSACTION = false;
    return true;
}


/**
 * @brief PDO method to set driver attributes.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @param attribute [in] The PDO or Mimer SQL attribute to set.
 * @param value [in] The value to set for the given attribute.
 * @return true on successfully able to set attribute value
 * @return false if unable to set value or if attribute not supported
 */
static bool pdo_mimer_set_attribute(pdo_dbh_t *dbh, zend_long attribute, zval *value) {
    switch (attribute) {
        /* TODO: add necessary attributes */
        /* TODO: add charset attribute support */

        /* custom driver attributes */
        case MIMER_ATTR_TRANS_OPTION: {
            long trans_readonly;
            if (!pdo_get_long_param(&trans_readonly, value)) /* user didnt enter a number type */
                return false;

            PDO_MIMER_TRANS_READONLY = trans_readonly; /* true => READWRITE, false => READONLY */
            return true;
        }

        default:
            return false;
    }
}


/**
 * @brief PDO method to retrieve the latest error.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @param stmt [in] A pointer to the PDOStatement handle object.
 * @param info [in] The information array to add Mimer SQL error information to.
 */
static void pdo_mimer_fetch_err(pdo_dbh_t *dbh, pdo_stmt_t *stmt, zval *info) {
    MimerErrorInfo *error_info = stmt ? PDO_MIMER_STMT_ERROR : PDO_MIMER_DBH_ERROR;

    if (!error_info->msg) {
        add_next_index_null(info);
        return;
    }

    /* add information to PDO::errorInfo or PDOStatement::errorInfo */
    /* the first index should already have been added by PDO, which is the SQLState code from either
     * dbh->error_code or stmt->error_code, which has been added previously in pdo_mimer_error or pdo_mimer_custom_error */
    add_next_index_long(info, error_info->code);
    add_next_index_string(info, error_info->msg);
}


/**
 * @brief A function to check if the user is still connected to a MimerSession.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @return true if connection is alive
 * @return false if not connected
 */
static zend_result pdo_mimer_check_liveness(pdo_dbh_t *dbh) {
    if (!MIMER_SUCCEEDED(MimerPing(MIMER_SESSION))) {
        pdo_mimer_dbh_error();
        return FAILURE;
    }
    return SUCCESS;
}


/**
 * @brief Get driver attributes.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @param attribute [in] The PDO or Mimer SQL attribute to get the value of.
 * @param return_value [out] The value of the attribute.
 * @return 1 upon success
 * @return 0 upon failure to get attribute value or if attribute not supported by driver
 * @remark @p return_value holds the gotten attribute's value
 */
static int pdo_mimer_get_attribute(pdo_dbh_t *dbh, zend_long attribute, zval *return_value) {
    switch (attribute) {
        /* TODO: add autocommit functionality and/or more */

        case PDO_ATTR_CLIENT_VERSION:
            ZVAL_STRING(return_value, MimerAPIVersion());
            break;

        case PDO_ATTR_DRIVER_NAME:
            ZVAL_STRING(return_value, "mimer");
            break;

        case PDO_ATTR_CONNECTION_STATUS: {
            /* TODO: find out how to do this */
            /* MySQLs status function https://dev.mysql.com/doc/refman/8.0/en/show-status.html */
            if (MIMER_SESSION == NULL || pdo_mimer_check_liveness(dbh) == FAILURE) {
                ZVAL_STRING(return_value, "Disconnected");
                break;
            }

            ZVAL_STRING(return_value, "Connected");
            break;
        }

        /* custom driver attributes */
        case MIMER_ATTR_TRANS_OPTION:
            ZVAL_STRING(return_value, PDO_MIMER_TRANS_READONLY ? "Read-only" : "Read and write");
            break;

        /* TODO: find more attributes for Mimer */

        default:
            return 0;
    }

    return 1;
}


/**
 * @brief Function entry for PDO to get extra driver/statement methods specific to PDO Mimer.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @param kind [in] The enum value to determine which set of methods should be used to extend PDO or PDOStatement.
 * @return An array of methods or NULL if no methods available.
 */
static inline const zend_function_entry *pdo_mimer_get_driver_methods(pdo_dbh_t *dbh, int kind) {
    return kind == PDO_DBH_DRIVER_METHOD_KIND_STMT ? class_PDOStatement_MimerSQL_Ext_methods : NULL;
}


/**
 * @brief Check if the connection currently has a started transaction.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @return true if a transaction has been started on this session
 * @false if no transaction has been started
 */
static inline bool pdo_mimer_in_transaction(pdo_dbh_t *dbh) {
    return PDO_MIMER_IN_TRANSACTION;
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
        pdo_mimer_get_driver_methods,   /* get driver method */
        NULL,   /* persistent connection shutdown method */
        pdo_mimer_in_transaction,    /* in transaction method */
        NULL    /* get gc method */
};


/**
 * @brief This method is called when constructing the PDO object, for use with Mimer SQL.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @param driver_options [in] Attributes and/or options that can be set upon construction of the PDO object. This is
 * usually where the user-defined options define the connection type to the database (eg. connection pooling,
 * autocommit, etc.)
 * @example @code $PDO = new PDO("mimer:dbname=$dbname", $user, $pass, $attr_array); @endcode
 */
static int pdo_mimer_handle_factory(pdo_dbh_t *dbh, zval *driver_options) {
    /* Leave this guard here until we're ready to test persistent connections */
    if(dbh->is_persistent){
        strcpy(dbh->error_code, SQLSTATE_FEATURE_NOT_SUPPORTED);
        pdo_throw_exception(PDO_MIMER_FEATURE_NOT_IMPLEMENTED, "Persistent connections not yet supported", &dbh->error_code);
    }

    
    enum { dbname, user, password, num_opts }; /* for ease of use when accessing opts */
    struct pdo_data_src_parser opts[] = { /* the data source name options to provide to the user */
            /* if the user does not give database name, NULL will trigger default database connection */
            { "dbname", NULL, 0 },
            { "user", "", 0 }, /* MimerBeginSession crashes on NULL user */
            { "password", NULL, 0 }
    };

    /* parse the data source name, getting user provided values, swapping out the default values for the user provided ones */
    /* when the user provides values to the matching provided options, the freeme field is set to one requiring freeing
     * of the optval field */
    php_pdo_parse_data_source(dbh->data_source, dbh->data_source_len, opts, num_opts);
    if (!dbh->username && opts[user].optval) /* username arg in PDO constructor takes precedence */
        dbh->username = pestrdup(opts[user].optval, dbh->is_persistent);
    if (!dbh->password && opts[password].optval) /* password arg in PDO constructor takes precedence */
        dbh->password = pestrdup(opts[password].optval, dbh->is_persistent);

    dbh->driver_data = NEW_PDO_MIMER_DBH(dbh->is_persistent); /* needs to be created here to access MIMER_SESSION below */
    if (!MIMER_SUCCEEDED(MimerBeginSession8(opts[dbname].optval, dbh->username, dbh->password, &MIMER_SESSION))) {
        pdo_mimer_dbh_error();
        mimer_throw_except(dbh);
        mimer_handle_closer(dbh); /* call handle closer to free driver data */
        goto cleanup;
    }

    /* provide the PDO handler with information about our driver */
    dbh->methods = &mimer_methods; /* an array of methods that PDO can call provided by our driver */
    dbh->skip_param_evt = 1 << PDO_PARAM_EVT_NORMALIZE
            | 1 << PDO_PARAM_EVT_FREE
            | 1 << PDO_PARAM_EVT_FETCH_PRE
            | 1 << PDO_PARAM_EVT_FETCH_POST;


    cleanup: /* free up options given by user in dsn */
    for (int i = 0; i < num_opts; i++) {
        if (opts[i].freeme) {
            efree(opts[i].optval);
        }
    }

    return dbh->driver_data != NULL;
}

/* register the driver in PDO */
const pdo_driver_t pdo_mimer_driver = {
        PDO_DRIVER_HEADER(mimer),
        pdo_mimer_handle_factory
};
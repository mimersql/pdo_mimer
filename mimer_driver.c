/*
   +--------------------------------------------------------------------------------+
   | MIT License                                                                    |
   +--------------------------------------------------------------------------------+
   | Copyright (c) 2023 Mimer Information Technology AB                             |
   +--------------------------------------------------------------------------------+
   | Permission is hereby granted, free of charge, to any person obtaining a copy   |
   | of this software and associated documentation files (the "Software"), to deal  |
   | in the Software without restriction, including without limitation the rights   |
   | to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      |
   | copies of the Software, and to permit persons to whom the Software is          |
   | furnished to do so, subject to the following conditions:                       |
   |                                                                                |
   | The above copyright notice and this permission notice shall be included in all |
   | copies or substantial portions of the Software.                                |
   |                                                                                |
   | THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     |
   | IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       |
   | FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    |
   | AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         |
   | LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  |
   | OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  |
   | SOFTWARE.                                                                      |
   +--------------------------------------------------------------------------------+
   | Authors: Alexander Hedberg <alexander.hedberg@mimer.com>                       |
   |          Ludwig von Feilitzen <ludwig.vonfeilitzen@mimer.com>                  |
   +--------------------------------------------------------------------------------+
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
        return SUCCESS;

    if(MimerIsBlob(stream_data->lob_type))
        rc = MimerGetBlobData(&stream_data->lob_handle, buf, count);
    else if(MimerIsClob(stream_data->lob_type) || MimerIsNclob(stream_data->lob_type))
        rc = MimerGetNclobData8(&stream_data->lob_handle, buf, count);
    else         
        return FAILURE;
    
    if (MIMER_SUCCEEDED(rc)) {
        stream->eof = rc <= count;
        return stream->eof ? rc : (ssize_t) count;
    } else 
        return FAILURE;
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
	return SUCCESS;
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
	return SUCCESS;
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
	pdo_mimer_stmt *mimer_stmt = stmt->driver_data;
    php_stream *stream;
    size_t lob_size;

	 pdo_mimer_lob_stream_data *stream_data = emalloc(sizeof(pdo_mimer_lob_stream_data));
	 stream_data->lob_type = lob_type;

    if (!MIMER_SUCCEEDED(MimerGetLob(mimer_stmt->stmt, colno, &lob_size, &stream_data->lob_handle))) {
        pdo_mimer_stmt_error();
        goto cleanup;
    }

    if ((stream = php_stream_alloc(&pdo_mimer_lob_stream_ops, stream_data, 0, "r+b")) == NULL)
        goto cleanup;

    return stream;

    cleanup:
    efree(stream_data);
    return NULL;
}


const char* pdo_mimer_get_sqlstate(MimerErrorCode error_code) {
	switch (error_code) {
		case MIMER_SUCCESS:
			return SQLSTATE_SUCCESSFUL_COMPLETION;
		case 90:
			return SQLSTATE_CONNECTION_FAILURE;
		default:
			return SQLSTATE_GENERAL_ERROR;
	}
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
	pdo_mimer_dbh *mimer_dbh = dbh->driver_data;
	pdo_mimer_stmt *mimer_stmt = stmt ? stmt->driver_data : NULL;
	MimerHandle mimer_handle = mimer_stmt ? (MimerHandle) mimer_stmt->stmt : (MimerHandle) mimer_dbh->session;

	if (mimer_dbh->error.msg != NULL) {
		pefree(mimer_dbh->error.msg, dbh->is_persistent);
		mimer_dbh->error.msg = NULL;
	}

	MimerReturnCode return_code = MimerGetError8(mimer_handle, &mimer_dbh->error.code, NULL, 0);

	char *err_msg;
	if (!MIMER_SUCCEEDED(return_code)) {
		spprintf(&err_msg, 0, "Unable to retrieve error information: %s:%d (%d)\n", FILE, LINE, return_code);
		mimer_dbh->error.code = return_code;
	} else {
		size_t str_len = return_code + 1;
		err_msg = emalloc(str_len);
		MimerGetError8(mimer_handle, &mimer_dbh->error.code, err_msg, str_len);
	}

	mimer_dbh->error.msg = pestrdup(err_msg, dbh->is_persistent);
	strcpy(mimer_dbh->error.sqlstate, pdo_mimer_get_sqlstate(mimer_dbh->error.code));
	efree(err_msg);

	strcpy(stmt ? stmt->error_code : dbh->error_code, pdo_mimer_get_sqlstate(mimer_dbh->error.code));

	if (!dbh->methods)
		pdo_throw_exception(mimer_dbh->error.code, mimer_dbh->error.msg, &mimer_dbh->error.sqlstate);
}

/**
 * @brief PDO method to end a Mimer SQL session.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @remark Frees any allocated memory.
 */
static void mimer_handle_closer(pdo_dbh_t *dbh) {
	pdo_mimer_dbh *mimer_dbh = dbh->driver_data;

    if (!MIMER_SUCCEEDED(MimerEndSession(&mimer_dbh->session))) {
        pdo_mimer_dbh_error();
//        mimer_throw_except(dbh);
    }

    if (mimer_dbh->error.msg != NULL)
        pefree(mimer_dbh->error.msg, dbh->is_persistent);

    pefree(dbh->driver_data, dbh->is_persistent);
    dbh->driver_data = NULL;
}


static char *pdo_mimer_rewrite_sql(pdo_stmt_t *stmt, zend_string *sql) {
	zend_string* sql_rewritten = NULL;
	char *sql_str = NULL;

	stmt->supports_placeholders = PDO_PLACEHOLDER_POSITIONAL;
	switch (pdo_parse_params(stmt, sql, &sql_rewritten)) {
		case FAILURE:
			return NULL;

		case SUCCESS:
			return estrdup(ZSTR_VAL(sql));

		default:
			sql_str = estrdup(ZSTR_VAL(sql_rewritten));
			zend_string_release(sql_rewritten);
			return sql_str;
	}
}


static pdo_mimer_stmt *pdo_mimer_create_stmt(pdo_dbh_t *dbh, MimerStatement statement, int32_t cursor_type) {
	pdo_mimer_stmt *mimer_stmt = emalloc(sizeof(pdo_mimer_stmt));
	*mimer_stmt = (pdo_mimer_stmt) {
		.dbh  = dbh->driver_data,
		.stmt = statement,
		.cursor     = {
			.is_open       = false,
			.is_scrollable = cursor_type == MIMER_SCROLLABLE,
		},
	};

	return mimer_stmt;
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
    MimerReturnCode return_code = MIMER_SUCCESS;
	pdo_mimer_dbh *mimer_dbh = dbh->driver_data;
    MimerStatement statement = MIMERNULLHANDLE;

	char *sql_str = pdo_mimer_rewrite_sql(stmt, sql);
	if (sql_str == NULL) {
		strcpy(dbh->error_code, stmt->error_code);
		return false;
	}

	/* if no option given, assign PDO_CURSOR_FWDONLY as default */
	int32_t cursor_type = pdo_attr_lval(driver_options, PDO_ATTR_CURSOR, PDO_CURSOR_FWDONLY) ==
		PDO_CURSOR_SCROLL ? MIMER_SCROLLABLE : MIMER_FORWARD_ONLY;

	if (!MIMER_SUCCEEDED(return_code = MimerBeginStatement8(mimer_dbh->session, sql_str, cursor_type, &statement))) {
		pdo_mimer_dbh_error();
		goto cleanup;
	}

    stmt->driver_data = pdo_mimer_create_stmt(dbh, statement, cursor_type);
    stmt->methods = &pdo_mimer_stmt_methods;

    cleanup:
    efree(sql_str);

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
	pdo_mimer_dbh *mimer_dbh = dbh->driver_data;

    if (!MIMER_SUCCEEDED(MimerExecuteStatement8(mimer_dbh->session, ZSTR_VAL(sql)))) {
        pdo_mimer_dbh_error();
        return FAILURE;
    }

	return SUCCESS;
}


/**
 * @brief Set quotes around identifiers where needed
 * @param dbh [in] A pointer to the PDO database handle object.
 * @param unquoted A string of the SQL statement before being quoted
 * @param param_type [in] The parameter's data type.
 * @return The quoted SQL statement
 * @todo Add LOB support
 */
static zend_string* mimer_handle_quoter(pdo_dbh_t *dbh, const zend_string *unquoted, enum pdo_param_type param_type) {
	if (param_type == PDO_PARAM_LOB)
		return NULL;

	size_t quoted_len, unquoted_len;
	quoted_len = unquoted_len = ZSTR_LEN(unquoted);
	const char escape = '\'';
	zend_string* quoted;
	char* quoted_str;

	const char* unquoted_str = ZSTR_VAL(unquoted);
	for (int i = 0; i < unquoted_len; i++) { // count number of double quotation marks that need escaping
		if (unquoted_str[i] == escape)
			quoted_len++;
	}

	quoted_len += 2; // leave room for escaping char in front and back
	quoted_str = ecalloc(quoted_len, sizeof(char));

	// prepend and append double quotation marks and null-terminate the string
	quoted_str[0] = '\'';
	quoted_str[quoted_len - 1] = '\'';

	for (int i = 0, j = 1; i < unquoted_len; i++, j++) { // add escaping char to each double quotation mark
		if (unquoted_str[i] == escape) // escape double quotation mark by using another quotation mark and increment j
			quoted_str[j++] = escape;
		quoted_str[j] = unquoted_str[i];
	}

	quoted = zend_string_init(quoted_str, quoted_len, false);
	efree(quoted_str);
	return quoted;
}


static bool mimer_handle_transaction(pdo_dbh_t *dbh, bool start_transaction, int32_t transaction_op) {
	pdo_mimer_dbh *mimer_dbh = dbh->driver_data;
	MimerReturnCode return_code = MIMER_SUCCESS;

	if (start_transaction) {
		transaction_op = mimer_dbh->transaction.is_read_only ? MIMER_TRANS_READONLY : MIMER_TRANS_READWRITE;
		return_code = MimerBeginTransaction(mimer_dbh->session, transaction_op);
	} else {
		return_code = MimerEndTransaction(mimer_dbh->session, transaction_op);
	}

	if (!MIMER_SUCCEEDED(return_code)) {
		pdo_mimer_dbh_error();
		return false;
	}

	mimer_dbh->transaction.is_in_transaction = start_transaction;
	return true;
}


/**
 * @brief PDO method to start a transaction.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @return true if transaction was started
 * @return false if unable to start transaction
 */
static bool mimer_handle_begin(pdo_dbh_t *dbh) {
    return mimer_handle_transaction(dbh, true, MIMER_TRANS_DEFAULT);
}


/**
 * @brief This function will be called by PDO to commit a transaction.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @return true if transaction was committed
 * @return false if unable to commit transaction
 */
static bool mimer_handle_commit(pdo_dbh_t *dbh) {
    return mimer_handle_transaction(dbh, false, MIMER_COMMIT);
}


/**
 * @brief This function will be called by PDO to rollback a transaction.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @return true if transaction was rollbacked
 * @return false if unable to rollback transaction
 */
static bool mimer_handle_rollback(pdo_dbh_t *dbh) {
	return mimer_handle_transaction(dbh, false, MIMER_ROLLBACK);
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
	pdo_mimer_dbh *mimer_dbh = dbh->driver_data;

    switch (attribute) {
        /* custom driver attributes */
        case MIMER_ATTR_TRANS_OPTION: {
            long trans_readonly;
            if (!pdo_get_long_param(&trans_readonly, value)) /* user didnt enter a number type */
                return false;

            mimer_dbh->transaction.is_read_only = trans_readonly == MIMER_TRANS_READONLY;
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
    pdo_mimer_dbh *mimer_dbh = dbh->driver_data;

    if (mimer_dbh->error.msg == NULL) {
        add_next_index_null(info);
        return;
    }

    add_next_index_long(info, mimer_dbh->error.code);
    add_next_index_string(info, mimer_dbh->error.msg);
}


/**
 * @brief A function to check if the user is still connected to a MimerSession.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @return true if connection is alive
 * @return false if not connected
 */
static zend_result pdo_mimer_check_liveness(pdo_dbh_t *dbh) {
	pdo_mimer_dbh *mimer_dbh = dbh->driver_data;

    if (!MIMER_SUCCEEDED(MimerPing(mimer_dbh->session))) {
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
	pdo_mimer_dbh *mimer_dbh = dbh->driver_data;

    switch (attribute) {
        case PDO_ATTR_CLIENT_VERSION:
            ZVAL_STRING(return_value, MimerAPIVersion());
            break;

        case PDO_ATTR_DRIVER_NAME:
            ZVAL_STRING(return_value, "mimer");
            break;

        case PDO_ATTR_CONNECTION_STATUS: {
            if (pdo_mimer_check_liveness(dbh) == FAILURE) {
                ZVAL_STRING(return_value, "Disconnected");
                break;
            }

            ZVAL_STRING(return_value, "Connected");
            break;
        }

        /* custom driver attributes */
        case MIMER_ATTR_TRANS_OPTION:
            ZVAL_STRING(return_value, mimer_dbh->transaction.is_read_only ? "Read-only" : "Read and write");
            break;

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
static const zend_function_entry *pdo_mimer_get_driver_methods(pdo_dbh_t *dbh, int kind) {
    return kind == PDO_DBH_DRIVER_METHOD_KIND_STMT ? class_PDOStatement_MimerSQL_Ext_methods : NULL;
}


/**
 * @brief Check if the connection currently has a started transaction.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @return true if a transaction has been started on this session
 * @false if no transaction has been started
 */
static bool pdo_mimer_in_transaction(pdo_dbh_t *dbh) {
    pdo_mimer_dbh *mimer_dbh = dbh->driver_data;
	return mimer_dbh->transaction.is_in_transaction;
}


/* declares the methods Mimer uses and give them to the PDO driver */
static const struct pdo_dbh_methods mimer_methods = { /* {{{ */
        mimer_handle_closer,   /* handle closer method */
        mimer_handle_preparer,   /* handle preparer method */
        mimer_handle_doer,   /* handle doer method */
        mimer_handle_quoter,   /* handle quoter method */
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


static bool pdo_mimer_create_session(pdo_dbh_t *dbh) {
	pdo_mimer_dbh *mimer_dbh = dbh->driver_data = pecalloc(1, sizeof(pdo_mimer_dbh), dbh->is_persistent);
	mimer_dbh->session = MIMERNULLHANDLE;

	enum opts_enum { db_name, username, password, num_opts };
	struct pdo_data_src_parser opts[] = {
		{ "dbname", NULL, 0 },
		{ "user",   "",   0 },
		{ "pass",   NULL, 0 },
	};

	php_pdo_parse_data_source(dbh->data_source, dbh->data_source_len, opts, num_opts);

	if (!dbh->username && opts[username].optval)
		dbh->username = pestrdup(opts[username].optval, dbh->is_persistent);

	if (!dbh->password && opts[password].optval)
		dbh->password = pestrdup(opts[password].optval, dbh->is_persistent);

	bool success = MIMER_SUCCEEDED(MimerBeginSession8(opts[db_name].optval, dbh->username, dbh->password, &mimer_dbh->session));

	for (int i = 0; i < num_opts; i++) {
		if (opts[i].freeme)
			efree(opts[i].optval);
	}

	return success;
}


/**
 * @brief This method is called when constructing the PDO object, for use with Mimer SQL.
 * @param dbh [in] A pointer to the PDO database handle object.
 * @param driver_options [in] Attributes and/or options that can be set upon construction of the PDO object. This is
 * usually where the user-defined options define the connection type to the database (eg. connection pooling,
 * autocommit, etc.)
 * @example @code $PDO = new PDO("mimer:dbname=$dbname", $user, $pass, $attr_array); @endcode
 */
static int pdo_mimer_handle_factory(pdo_dbh_t *dbh, zval *driver_options) {
	bool success = pdo_mimer_create_session(dbh);
	if (!success)
		pdo_mimer_dbh_error();

    /* provide the PDO handler with information about our driver */
	dbh->alloc_own_columns = true;
	dbh->max_escaped_char_length = 2;
    dbh->methods = &mimer_methods; /* an array of methods that PDO can call provided by our driver */
    dbh->skip_param_evt = 1 << PDO_PARAM_EVT_NORMALIZE
            | 1 << PDO_PARAM_EVT_FREE
            | 1 << PDO_PARAM_EVT_FETCH_PRE
            | 1 << PDO_PARAM_EVT_FETCH_POST;

    return success;
}

/* register the driver in PDO */
const pdo_driver_t pdo_mimer_driver = {
        PDO_DRIVER_HEADER(mimer),
        pdo_mimer_handle_factory
};
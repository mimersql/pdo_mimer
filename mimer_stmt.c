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
 * @remark Driver is responsible for setting the column_count field in stmt for result set statements
*  @see <a href="https://php-legacy-docs.zend.com/manual/php5/en/internals2.pdo.pdo-stmt-t">
 *          PDO Driver How-To: pdo_stmt_t definition
 *      </a>
 */
static int pdo_mimer_stmt_executer(pdo_stmt_t *stmt) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;

    if(MimerStatementHasResultSet(stmt_handle->statement)) {
        if (stmt->executed) {
            return_on_err_stmt(MimerCloseCursor(stmt_handle->statement), 0)
        }
        return_on_err_stmt(MimerOpenCursor(stmt_handle->statement), 0)

        int num_columns;
        return_on_err_stmt(num_columns = MimerColumnCount(stmt_handle->statement), 0)
        php_pdo_stmt_set_column_count(stmt, num_columns);
    } else {
        return_on_err_stmt(MimerExecute(stmt_handle->statement), 0)
    }

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
    int32_t fetch_op_mode = MIMER_NEXT;
    MimerError return_code;

    /* map PDO fetch orientation to MimerFetchScroll operation mode */
    switch (ori) {
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

    return_on_err_stmt(return_code = stmt_handle->cursor_type == MIMER_FORWARD_ONLY ? MimerFetch(*mimer_statement) :
            MimerFetchScroll(*mimer_statement, fetch_op_mode, (int32_t)offset), 0)

    return return_code != MIMER_NO_DATA;
}

/**
 * @brief This function will be called by PDO to query information about a particular column.
 * 
 * @param stmt Pointer to the statement structure initialized by pdo_mimer_handle_preparer.
 * @param colno The column number to be queried.
 * @return 1 for success or 0 in the event of failure.
 * @remark PDO uses 0-index for columns, MimerAPI starts at 1, needs offset.
 */
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

/**
 * @brief This function will be called by PDO to retrieve data from the specified column.
 * 
 * @param stmt Pointer to the statement structure initialized by mimer_handle_preparer.
 * @param colno The column number to be queried.
 * @param result Pointer to the retrieved data.
 * @param type Parameter data type.
 * @return 1 for success or 0 in the event of failure.
 * @remark PDO uses 0-index for columns, MimerAPI starts at 1, needs offset.
 */
static int pdo_mimer_stmt_get_col_data(pdo_stmt_t *stmt, int colno, zval *result, enum pdo_param_type *type) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    int mim_colno = colno + 1; 
    MimerError return_code;

    return_on_err_stmt(return_code = MimerColumnType(stmt_handle->statement, mim_colno), 0)
    
    if (MimerIsInt64(return_code)){
        int64_t res;
        return_on_err_stmt(MimerGetInt64(stmt_handle->statement, mim_colno, &res), 0)
        ZVAL_LONG(result, res);
    } else if (MimerIsInt32(return_code)) {
        int32_t res;
        return_on_err_stmt(MimerGetInt32(stmt_handle->statement, mim_colno, &res), 0)
        ZVAL_LONG(result, res);
    } else if (MimerIsString(return_code)){
        MimerGetStr(MimerGetString8, str_buf, return_code, stmt_handle->statement, mim_colno);
        return_on_err_stmt(return_code, 0)
        ZVAL_STRING(result, str_buf);
    } else if (MimerIsBlob(return_code)) {
        php_stream *stm;
        stm = pdo_mimer_create_lob_stream(stmt, mim_colno, MIMER_BLOB);
        php_stream_to_zval(stm, result);
    }

    else {
        mimer_throw_except(&stmt_handle->error_info, "Unknown column type", MIMER_FEATURE_NOT_IMPLEMENTED,
                          SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED, stmt->dbh->is_persistent, stmt->error_code)
    }
    
    return 1;
}

/**
 * @brief Gets number of right-most bytes containing only partial
 * multi-byte sequences.
 *
 * @param[in] buf Buffer with multi-byte encoded data.
 * @param[in] len Length of buffer in bytes.
 * @return Number of bytes at the end of buffer which do not
 * contain any valid multi-byte characters, or a negative
 * error code.
 *
 * @remark Does not validate all characters in buffer.
 * @remark What characters are valid is determined by mblen(), which in turn
 * depends on the LC_TYPE category in the current locale settings.
 * @todo The function does not detect the case in which there are invalid
 * (as in NOT partial) byte sequences smaller than MIMER_MAX_MB_LEN. Any errors
 * in the byte section smaller than that at the end of the buffer are assumed
 * to be partial characters.
 * @see https://www.oreilly.com/library/view/c-in-a/0596006977/re164.html
 */
static ssize_t get_mbpartial(const char *buf, size_t len) {
    ssize_t pos;
    ssize_t nbytes;

    /** Cannot handle stateful encodings. */
    if (mblen(NULL, 0)){
        return MIMER_PDO_ENC_STATEFUL;
    }

    /** If only last char is misformed, the furtest away from the buffer end
     * that the start of a full character can be is MAX_MB_LEN + (MAX_MB_LEN - 1) */
    pos = len - (2 * MIMER_MAX_MB_LEN - 2);

    /* find the start of a character */
    while ((nbytes = mblen(buf + pos, len - pos)) < 0){
        pos++;
        if (pos == len)
            /* No valid characters in given window, can't be due to partials */
            return MIMER_PDO_ENC_UNKNOWN;
    }

    /* pass through all valid chars */
    while (pos < len && (nbytes = mblen(buf + pos, len - pos)) > 0)
        pos += nbytes;

    if ((len - pos) < MIMER_MAX_MB_LEN){
        return len - pos;
    } else {
        return MIMER_PDO_ENC_UNKNOWN;
    }
}

/**
 * @brief Fills a buffer with data from a stream and returns
 * the number of bytes which make up valid multi-byte encoded characters.
 *
 * @param[in] stm A stream to read bytes from.
 * @param[in] buf A buffer in which to place the read stream data.
 * @param[in] len Length of buffer in bytes.
 * @return Number of bytes (<=len) which contain valid multibyte characters if
 * successful, 0 if nothing more to read, negative error code at failure.
 *
 * @remark Assumes all invalid character byte sequences are at the
 * end of the buffer.
 */
static ssize_t get_valid_stream_chunk(php_stream *stm, char *buf, size_t len){
    ssize_t nbytes_partial, nbytes_read;

    if ((nbytes_read = php_stream_read(stm, buf, len)) == 0)
        return 0;

    nbytes_partial = get_mbpartial(buf, nbytes_read);
    if (nbytes_partial < 0)
        return nbytes_partial;

    php_stream_seek(stm, -nbytes_partial, SEEK_CUR);
    return nbytes_read - nbytes_partial;
}

/**
 * @brief Counts the number of multi-byte characters in a string.
 *
 * @param[in] str String containing multi-byte characters.
 * @param[in] len Length of string in bytes.
 * @return Number of characters if successful, -1 when invalid
 * character was found.
 *
 * @remark Counts up until the given length or the first null termination
 * character, whichever comes first.
 * @remark Similar to mbslen, but using mbslen can be a portability issue.
 * @see https://www.ibm.com/docs/en/aix/7.2?topic=m-mbslen-subroutine
 */
static size_t get_mbchar_count(char *str, size_t len){
    size_t pos = 0;
    ssize_t nbytes = 0;
    size_t nchars = 0;

    while ((pos < len) && (nbytes = mblen(str + pos, len - pos)) > 0){
        pos += nbytes;
        nchars++;
    }

    return nbytes >= 0 ? nchars : nbytes;
}

/**
 * @brief Calculates the length of a stream of character data
 * in both bytes and characters.
 *
 * @param[in] stm Stream containing character data.
 * @param[out] tot_size Total number of bytes from current place
 * in stream until EOF.
 * @return Number of characters found in stream, or negative error code.
 *
 * @remark Reads the stream in chunks to avoid putting entire LOB in memory.
 * @todo Might not need to calculate total bytes for clobs/nclobs anymore
 */
static ssize_t pdo_mimer_cloblen(php_stream *stm, size_t *tot_size){
    char *buf;
    ssize_t nchars = 0;
    size_t nchars_tot = 0;
    size_t nbytes_tot = 0;
    ssize_t nbytes_valid = 0;

    buf = emalloc(MIMER_LOB_IN_CHUNK);

    while((nbytes_valid = get_valid_stream_chunk(stm, buf, MIMER_LOB_IN_CHUNK)) > 0) {
        if ((nchars = get_mbchar_count(buf, nbytes_valid)) < 0){
            /** There were encoding errors not at the end of buffer */
            efree(buf);
            return MIMER_PDO_ENC_UNKNOWN;
        }
        nchars_tot += nchars;
        nbytes_tot += nbytes_valid;
    }

    efree(buf);

    if (nbytes_valid < 0)
        return nbytes_valid;
    else {
        *tot_size = nbytes_tot;
        php_stream_rewind(stm);
        return nchars_tot;
    }
}

/**
 * @brief Gets the length of the LOB stream, in bytes for all LOB types
 * and in number of characters for CLOBs and NCLOBs.
 *
 * @param[in] stm Pointer to the PHP stream with the data.
 * @param[in] lob_type Mimer constant for one of BLOB/CLOB/NCLOB.
 * @param[out] tot_size Total size of stream in bytes.
 * @return Number of characters or bytes found in stream, depending on LOB type,
 * or negative error code.
 *
 * @todo: Might not need to calculate total bytes for CLOBS/NCLOBS anymore
 */
static ssize_t pdo_mimer_loblen(php_stream *stm, int32_t lob_type, size_t *tot_size){
    ssize_t nchars = 0;

    if (MimerIsBlob(lob_type)){
        php_stream_seek(stm, 0, SEEK_END);
        nchars = php_stream_tell(stm);
        *tot_size = nchars;
        php_stream_rewind(stm);
        return nchars;

    } else if (MimerIsClob(lob_type) || MimerIsNclob(lob_type)){
        return pdo_mimer_cloblen(stm, tot_size);

    } else {
        return MIMER_PDO_UNKNOWN_LOB_TYPE;
    }
}

/**
 * @brief Reads content of stream (in chunks) into BLOB in DB.
 *
 * @param[in] stm Stream to read data from.
 * @param[in] lob_handle Handle to BLOB, prepared by call to MimerSetLob.
 * @param[in] lob_size Total number of bytes in stream.
 * @return 0 on success, negative error code otherwise.
 */
static ssize_t pdo_mimer_set_blob_data(php_stream *stm, MimerLob *blob_handle, size_t blob_size){
    size_t chunk_size = MIMER_LOB_IN_CHUNK;
    size_t bytes_read = 0;
    size_t bytes_left = SIZE_MAX;
    MimerError rc;
    char *blob_buf = emalloc(MIMER_LOB_IN_CHUNK);

    do {
        /** can't read more data than what is left */
        bytes_left = blob_size - bytes_read;
        chunk_size = bytes_left < MIMER_LOB_IN_CHUNK ? bytes_left : MIMER_LOB_IN_CHUNK;
        bytes_read += php_stream_read(stm, blob_buf, chunk_size);
        handle_err(rc = MimerSetBlobData(blob_handle, blob_buf, chunk_size), efree(blob_buf), return rc)
    } while(bytes_read < blob_size);
    efree(blob_buf);
    return rc;
}

/**
 * @brief Transfers the character data from a stream to a CLOB column in DB.
 *
 * @param[in] stm Stream with character data.
 * @param[in] clob_handle Handle to MimerLob already prepared by call to MimerSetLob.
 * @return 0 if successfully read all data from stream, negative error code otherwise.
 */
static ssize_t pdo_mimer_set_clob_data(php_stream *stm, MimerLob *clob_handle){
    ssize_t nbytes_valid;
    char *clob_buf = emalloc(MIMER_LOB_IN_CHUNK);
    while((nbytes_valid = get_valid_stream_chunk(stm, clob_buf, MIMER_LOB_IN_CHUNK)) > 0){
        handle_err(nbytes_valid = MimerSetClobData8(clob_handle, clob_buf, nbytes_valid),
            efree(clob_buf) ,
            return nbytes_valid)
    }
    efree(clob_buf);
    return nbytes_valid;
}

/**
 * @brief Transfers the character data from a stream to a NCLOB column in DB.
 *
 * @param[in] stm Stream with character data.
 * @param[in] clob_handle Handle to MimerLob already prepared by call to MimerSetLob.
 * @return 0 if successfully read all data from stream, negative error code otherwise.
 */
static ssize_t pdo_mimer_set_nclob_data(php_stream *stm, MimerLob *clob_handle){
    ssize_t nbytes_valid;
    char *nclob_buf = emalloc(MIMER_LOB_IN_CHUNK);

    while((nbytes_valid = get_valid_stream_chunk(stm, nclob_buf, MIMER_LOB_IN_CHUNK)) > 0){
        handle_err(nbytes_valid = MimerSetNclobData8(clob_handle, nclob_buf, nbytes_valid),
        efree(nclob_buf),
        return nbytes_valid)
    }
    efree(nclob_buf);
    return nbytes_valid;
}

/**
 * @brief Writes the LOB data from a streamable resource.
 *
 * @param stmt A pointer to the PDO statement handle object.
 * @param parameter The value to set for the parameter
 * @param paramno The number of the parameter to set
 * @return Mimer status code.
 */
static MimerError pdo_mimer_set_lob_data(pdo_stmt_t *stmt, zval *parameter, int16_t paramno){
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    MimerStatement *statement = &stmt_handle->statement;
    MimerError return_code, lob_type;
    MimerLob lob_handle;
    ssize_t lob_len = 0;
    size_t lob_size = 0;
    php_stream *stm = NULL;

    /** make a PHP stream from the zval
        TODO: More precise error info */
    if (Z_TYPE_P(parameter) != IS_RESOURCE){
        mimer_throw_except(&stmt_handle->error_info, "Expected a resource for LOB parameter", \
            MIMER_PDO_GENERAL_ERROR, SQLSTATE_GENERAL_ERROR, stmt->dbh->is_persistent, stmt->error_code)
    }
    php_stream_from_zval_no_verify(stm, parameter);
    if (!stm){
        mimer_throw_except(&stmt_handle->error_info, "Expected a stream resource for LOB parameter.", \
            MIMER_PDO_GENERAL_ERROR, SQLSTATE_GENERAL_ERROR, stmt->dbh->is_persistent, stmt->error_code)
    }
    zval_ptr_dtor(parameter);

    /** DB column type decides how we interpret and insert data */
    handle_err(return_code = MimerParameterType(*statement, paramno), return return_code)
    lob_type = return_code;

    /** lob_len has different meanings for BLOBs and CLOB/NCLOBs */
    lob_len = pdo_mimer_loblen(stm, lob_type, &lob_size);
    if (lob_len == 0){
        return MimerSetLob(*statement, paramno, 0, &lob_handle);
    } else if (lob_len < 0){
        /** TODO: error handling */
        mimer_throw_except(&stmt_handle->error_info, "Error while calculating LOB length.", \
            lob_len, SQLSTATE_GENERAL_ERROR, stmt->dbh->is_persistent, stmt->error_code)
    }

    /** make space in DB */
    return_on_err_stmt(return_code = MimerSetLob(*statement, paramno, lob_len, &lob_handle), return_code)

    /* start data transfer */
    if (MimerIsBlob(lob_type)){
        return_code = pdo_mimer_set_blob_data(stm, &lob_handle, lob_size);
    } else if (MimerIsClob(lob_type)){
        return_code = pdo_mimer_set_clob_data(stm, &lob_handle);
    } else if (MimerIsNclob(lob_type)){
        return_code = pdo_mimer_set_nclob_data(stm, &lob_handle);
    } else {
        /** TODO: More precise error info */
        mimer_throw_except(&stmt_handle->error_info, "Expected BLOB, CLOB or NCLOB column type", \
            MIMER_PDO_GENERAL_ERROR, SQLSTATE_GENERAL_ERROR, stmt->dbh->is_persistent, stmt->error_code)
    }

    return return_code;
}


/**
 * @brief Set parameter values for the statement
 * @param stmt A pointer to the PDO statement handle object.
 * @param parameter The value to set for the parameter
 * @param paramno The number of the parameter to set
 * @param param_type The parameter's data type
 * @return return code from MimerSetX()
 */
static MimerError pdo_mimer_stmt_set_params(pdo_stmt_t *stmt, zval *parameter, int16_t paramno, enum pdo_param_type param_type) {
    pdo_mimer_stmt *mimer_stmt = stmt->driver_data;
    MimerStatement *statement = &mimer_stmt->statement;
    MimerError return_code = MIMER_SUCCESS;

    switch (PDO_PARAM_TYPE(param_type)) {
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

        case PDO_PARAM_LOB:
            return_code = pdo_mimer_set_lob_data(stmt, parameter, paramno);
            break;

        /* unimplemented */
#       define UNSUPPORTED(pdo_param) \
        case pdo_param:               \
            mimer_throw_except(&mimer_stmt->error_info, #pdo_param " support is not yet implemented", \
                MIMER_FEATURE_NOT_IMPLEMENTED, SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED, stmt->dbh->is_persistent, stmt->error_code) \
            break;

        UNSUPPORTED(PDO_PARAM_INPUT_OUTPUT)
        UNSUPPORTED(PDO_PARAM_STR_NATL)
        UNSUPPORTED(PDO_PARAM_STR_CHAR)
        UNSUPPORTED(PDO_PARAM_STMT)
        default:
            mimer_throw_except(&mimer_stmt->error_info, "Unexpected parameter type",
                MIMER_PDO_GENERAL_ERROR, SQLSTATE_GENERAL_ERROR, stmt->dbh->is_persistent, stmt->error_code)
    }

    return return_code;
}


/**
 * @brief Handle bound parameters and columns
 * @param stmt A pointer to the PDO statement handle object.
 * @param param The structure describing either a statement parameter or a bound column.
 * @param event_type The type of event to occur for this parameter.
 * @return 1 for success, 0 for failure.
 * @remark This hook will be called for each bound parameter and bound column in the statement.
*           For ALLOC and FREE events, a single call will be made for each parameter or column
 * @see <a href="https://php-legacy-docs.zend.com/manual/php5/en/internals2.pdo.implementing">Implementing PDO</a>
 * @see <a href="https://www.php.net/manual/en/pdo.constants.php">PHP: Predefined Constants</a>
 */
static int pdo_mimer_stmt_param_hook(pdo_stmt_t *stmt, struct pdo_bound_param_data *param, enum pdo_param_event event_type) {
    pdo_mimer_stmt *stmt_handle = stmt->driver_data;
    MimerError return_code;

    if (stmt_handle->statement == NULL || !param->is_param) { /* nothing to do */
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

    zval *parameter = Z_ISREF(param->parameter) ? Z_REFVAL(param->parameter) : &param->parameter;
    if (event_type == PDO_PARAM_EVT_ALLOC && !Z_ISREF(param->parameter)) {
        return_code = pdo_mimer_stmt_set_params(stmt, parameter, param->paramno + 1,param->param_type);
    } else if (event_type == PDO_PARAM_EVT_EXEC_PRE && Z_ISREF(param->parameter)) {
        return_code = pdo_mimer_stmt_set_params(stmt, parameter, param->paramno + 1,param->param_type);
    } else {
        return 1;
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

    return_on_err_stmt(MimerCloseCursor(stmt_handle->statement), 0)

    return 1;
}


/**
 * @brief The PHP method <code>mimerAddBatch()</code> extending the <code>PDOStatement</code> class to be able to set
 *     multiple parameter values on a prepared statement with Mimer SQL
 * @param return_value true on success, false if failed to add batch statement
 * @throws PDOException if PDO Statement object, PDO Mimer statement object, or MimerStatement is uninitialized
 */
PHP_METHOD(PDOStatement_MimerSQL_Ext, mimerAddBatch) {
    pdo_stmt_t *stmt= Z_PDO_STMT_P(ZEND_THIS);
    pdo_mimer_stmt *mimer_stmt = (pdo_mimer_stmt *)stmt->driver_data;
    zend_class_entry *pdoexception_ce =  zend_hash_str_find_ptr(CG(class_table), "pdoexception", sizeof("pdoexception") -1);

    if (stmt->dbh == NULL || mimer_stmt == NULL || mimer_stmt->statement == NULL) {
        zend_throw_error(pdoexception_ce,
                         !stmt->dbh ? "PDO object is uninitialized." :
                         !mimer_stmt ? "PDO Mimer statement object is uninitialized." :
                         "No statement started.");
        RETURN_THROWS();
    }

    handle_err(MimerAddBatch(mimer_stmt->statement), pdo_mimer_stmt_error(stmt), RETURN_FALSE)
    RETURN_TRUE;
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

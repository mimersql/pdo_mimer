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


/**
 * @brief PDO Mimer method to end a statement and free necessary memory.
 * @param stmt [in] A pointer to the PDOStatement handle object.
 * @return 1 upon success
 * @return 0 upon failure
 */
static int pdo_mimer_stmt_dtor(pdo_stmt_t *stmt) {
    int success = 1;

    if (PDO_MIMER_CURSOR_OPEN && !MIMER_SUCCEEDED(MimerCloseCursor(MIMER_STMT))) {
        pdo_mimer_stmt_error();
        success = 0;
    }

    /* if unable to properly end statement, throw an except since something more fatal has probably happened */
    if (!MIMER_SUCCEEDED(MimerEndStatement(&MIMER_STMT))) {
        pdo_mimer_stmt_error();
        mimer_throw_except(stmt);
        success = 0;
    }

    if (PDO_MIMER_STMT_ERROR->msg != NULL)
        efree(PDO_MIMER_STMT_ERROR->msg);

    efree(stmt->driver_data);
    stmt->driver_data = NULL;
    return success;
}


/**
 * @brief Execute a prepared SQL statement.
 * @param stmt [in] A pointer to the PDOStatement handle object.
 * @return 1 upon success
 * @return 0 upon failure
 * @remark Driver is responsible for setting the column_count field in stmt for result set statements.
*  @see <a href="https://php-legacy-docs.zend.com/manual/php5/en/internals2.pdo.pdo-stmt-t">
 *          PDO Driver How-To: pdo_stmt_t definition
 *      </a>
 */
static int pdo_mimer_stmt_executer(pdo_stmt_t *stmt) {
    int num_columns;

    if(MimerStatementHasResultSet(MIMER_STMT)) {
        if (PDO_MIMER_CURSOR_OPEN) {
            if(!MIMER_SUCCEEDED(MimerCloseCursor(MIMER_STMT)))
                return 0;
            PDO_MIMER_CURSOR_OPEN = false;
        }

        if (MIMER_SUCCEEDED(num_columns = MimerColumnCount(MIMER_STMT))
                && MIMER_SUCCEEDED(MimerOpenCursor(MIMER_STMT))) {
            PDO_MIMER_CURSOR_OPEN = true;
            php_pdo_stmt_set_column_count(stmt, num_columns);
            return 1;
        }
    } else if (MIMER_SUCCEEDED(MimerExecute(MIMER_STMT))) {
        return 1;
    }

    pdo_mimer_stmt_error();
    return 0;
}


/* lookup table for converting PDO fetch orientation to Mimer SQL fetch operation */
const int mimer_fetch_op_lut[] = {
        MIMER_NEXT,
        MIMER_PREVIOUS,
        MIMER_FIRST,
        MIMER_LAST,
        MIMER_ABSOLUTE,
        MIMER_RELATIVE,
};


/**
 * @brief This function will be called by PDO to fetch a row from a previously executed statement object.
 * @param stmt [in] A pointer to the PDOStatement handle object.
 * @param ori [in] One of PDO_FETCH_ORI_xxx which will determine which row will be fetched.
 * @param offset [in] If @p ori is set to PDO_FETCH_ORI_ABS or PDO_FETCH_ORI_REL, offset represents the row desired or the row
 *      relative to the current position, respectively. Otherwise, this value is ignored.
 * @return 1 upon success
 * @return 0 upon failure
 * @remark The results of this fetch are driver dependent and the data is usually stored in the driver_data member of
 * the pdo_stmt_t object. The ori and offset parameters are only meaningful if the statement represents a scrollable
 * cursor.
 * @see <a href="https://php-legacy-docs.zend.com/manual/php5/en/internals2.pdo.implementing">
 *          PDO Driver How-To: Fleshing out your skeleton (SKEL_stmt_fetch)
 *      </a>
 */
static int pdo_mimer_stmt_fetch(pdo_stmt_t *stmt, enum pdo_fetch_orientation ori, zend_long offset) {
    MimerReturnCode return_code;

    if (PDO_MIMER_SCROLLABLE)
        return_code = MimerFetchScroll(MIMER_STMT, mimer_fetch_op_lut[ori], (int32_t) offset);
    else
        return_code = MimerFetch(MIMER_STMT);

    if (!MIMER_SUCCEEDED(return_code)) {
        pdo_mimer_stmt_error();
        return 0;
    }

    return return_code != MIMER_NO_DATA;
}


/**
 * @brief This function will be called by PDO to query information about a particular column.
 * @param stmt [in] A pointer to the PDOStatement handle object.
 * @param colno [in] The column number to be queried.
 * @return 1 upon success
 * @return 0 upon failure
 * @remark PDO is zero-indexed for columns, Mimer SQL's C API is one-indexed.
 */
static int pdo_mimer_describe_col(pdo_stmt_t *stmt, int colno) {
    MimerReturnCode return_code;
    int mim_colno = colno + 1;

    MimerGetStr(MimerColumnName8, str_buf, return_code, MIMER_STMT, mim_colno);
    if (!MIMER_SUCCEEDED(return_code)) {
        pdo_mimer_stmt_error();
        return 0;
    }

	stmt->columns[colno] = (struct pdo_column_data) {
            zend_string_init(str_buf, return_code, 0), /* return_code gives str len */
            SIZE_MAX,
            0
    };

    return 1;
}

/**
 * @brief This function will be called by PDO to retrieve data from the specified column.
 * @param stmt [in] stmt [in] A pointer to the PDOStatement handle object.
 * @param colno [in] The column number to be queried.
 * @param result [out] Pointer to the retrieved data.
 * @param type [in] Parameter data type.
 * @return 1 upon success
 * @return 0 upon failure
 * @remark PDO is zero-indexed for columns, Mimer SQL's C API is one-indexed.
 */
static int pdo_mimer_stmt_get_col_data(pdo_stmt_t *stmt, int colno, zval *result, enum pdo_param_type *type) {
    MimerReturnCode return_code;
    int32_t column_type;

    int16_t mim_colno = colno + 1;
    if (!MIMER_SUCCEEDED(column_type = MimerColumnType(MIMER_STMT, mim_colno))) {
        pdo_mimer_stmt_error();
        return 0;
    }

    return_code = MimerIsNull(MIMER_STMT, mim_colno);
    if (return_code > 0)
        return 1;
    else if (return_code < 0){
        pdo_mimer_stmt_error();
        return 0;
    }

    if (MimerIsInt64(column_type)) {
        int64_t data;

        if (MIMER_SUCCEEDED(return_code = MimerGetInt64(MIMER_STMT, mim_colno, &data))) {
            ZVAL_LONG(result, data);
            return 1;
        }
    }

    else if (MimerIsInt32(column_type)) {
        int32_t data;

        if (MIMER_SUCCEEDED(return_code = MimerGetInt32(MIMER_STMT, mim_colno, &data))) {
            ZVAL_LONG(result, data);
            return 1;
        }
    }

    else if (MimerIsBinary(column_type)){
        if (MIMER_SUCCEEDED(return_code = MimerGetBinary(MIMER_STMT, mim_colno, NULL, 0))) {
            size_t len = return_code + 1;
            char *data = emalloc(len);
            data[len-1] = '\0';

            if (MIMER_SUCCEEDED(return_code = MimerGetBinary(MIMER_STMT, mim_colno, data, len)))
                ZVAL_STRING(result, data);

            efree(data);
        }
    }

    else if (MimerIsBoolean(column_type)) {
        if (MIMER_SUCCEEDED(return_code = MimerGetBoolean(MIMER_STMT, mim_colno))) {
            ZVAL_BOOL(result, return_code);
            return 1;
        }
    }

    else if (MimerIsFloat(column_type)){
        float data;
        if (MIMER_SUCCEEDED(return_code = MimerGetFloat(MIMER_STMT, mim_colno, &data))) {
            ZVAL_DOUBLE(result, data);
            convert_to_string(result);
            return 1;
        }
    }

    else if (MimerIsDouble(column_type)){
        double data;
        if (MIMER_SUCCEEDED(return_code = MimerGetDouble(MIMER_STMT, mim_colno, &data))) {
            ZVAL_DOUBLE(result, data);
            convert_to_string(result);
            return 1;
        }
    }

    else if(MimerIsDecimal(column_type) || MimerIsDatetime(column_type)){
        /* TODO: Await update to API.
            Temporary block for handling types which currently gives segfault
            when checking string length.  */
        const int max_chars = 100;
        char str[max_chars];

        if (MIMER_SUCCEEDED(return_code = MimerGetString8(MIMER_STMT, mim_colno, str, max_chars)))
            ZVAL_STRING(result, str);
    }

    else if (MimerIsBlob(column_type)) {
        if (!type || *type == PDO_PARAM_STR){ // type is null on fetch where no PARAM_ type has been given
            MimerLob lob_handle;
            size_t lob_len;
            if (MIMER_SUCCEEDED(return_code = MimerGetLob(MIMER_STMT, mim_colno, &lob_len, &lob_handle))){
                char *buf = emalloc(lob_len+1);
                return_code = MimerGetBlobData(&lob_handle, buf, lob_len);
                buf[lob_len] = '\0';
                ZVAL_STRING(result, buf);
                efree(buf);
            }
        }

        else {
            php_stream *stream = pdo_mimer_create_lob_stream(stmt, mim_colno, MIMER_BLOB);
            if (stream)
                php_stream_to_zval(stream, result)
            else {
                pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = PDO_MIMER_UNABLE_PHPSTREAM_ALLOC,
                                    "Unable to allocate PHP stream");
            }
        }
    }

    else if (MimerIsClob(column_type) || MimerIsNclob(column_type)) {
        if (!type || *type == PDO_PARAM_STR){
            MimerLob lob_handle;
            size_t lob_len;
            if (MIMER_SUCCEEDED(return_code = MimerGetLob(MIMER_STMT, mim_colno, &lob_len, &lob_handle))){
                char *buf = emalloc(lob_len*MIMER_MAX_MB_LEN+1);
                return_code = MimerGetNclobData8(&lob_handle, buf, lob_len*MIMER_MAX_MB_LEN+1);
                ZVAL_STRING(result, buf);
                efree(buf);
            }
        }

        else {
            php_stream *stream = NULL;
            if (MimerIsClob(column_type))
                stream = pdo_mimer_create_lob_stream(stmt, mim_colno, MIMER_CLOB);
            else
                stream = pdo_mimer_create_lob_stream(stmt, mim_colno, MIMER_NCLOB);

            if (stream)
                php_stream_to_zval(stream, result)
            else {
                pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = PDO_MIMER_UNABLE_PHPSTREAM_ALLOC,
                                    "Unable to allocate PHP stream");
            }
        }
    }

    else if (MimerIsString(column_type)){
        if (MIMER_SUCCEEDED(return_code = MimerGetString8(MIMER_STMT, mim_colno, NULL, 0))) {
            size_t str_len = return_code + 1; // +1 for null-terminator
            char *data = emalloc(str_len);

            if (MIMER_SUCCEEDED(return_code = MimerGetString8(MIMER_STMT, mim_colno, data, str_len)))
                ZVAL_STRING(result, data);

            efree(data);
        }
    }

    else {
        pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = PDO_MIMER_UNKNOWN_COLUMN_TYPE,
                               "Unknown column type");
        return 0;
    }

    if (!MIMER_SUCCEEDED(return_code)) {
        pdo_mimer_stmt_error();
        return 0;
    }

    return 1;
}

/**
 * @brief Gets the length of the LOB stream, in bytes for all LOB types
 * and in number of characters for CLOBs and NCLOBs.
 *
 * @param[in] stm Pointer to the PHP stream with the data.
 * @param[in] lob_type Mimer constant for one of BLOB/CLOB/NCLOB.
 * @return Number of characters or bytes found in stream, depending on LOB type,
 * or negative error code.
 *
 * @remark Currently only works for NCLOBS if the stream data is UTF8 encoded.
 *
 */
static ssize_t pdo_mimer_loblen(php_stream *stm, int32_t lob_type){
    ssize_t nchars = 0;
    char c;

    // Clobs have one byte per character encoding (Latin 8859-1)
    if (MimerIsBlob(lob_type) || MimerIsClob(lob_type)) {
        php_stream_seek(stm, 0, SEEK_END);
        nchars = php_stream_tell(stm);

    } else if (MimerIsNclob(lob_type)) {
        while(!php_stream_eof(stm)){
            c = (char) php_stream_getc(stm);
            if((c & 0xC0) != 0x80) // is not a UTF-8 continuation byte
                nchars++;
        }

    } else {
        return PDO_MIMER_UNKNOWN_LOB_TYPE;
    }

    php_stream_rewind(stm);
    return nchars;
}

/**
 * @brief Writes the LOB data from a streamable resource.
 *
 * @param stmt A pointer to the PDO statement handle object.
 * @param parameter The value to set for the parameter
 * @param paramno The number of the parameter to set
 * @return Mimer status code.
 */
static MimerReturnCode pdo_mimer_set_lob_data(pdo_stmt_t *stmt, zval *parameter, int16_t paramno){
    MimerReturnCode return_code = MIMER_SUCCESS;
    MimerLob lob_handle;
    int32_t lob_type;
    ssize_t lob_len;
    php_stream *stm = NULL;
    char data_buf[MIMER_LOB_IN_CHUNK];
    ssize_t nread_bytes;

    /** Try to make a PHP stream from the resource variable
        TODO: More precise error info */
    if (Z_TYPE_P(parameter) != IS_RESOURCE) {
        pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = PDO_MIMER_GENERAL_ERROR,
                               "Expected a resource for LOB parameter");
        return return_code;
    }
    php_stream_from_zval_no_verify(stm, parameter);
    if (!stm){
        pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = PDO_MIMER_GENERAL_ERROR,
                               "Expected a stream resource for LOB parameter");
        return return_code;
    }

    /** LOB type decides how we insert data from stream */
    if (!MIMER_SUCCEEDED(return_code = MimerParameterType(MIMER_STMT, paramno)))
        return return_code;
    lob_type = return_code;
    if (!MimerIsBlob(lob_type) && !MimerIsClob(lob_type) && !MimerIsNclob(lob_type)){
        /** TODO: More precise error code */
        pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = PDO_MIMER_GENERAL_ERROR,
                               "Expected BLOB, CLOB or NCLOB column type");
        return return_code;
    }

    /** Need LOB len for MimerSetLob (len =bytes for BLOBS, =chars for CLOBS/NCLOBS) */
    lob_len = pdo_mimer_loblen(stm, lob_type);
    if (lob_len == 0)
        return MimerSetLob(MIMER_STMT, paramno, 0, &lob_handle);
    else if (lob_len < 0){
        /** TODO: error code */
        pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = lob_len,
                               "Error while calculating LOB length");
        return return_code;
    }

    /* Move data into DB in chunks (not visible until statement is executed) */
    if (!MIMER_SUCCEEDED(return_code = MimerSetLob(MIMER_STMT, paramno, lob_len, &lob_handle)))
        return return_code;

    if (MimerIsBlob(lob_type)){
        while(!php_stream_eof(stm) && MIMER_SUCCEEDED(return_code)){
            nread_bytes = php_stream_read(stm, data_buf, MIMER_LOB_IN_CHUNK);
            return_code = MimerSetBlobData(&lob_handle, data_buf, nread_bytes);
        }

    } else if (MimerIsClob(lob_type)){
        while(!php_stream_eof(stm) && MIMER_SUCCEEDED(return_code)){
            nread_bytes = php_stream_read(stm, data_buf, MIMER_LOB_IN_CHUNK);
            return_code = MimerSetNclobData8(&lob_handle, data_buf, nread_bytes);
        }

    } else if (MimerIsNclob(lob_type)){
        ssize_t nvalid_bytes;
        while(!php_stream_eof(stm) && MIMER_SUCCEEDED(return_code)){
            nread_bytes = php_stream_read(stm, data_buf, MIMER_LOB_IN_CHUNK);
            nvalid_bytes = nread_bytes;

            // Avoid partial characters in chunk: cutoff before last non-continuation byte
            if (!php_stream_eof(stm)) {
                for(; nvalid_bytes > 0;){
                    if ((data_buf[(nvalid_bytes--) -1] & 0xC0) != 0x80) // Only works for UTF-8
                        break;
                }
                // Include left-out bytes in next read
                php_stream_seek(stm, nvalid_bytes - nread_bytes, SEEK_CUR);
            }

            if(nvalid_bytes <= 0){
                pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = PDO_MIMER_GENERAL_ERROR,
                               "Error when reading NCLOB resource");
                break;
            }
            return_code = MimerSetNclobData8(&lob_handle, data_buf, nvalid_bytes);
        }
    }

    return return_code;
}

/**
 * @brief Set parameter values for the statement to be executed.
 * @param stmt [in] A pointer to the PDOStatement handle object.
 * @param parameter [in] The value to set for the parameter.
 * @param paramno [in] The number of the parameter to set.
 * @param param_type [in] The parameter's data type.
 * @return return code from <code>MimerSetX()</code>
 */
static MimerReturnCode pdo_mimer_stmt_set_params(pdo_stmt_t *stmt, zval *parameter, int16_t paramno, enum pdo_param_type param_type) {
    MimerReturnCode return_code;
    MimerReturnCode mim_type;

    if (Z_TYPE_P(parameter) == IS_NULL)
        return MimerSetNull(MIMER_STMT, paramno);

    if (!MIMER_SUCCEEDED(return_code = MimerParameterType(MIMER_STMT, paramno)))
        return return_code;

    mim_type = return_code;
    if (MimerIsInt32(mim_type)) {
        // TODO: Overflow check
        zend_long val = zval_get_long(parameter);
        return_code = MimerSetInt32(MIMER_STMT, paramno, val);
    }
    else if (MimerIsInt64(mim_type)) {
        zend_long val = zval_get_long(parameter);
        return_code = MimerSetInt64(MIMER_STMT, paramno, val);
    }
    else if (MimerIsBoolean(mim_type)) {
        bool val = zend_is_true(parameter);
        return_code = MimerSetBoolean(MIMER_STMT, paramno, val);
    }
    else if (MimerIsFloat(mim_type)) {
        // TODO: Overflow check
        double val = zval_get_double(parameter);
        return_code = MimerSetFloat(MIMER_STMT, paramno, (float)val);
    }
    else if (MimerIsDouble(mim_type)) {
        double val = zval_get_double(parameter);
        return_code = MimerSetDouble(MIMER_STMT, paramno, val);
    }
    else if (MimerIsBinary(mim_type)) {
        zend_string *str = zval_get_string(parameter);
        return_code = MimerSetBinary(MIMER_STMT, paramno, ZSTR_VAL(str), ZSTR_LEN(str));
        zend_string_release(str);
    }
    else if (MimerIsBlob(mim_type) || MimerIsClob(mim_type) || MimerIsNclob(mim_type)) {

        if (param_type == PDO_PARAM_LOB)
            return_code = pdo_mimer_set_lob_data(stmt, parameter, paramno);

        else {
            zend_string *str = zval_get_string(parameter);
            size_t lob_len = ZSTR_LEN(str);
            MimerLob lob_handle;
            if (MIMER_SUCCEEDED(return_code = MimerSetLob(MIMER_STMT, paramno, lob_len, &lob_handle))){
                if (MimerIsBlob(mim_type))
                    return_code = MimerSetBlobData(&lob_handle, ZSTR_VAL(str), lob_len);
                else
                    return_code = MimerSetNclobData8(&lob_handle, ZSTR_VAL(str), lob_len);
            }
            zend_string_release(str);
        }
    }
    else if (MimerIsString(mim_type)){
        zend_string *str = zval_get_string(parameter);
        return_code = MimerSetString8(MIMER_STMT, paramno, ZSTR_VAL(str));
        zend_string_release(str);
    }
    else
        pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = PDO_MIMER_GENERAL_ERROR,
            "Unknown parameter type");

    return return_code;
}


/**
 * @brief Handle bound parameters and columns.
 * @param stmt [in] A pointer to the PDOStatement handle object.
 * @param param [in] The structure describing either a statement parameter or a bound column.
 * @param event_type [in] The type of event to occur for this parameter.
 * @return 1 upon success
 * @return 0 upon failure
 * @remark This hook will be called for each bound parameter and bound column in the statement.
 * For ALLOC and FREE events, a single call will be made for each parameter or column.
 * @see <a href="https://php-legacy-docs.zend.com/manual/php5/en/internals2.pdo.implementing">Implementing PDO</a>
 * @see <a href="https://www.php.net/manual/en/pdo.constants.php">PHP: Predefined Constants</a>
 */
static int pdo_mimer_stmt_param_hook(pdo_stmt_t *stmt, struct pdo_bound_param_data *param, enum pdo_param_event event_type) {
    MimerReturnCode return_code = MIMER_SUCCESS;
    int16_t paramno = param->paramno + 1; /* Mimer SQL is one-indexed */

    if (MIMER_STMT == NULL || !param->is_param) { /* if not param, is of column type */
        return 1;
    }

    if (param->paramno >= INT16_MAX) {
        pdo_mimer_custom_error(stmt, SQLSTATE_FEATURE_NOT_SUPPORTED, PDO_MIMER_VALUE_TOO_LARGE,
                               "Parameter number is larger than 32767. Mimer only supports up to 32767 parameters");
        return 0;
    }

    /**
     * Values bound with PDOStatement::bindParam() cannot be touched until PDO_PARAM_EVT_EXEC_PRE. When using
     * bindParam(), values are passed by reference, ergo Z_ISREF() check is performed.
     *
     * Since PDO_PARAM_EVT_EXEC_PRE only occurs first when PDOStatement::execute() is called, we set the parameter
     * values when bindValue() is called already at PDO_PARAM_EVT_ALLOC so that PDOStatement::mimerAddBatch() can be
     * used before execute(). This therefore limits usage of mimerAddBatch() to only be able to be used in tandem with
     * bindValue() and cannot be used with bindParam(). At least, for now.
     */
    switch(event_type) {
        case PDO_PARAM_EVT_ALLOC:
            if (!Z_ISREF(param->parameter)) /* bindValue() was used, let's set those params */
                /* if param is ref, that means bindParam() was used, cannot continue until EXEC_PRE */
                return_code = pdo_mimer_stmt_set_params(stmt, &param->parameter, paramno, param->param_type);
            break;

        case PDO_PARAM_EVT_EXEC_PRE:
            /* cannot set parameters if a cursor is currently open */
            if (PDO_MIMER_CURSOR_OPEN) {
                if(!MIMER_SUCCEEDED(MimerCloseCursor(MIMER_STMT)))
                    return 0;
                PDO_MIMER_CURSOR_OPEN = false;
            }

            if (Z_ISREF(param->parameter)){ /* bindParam() was used, let's set those params */
                /* if param is not ref, that means bindValue() was used which should have been set in EVT_ALLOC */
                return_code = pdo_mimer_stmt_set_params(stmt, Z_REFVAL(param->parameter), paramno, param->param_type);
            }
            break;

        default:
            break;
    }

    if (!MIMER_SUCCEEDED(return_code)) {
        pdo_mimer_stmt_error();
        return 0;
    }

    return 1;
}


/**
 * @brief The function PDO calls to close the current cursor.
 * @param stmt [in] A pointer to the PDOStatement handle object.
 * @return 1 upon success
 * @return 0 upon failure
 */
static int pdo_mimer_cursor_closer(pdo_stmt_t *stmt) {
    if (PDO_MIMER_CURSOR_OPEN) {
        if (!MIMER_SUCCEEDED(MimerCloseCursor(MIMER_STMT))) {
            pdo_mimer_stmt_error();
            return 0;
        }

        PDO_MIMER_CURSOR_OPEN = false;
    }

    return 1;
}


static inline const char* get_native_type_string(int32_t col_type) {
    switch(col_type) {
        case MIMER_BINARY:
            return "BINARY";
        case MIMER_BINARY_VARYING:
            return "VARBINARY";
        case MIMER_BLOB:
        case MIMER_BLOB_LOCATOR:
        case MIMER_NATIVE_BLOB:
        case MIMER_NATIVE_BLOB_LOCATOR:
            return "BLOB";

        case MIMER_CHARACTER:
            return "CHAR";
        case MIMER_CHARACTER_VARYING:
            return "VARCHAR";
        case MIMER_CLOB:
        case MIMER_CLOB_LOCATOR:
        case MIMER_NATIVE_CLOB:
        case MIMER_NATIVE_CLOB_LOCATOR:
            return "CLOB";

        case MIMER_NCHAR:
            return "NCHAR";
        case MIMER_NCHAR_VARYING:
            return "NVARCHAR";
        case MIMER_NCLOB:
        case MIMER_NCLOB_LOCATOR:
        case MIMER_NATIVE_NCLOB:
        case MIMER_NATIVE_NCLOB_LOCATOR:
            return "NCLOB";

        case MIMER_BOOLEAN:
            return "BOOLEAN";

        case MIMER_DECIMAL:
            return "DECIMAL";

        case MIMER_FLOAT:
        case MIMER_T_FLOAT:
            return "FLOAT";

        case MIMER_INTEGER:
        case MIMER_NATIVE_INTEGER:
        case MIMER_NATIVE_INTEGER_NULLABLE:
        case MIMER_T_INTEGER:
        case MIMER_T_UNSIGNED_INTEGER:
        case MIMER_UNSIGNED_INTEGER:
            return "INTEGER";

        case MIMER_NATIVE_BIGINT:
        case MIMER_NATIVE_BIGINT_NULLABLE:
        case MIMER_T_BIGINT:
        case MIMER_T_UNSIGNED_BIGINT:
            return "BIGINT";

        case MIMER_NATIVE_DOUBLE:
        case MIMER_NATIVE_DOUBLE_NULLABLE:
        case MIMER_T_DOUBLE:
            return "DOUBLE";

        case MIMER_NATIVE_REAL:
        case MIMER_NATIVE_REAL_NULLABLE:
        case MIMER_T_REAL:
            return "REAL";

        case MIMER_NATIVE_SMALLINT:
        case MIMER_NATIVE_SMALLINT_NULLABLE:
        case MIMER_T_SMALLINT:
        case MIMER_T_UNSIGNED_SMALLINT:
            return "SMALLINT";

        case MIMER_DATE:
            return "DATE";
        case MIMER_TIME:
            return "TIME";
        case MIMER_TIMESTAMP:
            return "TIMESTAMP";

        case MIMER_INTERVAL_DAY:
            return "INTERVAL_DAY";
        case MIMER_INTERVAL_DAY_TO_HOUR:
            return "INTERVAL_DAY_TO_HOUR";
        case MIMER_INTERVAL_DAY_TO_MINUTE:
            return "INTERVAL_DAY_TO_MINUTE";
        case MIMER_INTERVAL_DAY_TO_SECOND:
            return "INTERVAL_DAY_TO_SECOND";
        case MIMER_INTERVAL_HOUR:
            return "INTERVAL_HOUR";
        case MIMER_INTERVAL_HOUR_TO_MINUTE:
            return "INTERVAL_HOUR_TO_MINUTE";
        case MIMER_INTERVAL_HOUR_TO_SECOND:
            return "INTERVAL_HOUR_TO_SECOND";
        case MIMER_INTERVAL_MINUTE:
            return "INTERVAL_MINUTE";
        case MIMER_INTERVAL_MINUTE_TO_SECOND:
            return "INTERVAL_MINUTE_TO_SECOND";
        case MIMER_INTERVAL_MONTH:
            return "INTERVAL_MONTH";
        case MIMER_INTERVAL_SECOND:
            return "INTERVAL_SECOND";
        case MIMER_INTERVAL_YEAR:
            return "INTERVAL_YEAR";
        case MIMER_INTERVAL_YEAR_TO_MONTH:
            return "INTERVAL_YEAR_TO_MONTH";

        default:
            return NULL;
    }
}

static int pdo_mimer_get_column_meta(pdo_stmt_t *stmt, zend_long colno, zval *return_value) {
    int16_t mimcolno;
    int32_t col_type;
    enum pdo_param_type pdo_param_type;
    const char *php_type;
    const char *native_type;
    zval flags;

    /* stmt->columns is only allocated when a fetch is performed, but we have access to column info prior to that */
    if (!stmt->columns) {
        if (!MIMER_SUCCEEDED(stmt->column_count = MimerColumnCount(MIMER_STMT))) {
            pdo_mimer_stmt_error();
            return FAILURE;
        }

        stmt->columns = ecalloc(stmt->column_count, sizeof(struct pdo_column_data));
        if (!pdo_mimer_describe_col(stmt, colno)) {
            efree(stmt->columns);
            return FAILURE;
        }
    }


    mimcolno = colno + 1;
    if (!MIMER_SUCCEEDED(col_type = MimerColumnType(MIMER_STMT, mimcolno))) {
        pdo_mimer_stmt_error();
        return FAILURE;
    }

    array_init(return_value);
    array_init(&flags);

    /* TODO: primary_key, not_null, unique_key, multiple_key, auto_increment when/if supported by C API*/
    if (col_type == MIMER_UNSIGNED_INTEGER || col_type == MIMER_T_UNSIGNED_INTEGER ||
        col_type == MIMER_T_UNSIGNED_SMALLINT || col_type == MIMER_T_UNSIGNED_BIGINT)
        add_next_index_string(&flags, "unsigned");
    if (MimerIsBlob(col_type) || MimerIsClob(col_type) || MimerIsNclob(col_type))
        add_next_index_string(&flags, "blob");

    if (MimerIsInt32(col_type) || MimerIsInt64(col_type)) {
        php_type = "integer";
        pdo_param_type = PDO_PARAM_INT;
    } else if (MimerIsFloat(col_type) || MimerIsDouble(col_type)) {
        php_type = "double";
        pdo_param_type = PDO_PARAM_INT;
    } else if (MimerIsBoolean(col_type)) {
        php_type = "boolean";
        pdo_param_type = PDO_PARAM_BOOL;
    } else {
        php_type = "string";
        pdo_param_type = PDO_PARAM_STR;
    }

    add_assoc_long(return_value, "pdo_type", pdo_param_type);
    add_assoc_string(return_value, "type", php_type);
    if ((native_type = get_native_type_string(col_type)) != NULL)
        add_assoc_string(return_value, "native_type", native_type);
    /* TODO: scale, table when/if supported by C API */

    add_assoc_zval(return_value, "flags", &flags); /* MySQL does this, adds flags array to returned "dict" in PHP */
    return SUCCESS;
}


/**
 * @brief The PHP method <code>mimerAddBatch()</code> extends the <code>PDOStatement</code> class to be able to set
 * multiple parameter values on a prepared statement with Mimer SQL.
 * @param return_value [out] true on success, false if failed to add batch statement.
 * @throws PDOException if PDO Statement object, PDO Mimer statement object, or MimerStatement is uninitialized.
 */
PHP_METHOD(PDOStatement_MimerSQL_Ext, mimerAddBatch) {
    pdo_stmt_t *stmt = Z_PDO_STMT_P(ZEND_THIS); /* gets PDOStatement object from which mimerAddBatch() was called on */
    /* get PDOException class object so we can throw an exception on an error */
    zend_class_entry *pdoexception_ce =  zend_hash_str_find_ptr(CG(class_table), "pdoexception", sizeof("pdoexception") -1);

    if (!stmt || !stmt->dbh || !PDO_MIMER_STMT || !MIMER_STMT) {
        zend_throw_error(pdoexception_ce,
                         !stmt ? "PDOStatement object is uninitialized." :
                         !stmt->dbh ? "PDO object is uninitialized." :
                         !PDO_MIMER_STMT? "PDO Mimer handle object is uninitialized." :
                         "A MimerStatement has not yet been prepared.");
        RETURN_THROWS();
    }

    if (!MIMER_SUCCEEDED(MimerAddBatch(MIMER_STMT))) {
            pdo_mimer_stmt_error();
            RETURN_FALSE;
    }

    RETURN_TRUE;
}


/* the methods implemented by PDO Mimer to interface with PDOStatement */
const struct pdo_stmt_methods pdo_mimer_stmt_methods = {
        pdo_mimer_stmt_dtor,   /* statement destructor method */
        pdo_mimer_stmt_executer,   /* statement executor method */
        pdo_mimer_stmt_fetch,   /* statement fetcher method */
        pdo_mimer_describe_col,   /* statement describer method */
        pdo_mimer_stmt_get_col_data,   /* statement get column method */
        pdo_mimer_stmt_param_hook,   /* statement parameter hook method */
        NULL,   /* statement set attribute method */
        NULL,   /* statement get attribute method */
        pdo_mimer_get_column_meta,   /* statement get column data method */
        NULL,   /* next statement rowset method */
        pdo_mimer_cursor_closer,   /* statement cursor closer method */
};

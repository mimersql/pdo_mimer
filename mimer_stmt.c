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
    if(MimerStatementHasResultSet(MIMER_STMT)) {
        int num_columns;
        if (MIMER_SUCCEEDED(num_columns = MimerColumnCount(MIMER_STMT)) && MIMER_SUCCEEDED(MimerOpenCursor(MIMER_STMT))) {
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
            0 /* TODO: support precision? */
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
        php_stream *stream = pdo_mimer_create_lob_stream(stmt, mim_colno, MIMER_BLOB);

        if (stream) {
            php_stream_to_zval(stream, result)
        } else {
            pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = PDO_MIMER_UNABLE_PHPSTREAM_ALLOC,
                                   "Unable to allocate php stream");
            return 0;
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
        return PDO_MIMER_ENC_STATEFUL;
    }

    /** If only last char is misformed, the furtest away from the buffer end
     * that the start of a full character can be is MAX_MB_LEN + (MAX_MB_LEN - 1) */
    pos = len - (2 * MIMER_MAX_MB_LEN - 2);

    /* find the start of a character */
    while ((nbytes = mblen(buf + pos, len - pos)) < 0){
        pos++;
        if (pos == len)
            /* No valid characters in given window, can't be due to partials */
            return PDO_MIMER_ENC_UNKNOWN;
    }

    /* pass through all valid chars */
    while (pos < len && (nbytes = mblen(buf + pos, len - pos)) > 0)
        pos += nbytes;

    if ((len - pos) < MIMER_MAX_MB_LEN){
        return len - pos;
    } else {
        return PDO_MIMER_ENC_UNKNOWN;
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
            return PDO_MIMER_ENC_UNKNOWN;
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

    if (MimerIsBlob(lob_type)) {
        php_stream_seek(stm, 0, SEEK_END);
        nchars = php_stream_tell(stm);
        *tot_size = nchars;
        php_stream_rewind(stm);
        return nchars;

    } else if (MimerIsNclob(lob_type)) {
        return pdo_mimer_cloblen(stm, tot_size);

    } else {
        return PDO_MIMER_UNKNOWN_LOB_TYPE;
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
    MimerReturnCode return_code;
    char *blob_buf = emalloc(MIMER_LOB_IN_CHUNK);

    do {
        /** can't read more data than what is left */
        bytes_left = blob_size - bytes_read;
        chunk_size = bytes_left < MIMER_LOB_IN_CHUNK ? bytes_left : MIMER_LOB_IN_CHUNK;
        bytes_read += php_stream_read(stm, blob_buf, chunk_size);
        if (!MIMER_SUCCEEDED(return_code = MimerSetBlobData(blob_handle, blob_buf, chunk_size)))
            break;
    } while(bytes_read < blob_size);

    efree(blob_buf);
    return return_code;
}

/**
 * @brief Transfers the character data from a stream to a CLOB column in DB.
 *
 * @param[in] stm Stream with character data.
 * @param[in] clob_handle Handle to MimerLob already prepared by call to MimerSetLob.
 * @return 0 if successfully read all data from stream, negative error code otherwise.
 */
static ssize_t pdo_mimer_set_clob_data(php_stream *stm, MimerLob *clob_handle){
    MimerReturnCode return_code;
    ssize_t nbytes_valid;

    char *clob_buf = emalloc(MIMER_LOB_IN_CHUNK);
    while((nbytes_valid = get_valid_stream_chunk(stm, clob_buf, MIMER_LOB_IN_CHUNK)) > 0) {
        if (!MIMER_SUCCEEDED(return_code = MimerSetClobData8(clob_handle, clob_buf, nbytes_valid)))
            break;
    }
    efree(clob_buf);
    return MIMER_SUCCEEDED(return_code) ? nbytes_valid : return_code;
}

/**
 * @brief Transfers the character data from a stream to a NCLOB column in DB.
 *
 * @param[in] stm Stream with character data.
 * @param[in] clob_handle Handle to MimerLob already prepared by call to MimerSetLob.
 * @return 0 if successfully read all data from stream, negative error code otherwise.
 */
static ssize_t pdo_mimer_set_nclob_data(php_stream *stm, MimerLob *clob_handle){
    MimerReturnCode return_code;
    ssize_t nbytes_valid;

    char *nclob_buf = emalloc(MIMER_LOB_IN_CHUNK);
    while((nbytes_valid = get_valid_stream_chunk(stm, nclob_buf, MIMER_LOB_IN_CHUNK)) > 0){
        if (!MIMER_SUCCEEDED(return_code = MimerSetNclobData8(clob_handle, nclob_buf, nbytes_valid)))
            break;
    }

    efree(nclob_buf);
    return MIMER_SUCCEEDED(return_code) ? nbytes_valid : return_code;
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
    MimerReturnCode return_code;
    MimerLob lob_handle;
    int32_t lob_type;
    ssize_t lob_len = 0;
    size_t lob_size = 0;
    php_stream *stm = NULL;

    /** make a PHP stream from the zval
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

    zval_ptr_dtor(parameter);

    /** DB column type decides how we interpret and insert data */
    if (!MIMER_SUCCEEDED(return_code = MimerParameterType(MIMER_STMT, paramno)))
        return return_code;

    lob_type = return_code;

    /** lob_len has different meanings for BLOBs and CLOB/NCLOBs */
    lob_len = pdo_mimer_loblen(stm, lob_type, &lob_size);
    if (lob_len == 0) {
        return MimerSetLob(MIMER_STMT, paramno, 0, &lob_handle);
    } else if (lob_len < 0){
        /** TODO: error handling */
        pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = lob_len,
                               "Error while calculating LOB length");
        return return_code;
    }

    /** make space in DB */
    if (!MIMER_SUCCEEDED(return_code = MimerSetLob(MIMER_STMT, paramno, lob_len, &lob_handle)))
        return return_code;

    /* start data transfer */
    if (MimerIsBlob(lob_type)){
        return_code = pdo_mimer_set_blob_data(stm, &lob_handle, lob_size);
    } else if (MimerIsClob(lob_type)){
        return_code = pdo_mimer_set_clob_data(stm, &lob_handle);
    } else if (MimerIsNclob(lob_type)){
        return_code = pdo_mimer_set_nclob_data(stm, &lob_handle);
    } else {
        /** TODO: More precise error info */
        pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = PDO_MIMER_GENERAL_ERROR,
                               "Expected BLOB, CLOB or NCLOB column type");
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
    MimerReturnCode return_code = MIMER_SUCCESS;

    switch (PDO_PARAM_TYPE(param_type)) {
        case PDO_PARAM_NULL:
            return_code = MimerSetNull(MIMER_STMT, paramno);
            break;

        case PDO_PARAM_BOOL:
            return_code = MimerSetBoolean(MIMER_STMT, paramno, Z_TYPE_P(parameter) == IS_TRUE);
            break;

        case PDO_PARAM_INT:
            return_code = MimerSetInt64(MIMER_STMT, paramno, Z_LVAL_P(parameter));
            break;

        case PDO_PARAM_STR:
            if (!MIMER_SUCCEEDED(return_code = MimerParameterType(MIMER_STMT, paramno)))
                break;
            if (MimerIsDouble(return_code)){
                double val = strtod(Z_STRVAL_P(parameter), NULL);
                return_code = MimerSetDouble(MIMER_STMT, paramno, val);
            }
            else if(MimerIsBinary(return_code)){
                char *binstr = Z_STRVAL_P(parameter);
                size_t strlen = Z_STRLEN_P(parameter);
                return_code = MimerSetBinary(MIMER_STMT, paramno, binstr, strlen);
            }

            else 
                return_code = MimerSetString8(MIMER_STMT, paramno, Z_STRVAL_P(parameter));
            break;

        case PDO_PARAM_LOB:
            return_code = pdo_mimer_set_lob_data(stmt, parameter, paramno);
            break;

        /* unimplemented */
#       define UNSUPPORTED(pdo_param) \
        case pdo_param:               \
            pdo_mimer_custom_error(stmt, SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED, \
                                   return_code = PDO_MIMER_FEATURE_NOT_IMPLEMENTED, \
                                   #pdo_param " support is not yet implemented");
            break;

        UNSUPPORTED(PDO_PARAM_INPUT_OUTPUT)
        UNSUPPORTED(PDO_PARAM_STR_NATL)
        UNSUPPORTED(PDO_PARAM_STR_CHAR)
        UNSUPPORTED(PDO_PARAM_STMT)
        default:
            pdo_mimer_custom_error(stmt, SQLSTATE_GENERAL_ERROR, return_code = PDO_MIMER_GENERAL_ERROR,
                                   "Unexpected parameter type");
    }

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
            if (Z_ISREF(param->parameter)) /* bindParam() was used, let's set those params */
                /* if param is not ref, that means bindValue() was used which should have been set in EVT_ALLOC */
                zend_unwrap_reference(&param->parameter);
                return_code = pdo_mimer_stmt_set_params(stmt, &param->parameter, paramno, param->param_type);
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


/**
 * @brief The PHP method <code>mimerAddBatch()</code> extending the <code>PDOStatement</code> class to be able to set
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
        NULL,   /* statement get column data method */
        NULL,   /* next statement rowset method */
        pdo_mimer_cursor_closer,   /* statement cursor closer method */
};

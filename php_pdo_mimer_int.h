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

#ifndef PHP_PDO_MIMER_INT_H
# define PHP_PDO_MIMER_INT_H

#include <mimerapi.h>
#include <mimerrors.h>

extern const pdo_driver_t pdo_mimer_driver;
extern const struct pdo_stmt_methods mimer_stmt_methods;

#define QUOTE(x) #x
#define QUOTE_EX(x) QUOTE(x)

/**
 * @brief Macro function to get a string value since Mimer SQL's C API functions behave similarly
 * @param mimer_func The Mimer API function to call, eg. <code>MimerGetString()</code>
 * @param str The name of the resulting <code>char[]</code> variable
 * @param rc A <code>MimerError</code> variable to store any possible errors
 * @param args Any additional arguments that need to be sent to @p mimer_func
 *
 * @example From <code>mimer_stmt::pdo_mimer_describe_col()</code>:<br><br>
 * @code
 *  int mim_colno = colno + 1;
 *  MimerError return_code;
 *  MimerGetStr(MimerColumnName8, str_buf, return_code, stmt_handle->statement, mim_colno);
 * @endcode
 *
 * @remark <code>char @p str[]</code> is allocated on the stack
 */
#define MimerGetStr(mimer_func, str, rc, ...)        \
    rc = (mimer_func)(__VA_ARGS__, NULL, 0);         \
    char str[(rc) > 0 ? (rc) + 1 : 0];               \
    if ((rc) > 0) {                                  \
        rc = (mimer_func)(__VA_ARGS__, str, rc + 1); \
    }

typedef int32_t MimerError;

typedef struct mimer_error_info_t {
    char *error_msg;
    MimerError mimer_error;
} MimerErrorInfo;

typedef struct pdo_mimer_handle_t {
    MimerSession session;
    int32_t trans_option;
    MimerErrorInfo error_info;
} pdo_mimer_handle;

typedef struct pdo_mimer_stmt_t {
    pdo_mimer_handle *handle;
    MimerStatement statement; 
    MimerErrorInfo error_info; 
    int32_t cursor_type; 
} pdo_mimer_stmt;

typedef struct pdo_data_src_parser data_src_opt;

/**
 * @brief Macro function to get a DSN option's value
 * @param optname The name of the option to get the value from
 * @return A string or NULL
 */
#define optval(optname) data_src_opts[optname##_opt].optval

/**
 * @brief Checks if the statement will yield a result set
 * @param statement A handle returned by <code>MimerBeginStatement[C|8]()</code>, identifying a prepared statement
 */
#define MimerStatementHasResultSet(statement) ((statement) != NULL && MimerColumnCount((statement)) > 0) /* workaround */


enum { /* Define custom driver attributes here */
    MIMER_ATTR_TRANS_OPTION = PDO_ATTR_DRIVER_SPECIFIC
};

/**
 * @brief The driver specific data needed in Mimer LOB streams.
 */
typedef struct pdo_mimer_lob_streamdata_t {
	MimerLob lob_handle;
    int32_t lob_type;
    char eof;
} pdo_mimer_lob_streamdata;

php_stream *pdo_mimer_create_lob_stream(pdo_stmt_t *stmt, int colno, int32_t lobtype);
extern const php_stream_ops pdo_mimer_lob_stream_ops;

#define MIMER_LOB_INSERT_CHUNKSIZE 8192

#endif /* PHP_PDO_MIMER_INT_H */
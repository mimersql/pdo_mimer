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
#include "pdo_mimer_error.h"


extern const pdo_driver_t pdo_mimer_driver;
extern const struct pdo_stmt_methods mimer_stmt_methods;


typedef int32_t MimerReturnCode;
typedef struct pdo_mimer_handle {
    union {
        struct pdo_mimer_dbh {
            struct pdo_mimer_transaction {
                bool started;
                bool read_only;
            } transaction;
        } dbh;

        struct pdo_mimer_stmt {
            struct pdo_mimer_cursor {
                bool open;
                bool scrollable;
            } cursor;
        } stmt;
    };

    MimerErrorInfo error_info;
    union {
        MimerHandle handle;
        MimerSession session;
        MimerStatement statement;
    };
} pdo_mimer_handle;


#define PDO_MIMER_HANDLE(pdo_handle) ((pdo_mimer_handle*)(pdo_handle)->driver_data)
#define MIMER_HANDLE(pdo_handle) PDO_MIMER_HANDLE(pdo_handle)->handle

#define PDO_MIMER_DBH (&(PDO_MIMER_HANDLE(dbh))->dbh)
#define PDO_MIMER_DBH_ERROR (&PDO_MIMER_HANDLE(dbh)->error_info)
#define PDO_MIMER_TRANS_READONLY PDO_MIMER_DBH->transaction.read_only
#define PDO_MIMER_TRANS_TYPE (PDO_MIMER_TRANS_READONLY ? MIMER_TRANS_READONLY : MIMER_TRANS_READWRITE)
#define PDO_MIMER_IN_TRANSACTION PDO_MIMER_DBH->transaction.started
#define MIMER_SESSION PDO_MIMER_HANDLE(dbh)->session
#define NEW_PDO_MIMER_DBH(persistent) \
    pecalloc(1, sizeof(pdo_mimer_handle), persistent)

#define PDO_MIMER_STMT (&(PDO_MIMER_HANDLE(stmt)->stmt))
#define PDO_MIMER_STMT_ERROR (&PDO_MIMER_HANDLE(stmt)->error_info)
#define PDO_MIMER_CURSOR_OPEN PDO_MIMER_STMT->cursor.open
#define PDO_MIMER_SCROLLABLE PDO_MIMER_STMT->cursor.scrollable
#define PDO_MIMER_CURSOR_TYPE (PDO_MIMER_STMT->cursor.scrollable ? MIMER_SCROLLABLE : MIMER_FORWARD_ONLY)
#define MIMER_STMT PDO_MIMER_HANDLE(stmt)->statement
#define NEW_PDO_MIMER_STMT(mimer_stmt, cursor_scrollable) \
    ecalloc(1, sizeof(pdo_mimer_handle));                 \
    MIMER_STMT = mimer_stmt;                              \
    PDO_MIMER_SCROLLABLE = ((cursor_scrollable) == MIMER_SCROLLABLE)


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
    do {if ((rc) > 0) {                              \
        rc = (mimer_func)(__VA_ARGS__, str, rc + 1); \
    }} while (0)

/**
 * @brief Checks if the statement will yield a result set
 * @param statement A handle returned by <code>MimerBeginStatement[C|8]()</code>, identifying a prepared statement
 */
#define MimerStatementHasResultSet(statement) ((statement) != NULL && MimerColumnCount((statement)) > 0) /* workaround */


/**
 * @brief Macro function to get a DSN option's value
 * @param optname The name of the option to get the value from
 * @return A string or NULL
 */


/* Define custom driver attributes here */
typedef enum pdo_mimer_attr {
    MIMER_ATTR_TRANS_OPTION = PDO_ATTR_DRIVER_SPECIFIC,
} pdo_mimer_attr;



////////////////////////////////////////////////
//              LOB-specifics
///////////////////////////////////////////////

#define MIMER_LOB_IN_CHUNK 8192
#define MIMER_MAX_MB_LEN   8 // max N bytes in a multibyte char

/**
 * @brief The driver specific data needed in Mimer LOB streams.
 */
typedef struct pdo_mimer_lob_stream_data_t {
	MimerLob lob_handle;
    int32_t lob_type;
    uint8_t eof;
} pdo_mimer_lob_stream_data;

extern php_stream *pdo_mimer_create_lob_stream(pdo_stmt_t *stmt, int colno, int32_t lob_type);
extern const php_stream_ops pdo_mimer_lob_stream_ops;

#endif /* PHP_PDO_MIMER_INT_H */
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
#define PHP_PDO_MIMER_INT_H

#include <mimerapi.h>
#include <mimerrors.h>
#include "pdo_mimer_error.h"


/* a struct containing data about our driver and is used to register/unregister the driver */
extern const pdo_driver_t pdo_mimer_driver;

/* our statement driver's methods to interface with PDOStatement */
extern const struct pdo_stmt_methods pdo_mimer_stmt_methods;


/********************************************
 *       PDO Mimer driver -specifics        *
 ********************************************/

/* Mimer SQL C API uses int32_t as return codes and returned data, this typedef exists to simply increase readability */
typedef int32_t MimerReturnCode;

/**
 * This struct exists to simplify the retrieving of data owned by the session or statement handle. This was previously
 * two separate structs but since they share many of the same attributes, it was much simpler to create a single
 * "wrapper" to hold the necessary data.
 */
typedef struct pdo_mimer_handle {
    /* either session or statement attributes */
    union {
        /* session (db) attributes */
        struct {
            bool started;
            bool read_only;
        } transaction;

        struct { /* statement attributes */
            bool open;
            bool scrollable;
        } cursor;
    };

    /* These structs are typedef'd to be pointers, and can be swapped for MimerHandle in certain situations */
    union {
        MimerHandle handle;
        MimerSession session;
        MimerStatement statement;
    };

    /* last error code and message */
    MimerErrorInfo error_info;
} pdo_mimer_handle;


/*********************************************
 * Accessor macros for PDO Mimer driver data *
 *********************************************/

/* helper macro to simplify retrieving our driver data and our generic database/statement handle */
#define PDO_MIMER_HANDLE(pdo_handle) ((pdo_mimer_handle*)(pdo_handle)->driver_data)
#define MIMER_HANDLE(pdo_handle) PDO_MIMER_HANDLE(pdo_handle)->handle

/* macros to retrieve our session handle and its attributes in a scope where the PDO "dbh" is accessible */
#define PDO_MIMER_DBH PDO_MIMER_HANDLE(dbh)
#define PDO_MIMER_DBH_ERROR (&PDO_MIMER_DBH->error_info)
#define PDO_MIMER_TRANS_READONLY PDO_MIMER_DBH->transaction.read_only
#define PDO_MIMER_TRANS_TYPE (PDO_MIMER_TRANS_READONLY ? MIMER_TRANS_READONLY : MIMER_TRANS_READWRITE)
#define PDO_MIMER_IN_TRANSACTION PDO_MIMER_DBH->transaction.started
#define MIMER_SESSION PDO_MIMER_DBH->session

/**
 * @brief Allocate our driver's database handle
 * @param persistent [in] If the memory to be allocated should be persistent (usually <code>dbh->is_persistent</code>
 * @return A pointer to the driver's newly allocated database handle
 */
#define NEW_PDO_MIMER_DBH(persistent) \
    pecalloc(1, sizeof(pdo_mimer_handle), persistent)

/* macros to retrieve our statement handle and its attributes in a scope where the PDOStatement "stmt" is accessible */
#define PDO_MIMER_STMT PDO_MIMER_HANDLE(stmt)
#define PDO_MIMER_STMT_ERROR (&PDO_MIMER_STMT->error_info)
#define PDO_MIMER_CURSOR_OPEN PDO_MIMER_STMT->cursor.open
#define PDO_MIMER_SCROLLABLE PDO_MIMER_STMT->cursor.scrollable
#define PDO_MIMER_CURSOR_TYPE (PDO_MIMER_STMT->cursor.scrollable ? MIMER_SCROLLABLE : MIMER_FORWARD_ONLY)
#define MIMER_STMT PDO_MIMER_STMT->statement

/**
 * @brief Allocate our driver's statement handle
 * @param mimer_stmt [in] The <code>MimerStatement</code> used on a successful <code>MimerBeginStatement[C|8]()</code>
 * @param cursor_scrollable [in] True or false if the created @p mimer_stmt was created with a scrollable cursor or not
 * @return A pointer to the driver's newly allocated statement handle
 */
#define NEW_PDO_MIMER_STMT(mimer_stmt, cursor_scrollable)                 \
    ecalloc(1, sizeof(pdo_mimer_handle));                                 \
    do {                                                                  \
        MIMER_STMT = mimer_stmt;                                          \
        PDO_MIMER_SCROLLABLE = ((cursor_scrollable) == MIMER_SCROLLABLE); \
    } while(0)

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
    do { if ((rc) > 0) {                             \
        rc = (mimer_func)(__VA_ARGS__, str, rc + 1); \
    }} while (0)

/**
 * @brief Checks if the statement will yield a result-set
 * @param mimer_stmt [in] A <code>MimerStatement</code>
 * @return true if the given <code>MimerStatement</code> could/will yield a result-set
 * @return false if the given <code>MimerStatement</code> can't/won't yield a result-set
 * @remark A workaround since the Mimer SQL C API does not provide a way to do this, so this function ultimately calls
 * <code>MimerColumnCount()</code> and checks if the column count is larger than zero.
 */
#define MimerStatementHasResultSet(mimer_stmt) ((mimer_stmt) != NULL && MimerColumnCount((mimer_stmt)) > 0)


/* PDOMimer-specific attributes here */
typedef enum pdo_mimer_attr {
    MIMER_ATTR_TRANS_OPTION = PDO_ATTR_DRIVER_SPECIFIC,
} pdo_mimer_attr;


/********************************************
 *              LOB-specifics               *
 ********************************************/

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


/********************************************
 *            Mimer C API Mock              *
 ********************************************/
/* Utilities that at some point could/should be provided by API instead */
#define MimerIsDatetime(n) (abs(n)==MIMER_DATE||abs(n)==MIMER_TIME||abs(n)==MIMER_TIMESTAMP)
#define MimerIsInterval(n) (abs(n)>=MIMER_INTERVAL_YEAR && abs(n) <= MIMER_INTERVAL_MINUTE_TO_SECOND)

#endif /* PHP_PDO_MIMER_INT_H */
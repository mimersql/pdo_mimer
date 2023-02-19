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
   | Authors: Alexander Hedberg <alexander.hedberg@mimer.com>                        |
   +--------------------------------------------------------------------------------+
*/

#ifndef PHP_PDO_MIMER_INT_H
#define PHP_PDO_MIMER_INT_H

#include <mimerapi.h>
#include <mimerrors.h>
#include "pdo_mimer_error.h"


extern const pdo_driver_t pdo_mimer_driver;
extern const struct pdo_stmt_methods pdo_mimer_stmt_methods;


/********************************************
 *       PDO Mimer driver -specifics        *
 ********************************************/

/* Mimer SQL C API uses int32_t as return codes and returned data, this typedef exists to simply increase readability */
typedef int32_t MimerReturnCode;

typedef struct {
	struct {
		bool is_in_transaction:1;
		bool is_read_only:1;
	} transaction;

	struct {
		MimerErrorCode code;
		char *msg;
		char sqlstate[6];
	} error;

	MimerSession session;
} pdo_mimer_dbh;

typedef struct pdo_mimer_stmt_t {
	struct {
		bool is_open:1;
		bool is_scrollable:1;
	} cursor;

	pdo_mimer_dbh *dbh;
	MimerStatement stmt;
} pdo_mimer_stmt;

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
} pdo_mimer_lob_stream_data;

extern php_stream *pdo_mimer_create_lob_stream(pdo_stmt_t *stmt, int colno, int32_t lob_type);
extern const php_stream_ops pdo_mimer_lob_stream_ops;


/********************************************
 *            Mimer C API Mock              *
 ********************************************/
/* Utilities that at some point could/should be provided by API instead */
#define MimerIsDatetime(n) (abs(n)==MIMER_DATE||abs(n)==MIMER_TIME||abs(n)==MIMER_TIMESTAMP)
#define MimerIsInterval(n) (abs(n)>=MIMER_INTERVAL_YEAR && abs(n) <= MIMER_INTERVAL_MINUTE_TO_SECOND)
#define MimerIsUnsigned(n) ((abs(n)<=MIMER_UNSIGNED_INTEGER && abs(n)>=MIMER_T_UNSIGNED_SMALLINT)||abs(n)==MIMER_T_UNSIGNED_BIGINT)

typedef enum MimerParamMode {
    MIMER_PARAM_INPUT = 1,
    MIMER_PARAM_OUTPUT,
    MIMER_PARAM_INPUT_OUTPUT
} MimerParamMode;

#define MimerParamIsOutput(n) (n==MIMER_PARAM_OUTPUT||n==MIMER_PARAM_INPUT_OUTPUT)

#endif /* PHP_PDO_MIMER_INT_H */
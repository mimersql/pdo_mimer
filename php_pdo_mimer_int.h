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

#ifndef PHP_PDO_MIMER_INT_H
# define PHP_PDO_MIMER_INT_H

#include <mimerapi.h>
#include <mimerrors.h>

#define GENERAL_ERROR_SQLSTATE (pdo_error_type*) "HY000"

extern const pdo_driver_t pdo_mimer_driver;

extern int _pdo_mimer_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file, int line);
#define pdo_mimer_error(x) _pdo_mimer_error(x, NULL, __FILE__, __LINE__)

extern bool _pdo_mimer_handle_checker(pdo_dbh_t *dbh, bool check_handle, bool check_session);
#define pdo_mimer_check_pdo_handle(x) _pdo_mimer_handle_checker(x, false, false)
#define pdo_mimer_check_handle(x) _pdo_mimer_handle_checker(x, true, false)
#define pdo_mimer_check_session(x) _pdo_mimer_handle_checker(x, true, true)

extern const struct pdo_stmt_methods mimer_stmt_methods;

typedef struct pdo_mimer_handle_t {
    MimerSession session;
    int32_t last_error;
    int32_t trans_option;
    enum pdo_cursor_type cursor_type;
} pdo_mimer_handle;

typedef struct pdo_mimer_stmt_t {
    pdo_mimer_handle *handle;
    MimerStatement statement;
    zend_string *query;
    MimerStatement statement;
    int32_t last_error;
} pdo_mimer_stmt;

/* TODO: maybe should be moved to mimerapi somewhere? */
enum {
    MIMER_TRANS_STARTED = -14011
};

/**
 * @brief Define custom driver attributes here
 */
enum {
    PDO_MIMER_ATTR_TRANS_OPTION = PDO_ATTR_DRIVER_SPECIFIC
};

#endif /* PHP_PDO_MIMER_INT_H */
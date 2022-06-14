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

extern const struct pdo_stmt_methods mimer_stmt_methods;

typedef struct pdo_mimer_handle_t {
    MimerSession session;
    int32_t last_error;
} pdo_mimer_handle;

typedef struct pdo_mimer_stmt_t {
    pdo_mimer_handle *handle;
    MimerStatement statement;
} pdo_mimer_stmt;

#endif /* PHP_PDO_MIMER_INT_H */
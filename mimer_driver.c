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

#include "php.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_mimer.h"
#include "php_pdo_mimer_int.h"

/* {{{ _pdo_mimer_error */
int _pdo_mimer_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file, int line)
{
    pdo_mimer_handle *handle = (pdo_mimer_handle*)dbh->driver_data;
    pdo_error_type *pdo_err;

    // TODO: add errors for statements

    // TODO: convert Mimer error codes (int32_t) to SQLSTATE (char[6])

}
/* }}} */

/* {{{ pdo_mimer_request_shutdown */
static void pdo_mimer_request_shutdown(pdo_dbh_t *dbh)
{
    /**
     * @note It seems like PDO does some cleaning up itself.
     *
     * Looking at the other provided drivers, none of them do any
     * nullifying of any pointers or freeing of any memory in this function.
     */

    pdo_mimer_handle *handle = (pdo_mimer_handle*)dbh->driver_data;

    if (handle != NULL) {
        int32_t return_code = MimerEndSession(&handle->session);
        if (!MIMER_SUCCEEDED(return_code)) {
            pdo_throw_exception(return_code, "Something went wrong.", GENERAL_ERROR_SQLSTATE);
        }
    }
}
/* }}} */


/**
 * @brief Declare the methods Mimer uses and give them to the PDO driver
 */
static const struct pdo_dbh_methods mimer_methods = { /* {{{ */
        NULL,   /* handle closer method */
        NULL,   /* handle preparer method */
        NULL,   /* handle doer method */
        NULL,   /* handle quoter method */
        NULL,   /* handle begin method */
        NULL,   /* handle commit method */
        NULL,   /* handle rollback method */
        NULL,   /* handle set attribute method */
        NULL,   /* last_id not supported */
        NULL,   /* fetch error method */
        NULL,   /* handle get attribute method */
        NULL,   /* check liveness method */
        NULL,   /* get driver method */
        pdo_mimer_request_shutdown,   /* request shutdown method */
        NULL,   /* in transaction method */
        NULL    /* get gc method */
};

/**
 * @brief This method handles PDO's construction for use with Mimer SQL
 *
 * @example @code $PDO = new PDO("mimer:host=localhost;dbname=test_db"); @endcode
 *
 * @remark DSN: <a href="https://www.php.net/manual/en/pdo.construct.php">Data Source Name</a>
 */
static int pdo_mimer_handle_factory(pdo_dbh_t *dbh, zval *driver_options) /* {{{ */
{
    /* This enum exists to add code readability */
//    typedef enum { protocol_opt, dbname_opt, host_opt, port_opt, ident_opt, /* user_opt, */ password_opt } opt_val; // TODO: possible future functionality
    typedef enum { dbname_opt } DataSourceOption;

    int num_data_src_opts;
    struct pdo_data_src_parser data_src_opts[] = {
            /* if the user does not give database name, NULL will trigger default database connection */
            {"dbname", NULL, 0 },

            /**
             * TODO: possible future functionality
             * { "protocol", PDO_MIMER_DEFAULT_PROTOCOL, 0},
             * { "host", PDO_MIMER_DEFAULT_HOST, 0 },
             * { "port", QUOTE(PDO_MIMER_DEFAULT_PORT), 0},
             * {"ident", PDO_MIMER_DEFAULT_IDENT, 0 }, // Mimer terminology
             * {"user", PDO_MIMER_DEFAULT_IDENT, 0 }, // PDO standard?
             */
    };

    num_data_src_opts = sizeof(*data_src_opts) / sizeof(void*);

    pdo_mimer_handle *handle = pecalloc(1, sizeof(pdo_mimer_handle), dbh->is_persistent);
    handle->error = 0;
    dbh->driver_data = handle;

    /**
     * Parsing the DSN
     *
     * This function, provided by php-src/ext/pdo/pdo.c, parses the user-provided DSN
     * during the construction of the PDO and copies each value (optval) to the respective key (optname).
     */
    php_pdo_parse_data_source(dbh->data_source, dbh->data_source_len, data_src_opts, num_data_src_opts);


    MimerSession session = NULL;

    /* TODO: add compatability for MimerBeginSession() and MimerBeginSessionC() */
    int32_t return_code = MimerBeginSession8(data_src_opts[dbname_opt].optval, dbh->username, dbh->password, &session);

    /* TODO: add more error codes and error information */
    if (return_code < 0 || session == NULL) {
        php_printf("Mimer error code: %d\n", return_code);
        pdo_throw_exception(return_code, "Something went wrong", GENERAL_ERROR_SQLSTATE);
    }

    if (MIMER_SUCCEEDED(return_code)) {
        php_printf("Connected to Mimer SQL (db=%s, user=%s, pwd=%s)\n", data_src_opts[dbname_opt].optval, dbh->username, dbh->password);
    }


    /* free up memory no longer needed */
    for (int i = 0; i < num_data_src_opts; i++) {
        if (data_src_opts[i].freeme) {  // check if each option is persistent
            efree(data_src_opts[i].optval);
        }
    }

    /* pass the methods the Mimer PDO driver uses to PDO */
    dbh->methods = &mimer_methods;
}

const pdo_driver_t pdo_mimer_driver = {
        PDO_DRIVER_HEADER(mimer),
        pdo_mimer_handle_factory
};
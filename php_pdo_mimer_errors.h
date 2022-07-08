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

#ifndef PHP_SRC_PHP_PDO_MIMER_ERRORS_H
#define PHP_SRC_PHP_PDO_MIMER_ERRORS_H

#include <mimerapi.h>
#include <mimerrors.h>

/* the functions to call upon any error */
extern int _pdo_mimer_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file, int line);
#define pdo_mimer_error(dbh) _pdo_mimer_error(dbh, NULL, __FILE__, __LINE__)
#define pdo_mimer_stmt_error(stmt) _pdo_mimer_error((stmt)->dbh, stmt, __FILE__, __LINE__)

/**
 * @brief Macro function to handle a possible error which executes `fail_action` upon an error
 * @param return_code The return code to check against for errors
 * @param fail_action The action to take upon any error
 * @param ... Any extra statement to execute after the error function is called (eg. `return`)
 * @example @code handle_err(MimerExecute(statement), some_func(some_param), return -1) @endcode
 *      The output of `MimerExecute()` will be checked with `MIMER_SUCCEEDED()`. If evaluated to `false`,
 *      `some_func(some_param)` will be called and exits returning the value `-1`.
 * @example @code handle_err(MimerError mimer_error = MimerExecute(statement), pdo_mimer_error(dbh), return false) @endcode
 *      The output of `MimerExecute` will be saved to the variable `mimer_error` and is then checked with
 *      `MIMER_SUCCEEDED()`. If evaluated to `false`, `pdo_mimer_error()` is called and `false` is returned.
 * @see `handle_err_dbh()`
 * @see `handle_err_stmt()`
 * @see `return_on_err()`
 * @see `return_on_err_stmt()`
 * @remark The purpose of these macro functions is to make it easier to handle MimerAPI errors.
 */
#define handle_err(return_code, fail_action, ...) \
    if (!MIMER_SUCCEEDED((return_code))) {        \
        fail_action;                              \
        __VA_ARGS__;                              \
    }

/**
 * @brief Macro function to check a return code for errors that expands `pdo_mimer_error(dbh)` upon failure.
 * @param return_code The return code from a `MimerAPI` function call to check
 * @remark Expands macro function `handle_err()`
 * @see `handle_err_stmt()`
 */
#define handle_err_dbh(return_code) \
    handle_err(return_code, pdo_mimer_error(dbh))

/**
 * @brief Macro function to check a return code for errors that expands `pdo_mimer_stmt_error(stmt)` upon failure.
 * @param return_code The return code from a `MimerAPI` function call to check
 * @remark Expands macro function `handle_err()`
 * @see `handle_err_dbh()`
 */
#define handle_err_stmt(return_code) \
    handle_err(return_code, pdo_mimer_stmt_error(stmt))

/**
 * @brief MMacro function to handle an error that expands `pdo_mimer_error(dbh)` upon error and returns
 *      the given return value.
 * @param return_code The return code from a `MimerAPI` function call to check
 * @param return_value The value to be returned upon failure.
 * @remark Expands macro function `handle_err()`
 * @see `return_on_err_stmt()`
 */
#define return_on_err(return_code, return_value) \
    handle_err(return_code, pdo_mimer_error(dbh), return (return_value))

/**
 * @brief Macro function to handle an error that expands `pdo_mimer_stmt_error(stmt)`
 *      upon error and returns the given return value.
 * @param return_code The return code from a `MimerAPI` function call to check
 * @param return_value The value to be returned upon failure.
 * @remark Expands macro function `handle_err()`
 * @see `return_on_err()`
 */
#define return_on_err_stmt(return_code, return_value) \
    handle_err(return_code, pdo_mimer_stmt_error(stmt), return (return_value))

/**
 * @brief Macro function to customize an error message
 * @param mimer_err_info_p A pointer to either the `pdo_mimer_handle` or `pdo_mimer_stmt`
 * @param err_msg A literal string of the error description
 * @param mim_errcode A native or custom MimerSQL error code
 * @param sqlstate A literal string with the SQLSTATE to be used by PDO
 * @param persistence A boolean value detailing if the error message should be persistent-allocated (`dbh->is_persistent)`
 * @param pdo_handle_errcode_p The pointer to the PDO database or statement handle object struct value `error_code`
 *      (`dbh->error_code` or `stmt->error_code`)
 * @see mimer_throw_except()
 */
#define handle_custom_err(mimer_err_info_p, err_msg, mim_errcode, sqlstate, persistence, pdo_handle_errcode_p) \
    (mimer_err_info_p)->error_msg ?: pefree((mimer_err_info_p)->error_msg, persistence);                       \
    (mimer_err_info_p)->error_msg = pestrdup(err_msg, persistence);                                            \
    (mimer_err_info_p)->mimer_error = mim_errcode;                                                             \
    strcpy(pdo_handle_errcode_p, sqlstate);

/**
 * @brief Macro function to customize an error message and then throw an exception
 * @param mimer_err_info_p A pointer to either the `pdo_mimer_handle` or `pdo_mimer_stmt`
 * @param err_msg A literal string of the error description
 * @param mim_errcode A native or custom MimerSQL error code
 * @param sqlstate A literal string with the SQLSTATE to be used by PDO
 * @param persistence A boolean value detailing if the error message should be persistent-allocated (`dbh->is_persistent)`
 * @param pdo_handle_errcode_p The pointer to the PDO database or statement handle object struct value `error_code`
 * @remark Expands handle_custom_err() and calls pdo_throw_exception()
 */
#define mimer_throw_except(mimer_err_info_p, err_msg, mim_errcode, sqlstate, persistence, pdo_handle_errcode_p) \
    handle_custom_err(mimer_err_info_p, err_msg, mim_errcode, sqlstate, persistence, pdo_handle_errcode_p)      \
    pdo_throw_exception(mim_errcode, err_msg, (pdo_error_type*)(sqlstate));

/*******************************************************************************
 *                              SQLSTATE CODES                                 *
 *******************************************************************************/

#define SQLSTATE_SUCCESSFUL_COMPLETION "00000"
#define SQLSTATE_WARNING "01000"
#define SQLSTATE_DISCONNECT_ERROR "01002"
#define SQLSTATE_NULL_VALUE_ELIMINATED_IN_SET_FUNCTION "01003"
#define SQLSTATE_STRING_DATA_RIGHT_TRUNCATION "01004"
#define SQLSTATE_INSUFFICIENT_ITEM_DESCRIPTOR_AREAS "01005"
#define SQLSTATE_PRIVILEGE_NOT_REVOKED "01006"
#define SQLSTATE_PRIVILEGE_NOT_GRANTED "01007"
#define SQLSTATE_RESULT_SETS_RETURNED "0100C"
#define SQLSTATE_IMPLICIT_ZERO_BIT_PADDING "01008"
#define SQLSTATE_SOME_STATEMENT_NOT_ALTERED "01997"
#define SQLSTATE_ROW_HAS_BEEN_UPDATED "01998"
#define SQLSTATE_ROW_HAS_BEEN_DELETED "01999"
#define SQLSTATE_ERROR_IN_ROW "01S01"
#define SQLSTATE_OPTION_VALUE_CHANGED "01S02"
#define SQLSTATE_CANCEL_TREATED_AS_CLOSE "01S05"
#define SQLSTATE_ATTEMPT_TO_FETCH_BEFORE_THE_RESULT_SET_RETURNED_THE_FIRST_ROWSET "01S06"
#define SQLSTATE_FRACTIONAL_TRUNCATION "01S07"
#define SQLSTATE_NO_DATA "02000"
#define SQLSTATE_DYNAMIC_SQL_ERROR "07000"
#define SQLSTATE_USING_CLAUSE_DOES_NOT_MATCH_DYNAMIC_PARAMETER_SPECIFICATIONS "07001"
#define SQLSTATE_USING_CLAUSE_DOES_NOT_MATCH_TARGET_SPECIFICATIONS "07002"
#define SQLSTATE_CURSOR_SPECIFICATION_CANNOT_BE_EXECUTED "07003"
#define SQLSTATE_USING_CLAUSE_REQUIRED_FOR_DYNAMIC_PARAMETERS "07004"
#define SQLSTATE_PREPARED_STATEMENT_IS_NOT_A_CURSOR_SPECIFICATION "07005"
#define SQLSTATE_RESTRICTED_DATA_TYPE_ATTRIBUTE_VIOLATION "07006"
#define SQLSTATE_USING_CLAUSE_REQUIRED_FOR_RESULT_FIELDS "07007"
#define SQLSTATE_INVALID_DESCRIPTOR_COUNT "07008"
#define SQLSTATE_INVALID_DESCRIPTOR_INDEX "07009"
#define SQLSTATE_INVALID_DATETIME_INTERVAL_CODE "0700F"
#define SQLSTATE_CONNECTION_EXCEPTION "08000"
#define SQLSTATE_CLIENT_UNABLE_TO_ESTABLISH_CONNECTION "08001"
#define SQLSTATE_CONNECTION_NAME_IN_USE "08002"
#define SQLSTATE_CONNECTION_DOES_NOT_EXIST "08003"
#define SQLSTATE_SERVER_REJECTED_THE_CONNECTION "08004"
#define SQLSTATE_CONNECTION_FAILURE "08006"
#define SQLSTATE_COMMUNICATION_LINK_FAILURE "08S01"
#define SQLSTATE_TRIGGERED_ACTION_EXCEPTION "09000"
#define SQLSTATE_FEATURE_NOT_SUPPORTED "0A000"
#define SQLSTATE_INVALID_TRANSACTION_INITIATION "0B000"
#define SQLSTATE_RESIGNAL_WHEN_HANDLER_NOT_ACTIVE "0K000"
#define SQLSTATE_PROHIBITED_STATEMENT_ENCOUNTERED_DURING_TRIGGER_EXECUTION "0W000"
#define SQLSTATE_CARDINALITY_VIOLATION "21000"
#define SQLSTATE_INSERT_VALUE_LIST_DOES_NOT_MATCH_COLUMN_LIST "21S01"
#define SQLSTATE_DEGREE_OF_DERIVED_TABLE_DOES_NOT_MATCH_COLUMN_LIST "21S02"
#define SQLSTATE_DATA_EXCEPTION "22000"
#define SQLSTATE_STRING_DATA_RIGHT_TRUNCATION_DATA_EXCEPTION "22001"
#define SQLSTATE_NULL_VALUE_NO_INDICATOR_PARAMETER "22002"
#define SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE "22003"
#define SQLSTATE_ERROR_IN_ASSIGNMENT "22005"
#define SQLSTATE_INVALID_INTERVAL_FORMAT "22006"
#define SQLSTATE_INVALID_DATETIME_FORMAT "22007"
#define SQLSTATE_DATETIME_FIELD_OVERFLOW "22008"
#define SQLSTATE_SUBSTRING_ERROR "22011"
#define SQLSTATE_DIVISION_BY_ZERO "22012"
#define SQLSTATE_INTERVAL_FIELD_OVERFLOW "22015"
#define SQLSTATE_INVALID_CHARACTER_VALUE_FOR_CAST "22018"
#define SQLSTATE_INVALID_ESCAPE_CHARACTER "22019"
#define SQLSTATE_INVALID_PARAMETER_VALUE "22023"
#define SQLSTATE_UNTERMINATED_C_STRING "22024"
#define SQLSTATE_INVALID_ESCAPE_SEQUENCE "22025"
#define SQLSTATE_STRING_DATA_LENGTH_MISMATCH "22026"
#define SQLSTATE_TRIM_ERROR "22027"
#define SQLSTATE_NONCHARACTER_IN_UCS_STRING "22029"
#define SQLSTATE_INTEGRITY_CONSTRAINT_VIOLATION "23000"
#define SQLSTATE_INVALID_CURSOR_STATE "24000"
#define SQLSTATE_INVALID_TRANSACTION_STATE "25000"
#define SQLSTATE_TRANSACTION_IS_ROLLED_BACK "25S03"
#define SQLSTATE_INVALID_SQL_STATEMENT_NAME "26000"
#define SQLSTATE_TRIGGERED_DATA_CHANGE_VIOLATION "27000"
#define SQLSTATE_INVALID_AUTHORIZATION_SPECIFICATION "28000"
#define SQLSTATE_INVALID_CONNECTION_NAME "2E000"
#define SQLSTATE_SQL_ROUTINE_EXCEPTION "2F000"
#define SQLSTATE_PROHIBITED_SQL_STATEMENT_ATTEMPTED "2F003"
#define SQLSTATE_FUNCTION_EXECUTED_NO_RETURN_STATEMENT "2F005"
#define SQLSTATE_INVALID_SQL_DESCRIPTOR_NAME "33000"
#define SQLSTATE_INVALID_CURSOR_NAME "34000"
#define SQLSTATE_INVALID_CONDITION_NUMBER "35000"
#define SQLSTATE_SYNTAX_ERROR_OR_ACCESS_VIOLATION_IN_PREPARE_OR_EXECUTE_IMMEDIATE "37000"
#define SQLSTATE_AMBIGUOUS_CURSOR_NAME "3C000"
#define SQLSTATE_TRANSACTION_ROLLBACK "40000"
#define SQLSTATE_SERIALIZATION_FAILURE "40001"
#define SQLSTATE_SYNTAX_ERROR_OR_ACCESS_RULE_VIOLATION "42000"
#define SQLSTATE_BASE_TABLE_OR_VIEW_ALREADY_EXISTS "42S01"
#define SQLSTATE_BASE_TABLE_OR_VIEW_NOT_FOUND "42S02"
#define SQLSTATE_INDEX_ALREADY_EXISTS "42S11"
#define SQLSTATE_COLUMN_ALREADY_EXISTS "42S21"
#define SQLSTATE_COLUMN_NOT_FOUND "42S22"
#define SQLSTATE_WITH_CHECK_OPTION_VIOLATION "44000"
#define SQLSTATE_UNHANDLED_USER_DEFINED_EXCEPTION "45000"
#define SQLSTATE_INTERNAL_ERROR "5000B"
#define SQLSTATE_GENERAL_ERROR "HY000"
#define SQLSTATE_MEMORY_ALLOCATION_ERROR "HY001"
#define SQLSTATE_INVALID_APPLICATION_BUFFER_TYPE "HY003"
#define SQLSTATE_INVALID_SQL_DATA_TYPE "HY004"
#define SQLSTATE_ASSOCIATED_STATEMENT_IS_NOT_PREPARED "HY007"
#define SQLSTATE_OPERATION_CANCELED "HY008"
#define SQLSTATE_INVALID_USE_OF_NULL_POINTER "HY009"
#define SQLSTATE_FUNCTION_SEQUENCE_ERROR "HY010"
#define SQLSTATE_ATTRIBUTE_CANNOT_BE_SET_NOW "HY011"
#define SQLSTATE_INVALID_TRANSACTION_OPERATION_CODE "HY012"
#define SQLSTATE_MEMORY_MANAGEMENT_ERROR "HY013"
#define SQLSTATE_LIMIT_ON_THE_NUMBER_OF_HANDLES_EXCEEDED "HY014"
#define SQLSTATE_NO_CURSOR_NAME_AVAILABLE "HY015"
#define SQLSTATE_CANNOT_MODIFY_AN_IMPLEMENTATION_ROW_DESCRIPTOR "HY016"
#define SQLSTATE_INVALID_USE_OF_AN_AUTOMATICALLY_ALLOCATED_DESCRIPTOR_HANDLE "HY017"
#define SQLSTATE_SERVER_DECLINED_CANCEL_REQUEST_SQLCANCEL "HY018"
#define SQLSTATE_NON_CHARACTER_AND_NON_BINARY_DATA_SENT_IN_PIECES "HY019"
#define SQLSTATE_ATTEMPT_TO_CONCATENATE_A_NULL_VALUE "HY020"
#define SQLSTATE_INCONSISTENT_DESCRIPTOR_INFORMATION "HY021"
#define SQLSTATE_INVALID_ATTRIBUTE_VALUE "HY024"
#define SQLSTATE_INVALID_STRING_OR_BUFFER_LENGTH "HY090"
#define SQLSTATE_INVALID_DESCRIPTOR_FIELD_IDENTIFIER "HY091"
#define SQLSTATE_INVALID_ATTRIBUTE_OPTION_IDENTIFIER "HY092"
#define SQLSTATE_INVALID_PARAMETER_NUMBER "HY093"
#define SQLSTATE_FUNCTION_TYPE_OUT_OF_RANGE "HY095"
#define SQLSTATE_INVALID_INFORMATION_TYPE "HY096"
#define SQLSTATE_COLUMN_TYPE_OUT_OF_RANGE "HY097"
#define SQLSTATE_SCOPE_TYPE_OUT_OF_RANGE "HY098"
#define SQLSTATE_NULLABLE_TYPE_OUT_OF_RANGE "HY099"
#define SQLSTATE_UNIQUENESS_OPTION_TYPE_OUT_OF_RANGE "HY100"
#define SQLSTATE_ACCURACY_OPTION_TYPE_OUT_OF_RANGE "HY101"
#define SQLSTATE_TABLE_TYPE_OUT_OF_RANGE "HY102"
#define SQLSTATE_INVALID_RETRIEVAL_CODE "HY103"
#define SQLSTATE_INVALID_PRECISION_OR_SCALE_VALUE "HY104"
#define SQLSTATE_INVALID_PARAMETER_TYPE "HY105"
#define SQLSTATE_FETCH_TYPE_OUT_OF_RANGE "HY106"
#define SQLSTATE_ROW_VALUE_OUT_OF_RANGE "HY107"
#define SQLSTATE_CONCURRENCY_OPTION_OUT_OF_RANGE "HY108"
#define SQLSTATE_INVALID_CURSOR_POSITION "HY109"
#define SQLSTATE_INVALID_DRIVER_COMPLETION "HY110"
#define SQLSTATE_INVALID_BOOKMARK_VALUE "HY111"
#define SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED "HYC00"
#define SQLSTATE_TIMEOUT_EXPIRED "HYT00"
#define SQLSTATE_CONNECTION_TIMEOUT_EXPIRED "HYT01"

/* TODO: pdo_error mappings to native Mimer SQL error codes */
#define MimerGetSQLState(mimer_error) SQLSTATE_GENERAL_ERROR


/*******************************************************************************
 *                       EXTRA MIMER SQL CODES                                 *
 *******************************************************************************/


#define IS_CUSTOM_MIMER_ERROR(err) ((err) < CUSTOM_MIMER_ERROR)

#define MIMER_NO_ERROR                0
#define MIMER_LOGIN_FAILED            (-14006)
#define CUSTOM_MIMER_ERROR            (-100000)
#define MIMER_FEATURE_NOT_IMPLEMENTED (-100001)
#define MIMER_VALUE_TOO_LARGE         (-100002)

#endif //PHP_SRC_PHP_PDO_MIMER_ERRORS_H

/**
* @file ep-odbc.h
* @brief odbc utility library 
*
* @date Sat Jul  8 12:19:09 PM CEST 2023
* @author Roberto Mantovani
* @copyright A&L srl 2003-2023
*/
#pragma once

typedef struct _odbc_column_t {
    char *name;
    int type;
    unsigned long size;
    int decimal_digits;
    int nullable;
    unsigned int flen;
} odbc_column_t;

typedef struct _odbc_result_t {
    SQLHSTMT odbc_stmt;
    SQLRETURN odbc_rc;
    int n_columns;
    long n_rows;
    odbc_column_t **columns;
    char ***data;
} odbc_result_t;

typedef enum  _odbc_exec_status_type_t {
	ODBC_EMPTY_QUERY = 0,
	ODBC_COMMAND_OK,
	ODBC_TUPLES_OK,
	ODBC_COPY_OUT,
	ODBC_COPY_IN,
	ODBC_BAD_RESPONSE,
	ODBC_NONFATAL_ERROR,
	ODBC_FATAL_ERROR
} odbc_exec_status_type_t;

typedef enum _odbc_field_type_t {
	ODBC_TYPE_NONE,
	ODBC_TYPE_STRING,
	ODBC_TYPE_INTEGER,
	ODBC_TYPE_FLOAT,
	ODBC_TYPE_DATE,
	ODBC_TYPE_TIME,
	ODBC_TYPE_DATETIME,
	ODBC_TYPE_TIMESTAMP,
	ODBC_TYPE_YEAR
} odbc_field_type_t;

#define SQL_SUCCESFULL(rc) (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)


bool odbc_connect(char *dns,char *user,char *pwd);                                                     
bool odbc_disconnect(void);
bool odbc_is_connection_active(void);
odbc_result_t *odbc_execute_stmt(bool trace,char *format, ...) __attribute__ ((format(printf,2,3)));
void odbc_free_result(odbc_result_t *odbc_res);
unsigned long odbc_get_ntuples(odbc_result_t *odbc_res);
unsigned long odbc_get_cmd_ntuples(odbc_result_t *odbc_res);
unsigned int odbc_get_nfields(odbc_result_t *odbc_res);
unsigned long odbc_get_fsize(odbc_result_t *odbc_res, unsigned int field);
char *odbc_get_fname(odbc_result_t *odbc_res, unsigned int field);
unsigned int odbc_get_fnumber(odbc_result_t *odbc_res,char *field_name);
char *odbc_get_fvalue(odbc_result_t *odbc_res,unsigned long tuple,unsigned int field);
unsigned int odbc_get_flen(odbc_result_t *odbc_res,unsigned int field);

/*
odbc_exec_status_type_t odbc_result_status(odbc_result_t *odbc_res);
char *odbc_result_error_message(odbc_result_t *odbc_res);
odbc_field_type_t odbc_get_ftype(odbc_result_t *odbc_res,int field);
*/

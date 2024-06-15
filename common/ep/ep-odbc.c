/**
* @file ep-odbc.c
* @brief odbc functions
* 
* @date Sat Jul 8 19:10:07 CEST 2023
* @author Roberto Mantovani
* @copyright A&L srl 2003-2023
*/
#ifdef __linux__
#include <glib.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include <trace.h>

#include <sql.h>
#include <sqlext.h>

#include <ep-common.h>
#include <ep-odbc.h>

static HENV odbc_env=NULL;
static HDBC odbc_dbc=NULL;
static bool odbc_is_connected=false;

/**
 * @brief 
 * 
 * @param dns 
 * @param user 
 * @param pwd 
 * @return true 
 * @return false 
 */
bool odbc_connect(char *dns,char *user,char *pwd)
{
    SQLRETURN rc;
    bool return_value = false;

    /* Allocate environment handle */
    
    if (SQL_SUCCEEDED(rc = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &odbc_env))) {
        /*  Set the ODBC version environment attribute */
        if (SQL_SUCCEEDED(rc = SQLSetEnvAttr (odbc_env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0))) {
            // Allocate connection handle 
            if (SQL_SUCCEEDED(rc = SQLAllocHandle (SQL_HANDLE_DBC, odbc_env, &odbc_dbc))) {

                /* Set login timeout to 5 seconds. */
                SQLSetConnectAttr (odbc_dbc, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER) 5, 0);

                // Connect to data source
                if (SQL_SUCCEEDED(rc = SQLConnect (odbc_dbc, (SQLCHAR *)dns, SQL_NTS, (SQLCHAR *)user, SQL_NTS, (SQLCHAR *)pwd, SQL_NTS))) {
                    odbc_is_connected = true;
                    return_value = true;
                } else {
                    /* maybe not needed */
                    SQLDisconnect (odbc_dbc);
                }
            } else {
                SQLFreeHandle (SQL_HANDLE_DBC, odbc_dbc);
            }
        }
    }

    if (!return_value){
        SQLFreeHandle (SQL_HANDLE_ENV, odbc_env);
        odbc_dbc = NULL;
        odbc_env = NULL;
    }

    return return_value;
}

/**
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool odbc_disconnect(void)
{
    if (odbc_dbc) {
        SQLDisconnect (odbc_dbc);
        SQLFreeHandle (SQL_HANDLE_DBC, odbc_dbc);
    }
    if (odbc_env) {
        SQLFreeHandle (SQL_HANDLE_ENV, odbc_env);
    }

    odbc_dbc = NULL;
    odbc_env = NULL;
    odbc_is_connected = false;

    return true;
}

/**
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool odbc_is_connection_active(void)
{
    return odbc_is_connected;
}

/**
 * @brief 
 * 
 * @return odbc_result_t* 
 */
static odbc_result_t *_odbc_allocate_res(void)
{
    odbc_result_t *p;

    if((p = malloc(sizeof(odbc_result_t)))){
        memset(p,0,sizeof(odbc_result_t));
    }
    return p;
}

/**
 * @brief 
 * 
 * @return int 
 */
unsigned int _odbc_fetch_data (odbc_result_t *odbc_res)
{
    char field_name[128];
    SQLSMALLINT name_length=0;
    SQLSMALLINT data_type=0; 
    SQLULEN column_size=0;
    SQLSMALLINT decimal_digits=0;
    SQLSMALLINT nullable=0;
    SQLLEN rows=0;
    SQLSMALLINT cols=0;
    char **tmp_buf;
    SQLLEN *tmp_len;
    int test_field=13;

    if (odbc_dbc == NULL || odbc_env == NULL || odbc_res->odbc_stmt == NULL) {
        return 0;
    }

    odbc_res->odbc_rc = SQLRowCount(odbc_res->odbc_stmt, &rows);
    odbc_res->odbc_rc = SQLNumResultCols(odbc_res->odbc_stmt, &cols);
    odbc_res->n_columns = cols;
    odbc_res->n_rows = rows;

    if(odbc_res->n_columns > 0 && odbc_res->n_rows > 0 ) {

        odbc_res->columns = malloc(cols*sizeof(void *));
        if((odbc_res->data = malloc(rows*sizeof(void *)))) memset(odbc_res->data,0,rows*sizeof(void *));

        tmp_buf = malloc(cols*sizeof(void *));
        if ((tmp_len = malloc(cols*sizeof(SQLLEN)))) { memset(tmp_len, 0, cols*sizeof(SQLLEN)); }
        for (int i = 0; i < cols; i++) {
            /* column number starting at 1 */
            odbc_res->odbc_rc = SQLDescribeCol(odbc_res->odbc_stmt, (SQLSMALLINT)i+1, (SQLCHAR *)field_name, (SQLSMALLINT)sizeof(field_name), 
                &name_length, 
                &data_type, 
                &column_size, 
                &decimal_digits, 
                &nullable);

            odbc_res->columns[i] = (odbc_column_t *)malloc(sizeof(odbc_column_t));
            odbc_res->columns[i]->name = strdup(field_name);
            odbc_res->columns[i]->type = data_type;
            odbc_res->columns[i]->size = column_size;
            odbc_res->columns[i]->decimal_digits = decimal_digits;
            odbc_res->columns[i]->nullable = nullable;
            odbc_res->columns[i]->flen = strlen(field_name);
            
            if((tmp_buf[i] = malloc(128+1))) {
                memset(tmp_buf[i], 0, 128 +1);
            }

            tmp_len[i]=0;
            if(!SQL_SUCCEEDED(odbc_res->odbc_rc = SQLBindCol( odbc_res->odbc_stmt, i + 1, SQL_C_CHAR, tmp_buf[ i ], 128, (SQLLEN *)&tmp_len[ i ] ))){
                printf("SQLBindCol -> %d\n", odbc_res->odbc_rc);
            }
        }
        /* Fetch the data */
        int row_index=0;
        while (SQL_SUCCEEDED(odbc_res->odbc_rc = SQLFetch(odbc_res->odbc_stmt))) {

            if((odbc_res->data[row_index] = malloc(cols*sizeof(void *)))) {memset(odbc_res->data[row_index],0,cols*sizeof(void *));}

            for ( int i = 0; i < cols; i ++ ) {
                if (tmp_len[i] == SQL_NULL_DATA) {
                    odbc_res->data[row_index][i] = strdup("NULL");
                } else {
                    odbc_res->data[row_index][i] = strdup(tmp_buf[i]);
                }
                odbc_res->columns[i]->flen = MAX(odbc_res->columns[i]->flen, strlen(odbc_res->data[row_index][i]));

            }

            row_index++;
        }

        for (int i = 0; i < cols; i++) {
            free(tmp_buf[i]);
        }
        free(tmp_buf);
        free(tmp_len);

    } else {
        odbc_res->columns = NULL;
        odbc_res->data = NULL;
    }

    return rows;
}


/**
 * @brief 
 * 
 * @return int 
 */
unsigned int _odbc_fetch_data_oracle (odbc_result_t *odbc_res)
{
    char field_name[128];
    SQLSMALLINT name_length=0;
    SQLSMALLINT data_type=0; 
    SQLULEN column_size=0;
    SQLSMALLINT decimal_digits=0;
    SQLSMALLINT nullable=0;
    SQLLEN rows=0;
    SQLSMALLINT cols=0;
    char **tmp_buf;
    SQLLEN *tmp_len;
    int test_field=13;

    if (odbc_dbc == NULL || odbc_env == NULL || odbc_res->odbc_stmt == NULL) {
        return 0;
    }

    odbc_res->odbc_rc = SQLRowCount(odbc_res->odbc_stmt, &rows);
    odbc_res->odbc_rc = SQLNumResultCols(odbc_res->odbc_stmt, &cols);
    odbc_res->n_columns = cols;
    odbc_res->n_rows = rows;

    if(odbc_res->n_columns > 0 && (odbc_res->n_rows > 0 || odbc_res->n_rows == -1) ) {

        odbc_res->columns = malloc(cols*sizeof(void *));

        tmp_buf = malloc(cols*sizeof(void *));
        if ((tmp_len = malloc(cols*sizeof(SQLLEN)))) { memset(tmp_len, 0, cols*sizeof(SQLLEN)); }
        for (int i = 0; i < cols; i++) {
            /* column number starting at 1 */
            odbc_res->odbc_rc = SQLDescribeCol(odbc_res->odbc_stmt, (SQLSMALLINT)i+1, (SQLCHAR *)field_name, (SQLSMALLINT)sizeof(field_name), 
                &name_length, 
                &data_type, 
                &column_size, 
                &decimal_digits, 
                &nullable);

            odbc_res->columns[i] = (odbc_column_t *)malloc(sizeof(odbc_column_t));
            odbc_res->columns[i]->name = strdup(field_name);
            odbc_res->columns[i]->type = data_type;
            odbc_res->columns[i]->size = column_size;
            odbc_res->columns[i]->decimal_digits = decimal_digits;
            odbc_res->columns[i]->nullable = nullable;
            odbc_res->columns[i]->flen = strlen(field_name);
            
            if((tmp_buf[i] = malloc(128+1))) {
                memset(tmp_buf[i], 0, 128 +1);
            }

            tmp_len[i]=0;
            if(!SQL_SUCCEEDED(odbc_res->odbc_rc = SQLBindCol( odbc_res->odbc_stmt, i + 1, SQL_C_CHAR, tmp_buf[ i ], 128, (SQLLEN *)&tmp_len[ i ] ))){
                printf("SQLBindCol -> %d\n", odbc_res->odbc_rc);
            }
        }


        /* Fetch the data */
        int row_index=0;
        while (SQL_SUCCEEDED(odbc_res->odbc_rc = SQLFetch(odbc_res->odbc_stmt))) {
            char **row;
            /* alloc data */
            if(odbc_res->data == NULL) {
                /* allocate the first line */
                if((odbc_res->data = malloc((row_index+1)*sizeof(void *)))) { memset(odbc_res->data,0,(row_index+1)*sizeof(void *)); }
            } else {
                /* allocate for one more line */
                if((odbc_res->data = realloc(odbc_res->data, (row_index+1)*sizeof(void *)))) { /* memset(odbc_res->data+(row_index)*sizeof(void *),0,sizeof(void *));*/ }
            }

            if((row = malloc(cols*sizeof(void *)))) { memset(row,0,cols*sizeof(void *)); }

            for ( int i = 0; i < cols; i ++ ) {
                if (tmp_len[i] == SQL_NULL_DATA) {
                    row[i] = strdup("NULL");
                } else {
                    row[i] = strdup(tmp_buf[i]);
                }
                odbc_res->columns[i]->flen = MAX(odbc_res->columns[i]->flen, strlen(row[i]));
            }

            odbc_res->data[row_index] = row;

            row_index++;
        }
        odbc_res->n_rows = row_index;

        for (int i = 0; i < cols; i++) {
            free(tmp_buf[i]);
        }
        free(tmp_buf);
        free(tmp_len);

    } else {
        odbc_res->n_rows = 0;
        odbc_res->columns = NULL;
        odbc_res->data = NULL;
    }

    return rows;
}


/**
 * @brief 
 * 
 * @param trace 
 * @param format 
 * 
 * @return odbc_result_t* 
 */
odbc_result_t *odbc_execute_stmt(bool trace, char *format, ...)
{
    SQLRETURN rc;
    odbc_result_t *odbc_res = _odbc_allocate_res();
    va_list ap;
    char query[4096];

    if (odbc_env == NULL || odbc_dbc == NULL || odbc_is_connected == false) {
        return 0;
    }

    va_start (ap, format);
    vsprintf (query, format, ap);
    va_end (ap);

    /* Allocate statement handle */
    odbc_res->odbc_rc = SQLAllocHandle (SQL_HANDLE_STMT, odbc_dbc, &(odbc_res->odbc_stmt));

    if ((odbc_res->odbc_rc == SQL_SUCCESS || odbc_res->odbc_rc == SQL_SUCCESS_WITH_INFO)) {

        odbc_res->odbc_rc = SQLExecDirect (odbc_res->odbc_stmt, (SQLCHAR *)query, SQL_NTS);

        if (odbc_res->odbc_rc == SQL_SUCCESS || odbc_res->odbc_rc == SQL_SUCCESS_WITH_INFO) {
            _odbc_fetch_data_oracle (odbc_res);
        }
    }

    return odbc_res;
}

/**
 * @brief 
 * 
 * @param odbc_res 
 */
void odbc_free_result(odbc_result_t *odbc_res)
{
    if (odbc_res) {
        SQLFreeHandle (SQL_HANDLE_STMT, odbc_res->odbc_stmt);
        if (odbc_res->columns) {
            for (int i = 0; i < odbc_res->n_columns; i++) {
                free(odbc_res->columns[i]->name);
                free(odbc_res->columns[i]);
            }
            free(odbc_res->columns);
        }
        if (odbc_res->data) {
            for (int i = 0; i < odbc_res->n_rows; i++) {
                for (int j = 0; j < odbc_res->n_columns; j++) {
                    if (odbc_res->data[i][j]) free(odbc_res->data[i][j]);
                }
                if (odbc_res->data[i]) free(odbc_res->data[i]);
            }
            free(odbc_res->data);
        }
        free(odbc_res);
        odbc_res = NULL;
    }
}

/**
 * @brief 
 * 
 * @param odbc_res 
 * @return unsigned int 
 */
unsigned long odbc_get_ntuples(odbc_result_t *odbc_res)
{
    unsigned int tuples = 0;

    if (odbc_res && odbc_res->columns && odbc_res->data) {
        tuples = odbc_res->n_rows;
    }
    return tuples;
}

/**
 * @brief 
 * 
 * @param odbc_res 
 * @return unsigned int 
 */
unsigned int odbc_get_nfields(odbc_result_t *odbc_res)
{
    unsigned int fields = 0;

    if (odbc_res && odbc_res->columns && odbc_res->data) {
        fields = odbc_res->n_columns;
    }
    return fields;
}

/**
 * @brief 
 * 
 * @param odbc_res 
 * @param tuple 
 * @param field 
 * @return char* 
 */
char *odbc_get_fvalue(odbc_result_t *odbc_res,unsigned long tuple, unsigned int field)
{
    char *value = NULL;

    if (odbc_res && tuple < odbc_res->n_rows && field < odbc_res->n_columns && odbc_res->data) {
        //printf("(%d,%d) (%s) \n", tuple, field, odbc_res->data[tuple][field]);
        value = odbc_res->data[tuple][field];
    }
    return value;
}

/**
 * @brief 
 * 
 * @param odbc_res 
 * @param field 
 * @return char* 
 */
char *odbc_get_fname(odbc_result_t *odbc_res, unsigned int field)
{
    char *value = NULL;

    if (odbc_res && field < odbc_res->n_columns && odbc_res->columns) {
        value = odbc_res->columns[field]->name;
    }
    return value;
}

/**
 * @brief 
 * 
 * @param odbc_res 
 * @return unsigned int 
 */
unsigned long odbc_get_cmd_ntuples(odbc_result_t *odbc_res)
{
    unsigned long cmd_tuples = 0;

    if (odbc_res) {
        odbc_res->odbc_rc = SQLRowCount(odbc_res->odbc_stmt,  (SQLLEN *)&cmd_tuples);  
    }
    return cmd_tuples;

}

/**
 * @brief 
 * 
 * @param odbc_res 
 * @param field 
 * @return unsigned long 
 */
unsigned long odbc_get_fsize(odbc_result_t *odbc_res, unsigned int field)
{
    unsigned long value = 0;

    if (odbc_res && field < odbc_res->n_columns && odbc_res->columns) {
        value = odbc_res->columns[field]->size;
    }
    return value;
}

/**
 * @brief 
 * 
 * @param odbc_res 
 * @param field_name 
 * @return unsigned int 
 */
unsigned int odbc_get_fnumber(odbc_result_t *odbc_res,char *field_name)
{
    int value = -1;

    if (odbc_res && odbc_res->columns) {
        for (int j = 0; j < odbc_res->n_columns; j++) {
            if(!strcasecmp(field_name, odbc_res->columns[j]->name)){
                value = j;
                break;
            }
        }
    }
    return value;
}

unsigned int odbc_get_flen(odbc_result_t *odbc_res, unsigned int field)
{
    int value = 0;

    if (odbc_res && odbc_res->columns) {
        value = odbc_res->columns[field]->flen;
    }
    return value;
}

#ifdef _TEST_
/*
odbc_exec_status_type_t odbc_result_status(odbc_result_t *odbc_res);
odbc_field_type_t odbc_get_ftype(odbc_result_t *odbc_res,int field);
char *odbc_result_error_message(odbc_result_t *odbc_res);
*/

static void _odbc_print_result(odbc_result_t *odbc_res)
{
    if (odbc_res) {

        for (int j=0; j < odbc_get_nfields(odbc_res); j++){
            printf("'%s'", odbc_get_fname(odbc_res, j));
        }
        printf("\n");
        for (int j=0; j < odbc_get_nfields(odbc_res); j++){
            printf(" ---------- ");
        }
        printf("\n");
        for (int i=0; i < odbc_get_ntuples(odbc_res); i++){
            for (int j=0; j < odbc_get_nfields(odbc_res); j++){
                printf("'%s'", odbc_get_fvalue(odbc_res, i, j));
            }
            printf("\n");
        }
    }
}

int main(int argc, char **argv)
{
    char *table_name="operatori";
    odbc_connect ("PSQL", "roberto", "3zin");

    srand(time(NULL));
    
    odbc_result_t *odbc_res;

    printf("execute : create table test (numero numeric,testo text)\n");
    if((odbc_res = odbc_execute_stmt (false, "create table test (numero numeric,testo text)"))){
        printf("cmd_tuples = %ld\n", odbc_cmd_ntuples(odbc_res));

        odbc_free_result(odbc_res);
    }
    printf("execute : insert into test values (13,'iutryiuytr')\n");
    if((odbc_res = odbc_execute_stmt (false, "insert into test values (13,'iutryiuytr')"))){
        printf("cmd_tuples = %ld\n", odbc_cmd_ntuples(odbc_res));
        odbc_free_result(odbc_res);
    }

    printf("execute : update test set numero=16\n");
    if((odbc_res = odbc_execute_stmt (false, "update test set numero=16"))){
        printf("cmd_tuples = %ld\n", odbc_cmd_ntuples(odbc_res));
        odbc_free_result(odbc_res);
    }


    if((odbc_res = odbc_execute_stmt (false, "select * from test"))){
        _odbc_print_result(odbc_res);
        odbc_free_result(odbc_res);
    }

    if((odbc_res = odbc_execute_stmt (false, "select * from test where numero=67"))){
        _odbc_print_result(odbc_res);
        odbc_free_result(odbc_res);
    }

    if((odbc_res = odbc_execute_stmt (false, "select * from %s", table_name))){
        _odbc_print_result(odbc_res);
        odbc_free_result(odbc_res);
    }

    if((odbc_res = odbc_execute_stmt (false, "select * from ric_ord limit 10"))){
        _odbc_print_result(odbc_res);
        printf("fname(0) = %s\n", odbc_get_fname(odbc_res, 0));
        printf("fname(10) = %s\n", odbc_get_fname(odbc_res, 10));
        printf("fnumber('ORDPROG') = %d\n", odbc_get_fnumber(odbc_res, "ORDPROG"));
        printf("fnumber('ROCPCLI') = %d\n", odbc_get_fnumber(odbc_res, "ROCPCLI"));
        odbc_free_result(odbc_res);
    }
    return 0;
}
#endif
#endif
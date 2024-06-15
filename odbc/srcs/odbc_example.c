#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>

#include <sql.h>
#include <sqlext.h>

SQLHENV henv = NULL;
SQLHDBC hdbc = NULL;

/**
 * @brief 
 * 
 * @param ds 
 * @param user 
 * @param pw 
 * @return int 
 */
int odbc_connect (SQLCHAR * ds, SQLCHAR * user, SQLCHAR * pw)
{
    SQLRETURN rc;

    hdbc = NULL;
    henv = NULL;

    // Allocate environment handle
    rc = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
        //  Set the ODBC version environment attribute
        rc = SQLSetEnvAttr (henv, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);

        if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
            // Allocate connection handle 
            rc = SQLAllocHandle (SQL_HANDLE_DBC, henv, &hdbc);

            if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
                // Set login timeout to 5 seconds. 
                SQLSetConnectAttr (hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER) 5, 0);

                // Connect to data source
                rc = SQLConnect (hdbc, ds, SQL_NTS, user, SQL_NTS, pw, SQL_NTS);

                if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
                    return 1;
                }
                SQLDisconnect (hdbc);
            }
            SQLFreeHandle (SQL_HANDLE_DBC, hdbc);
        }
    }
    SQLFreeHandle (SQL_HANDLE_ENV, henv);

    hdbc = NULL;
    henv = NULL;

    return 0;
}

/**
 * @brief 
 * 
 * @return int 
 */
int odbc_disconnect ()
{
    if (hdbc) {
        SQLDisconnect (hdbc);
        SQLFreeHandle (SQL_HANDLE_DBC, hdbc);
    }
    if (henv)
        SQLFreeHandle (SQL_HANDLE_ENV, henv);
    hdbc = NULL;
    henv = NULL;
    return 1;
}

/**
 * @brief 
 * 
 * @param stmt_str 
 * @return int 
 */
SQLHSTMT odbc_exec_stmt (char *stmt_str, ...)
{
    SQLHSTMT hstmt;
    SQLRETURN rc;
    va_list ap;
    char query[4096];

    if (henv == NULL || hdbc == NULL)
        return 0;

    va_start (ap, stmt_str);
    vsprintf (query, stmt_str, ap);
    va_end (ap);

    /* Allocate statement handle */
    rc = SQLAllocHandle (SQL_HANDLE_STMT, hdbc, &hstmt);
    if (!(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO))
        return NULL;

    rc = SQLExecDirect (hstmt, query, SQL_NTS);
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
        return hstmt;
    if (rc == SQL_NO_DATA)
        return hstmt;

    return NULL;
}

SQLRETURN odbc_free_stmt (SQLHSTMT hstmt)
{
    return SQLFreeHandle (SQL_HANDLE_STMT, hstmt);
}

/**
 * @brief 
 * 
 * @return int 
 */
int odbc_fetch (SQLHSTMT hstmt)
{
    SQLRETURN r;
    SQLCHAR sql[256];
    SQLLEN  n;
    SQLINTEGER id;
    SQLCHAR name[20];
    SQLREAL age;
    SQL_DATE_STRUCT birthday;
    SQL_TIMESTAMP_STRUCT create_timestamp;
    SQLSMALLINT columns; /* number of columns in result-set */
    SQLCHAR buf[ 5 ][ 64 ];
    SQLINTEGER indicator[ 5 ];


    if (hdbc == NULL || henv == NULL || hstmt == NULL)
        return 0;

    SQLNumResultCols(hstmt, &columns);

    /* Loop through the rows in the result-set binding to */
    /* local variables */
    for (int i = 0; i < columns; i++) {
        SQLBindCol( hstmt, i + 1, SQL_C_CHAR,
        buf[ i ], sizeof( buf[ i ] ), &indicator[ i ] );
    }

    /* Fetch the data */
    while (SQL_SUCCEEDED(SQLFetch(hstmt))) {
        /* display the results that will now be in the bound area's */
        for ( int i = 0; i < columns; i ++ ) {
            if (indicator[ i ] == SQL_NULL_DATA) {
                printf(" NULL ", i);
            } else {
                printf(" %s ", buf[ i ]);
            }
        }
        printf("\n");
    }    

    /*
    while (1) {
        r = SQLFetch (hstmt);
        if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
            r = SQLGetData (hstmt, 1, SQL_C_ULONG, &id, 0, &n);
            r = SQLGetData (hstmt, 2, SQL_C_CHAR, name, 20, &n);
            r = SQLGetData (hstmt, 3, SQL_C_FLOAT, &age, 0, &n);
            r = SQLGetData (hstmt, 4, SQL_C_TYPE_DATE, &birthday, 0, &n);
            r = SQLGetData (hstmt, 5, SQL_C_TYPE_TIMESTAMP, &create_timestamp, 0, &n);
            printf ("%d, %s\n", id, name);
        }
        else if (SQL_NO_DATA == r)
            break;
        else {
            printf ("%s\n", "fail to fetch data");
            break;
        }
    }
    */
}

void test_oracle(void)
{
    char *dsn = "ORA21";
    char *uid = "TEST_PLC";
    char *pwd = "TEST_PLC";
    char *table_name = "TEST_MANTOVANI";
    SQLHSTMT hstmt;

    odbc_connect ("ORA21", "TEST_PLC", "TEST_PLC");

    srand(time(NULL));

    char col1=random()%26+65;
    char col2=random()%100;
    hstmt = odbc_exec_stmt ("insert into %s (col1,col2) values (\'%c\',%d)", table_name, col1, col2);

    odbc_free_stmt (hstmt);

    hstmt = odbc_exec_stmt ("select * from %s", table_name);

    odbc_fetch (hstmt);

    odbc_free_stmt (hstmt);

    odbc_disconnect ();
}

void test_informix(char *barcode)
{
    char *dsn = "generoaix02";
    char *uid = "sorterp6";
    char *pwd = "Incasgr0up";
    char *table_name = "AEL_COLLI";
    char sql_cmd[256];
    SQLHSTMT hstmt;

    if (odbc_connect (Cfg.DSN, Cfg.user_name, Cfg.pwd)){
        sprintf(sql_cmd, "select forza, pesot, toll, cntpesatura from AEL_COLLI where barcode='%s'", barcode);
        printf("execute : %s\n", sql_cmd);
        if((odbc_res = odbc_execute_stmt (FALSE, sql_cmd)) && SQL_SUCCEEDED(odbc_res->odbc_rc)){
            tuples = odbc_get_ntuples(odbc_res);
            odbc_free_result(odbc_res);
        } else {
            printf("ODBC EXECUTE STMT ERROR");
        } 
        odbc_disconnect();
    } else {
        printf("ODBC CONNECT ERROR");
    }
}

int main (int argc, char **argv)
{
    test_informix("AME057810775");
    return 0;
}

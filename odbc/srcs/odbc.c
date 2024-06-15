/**
* @file ep-odbc.c
* @brief odbc functions
* 
* @date Sat Jul 8 19:10:07 CEST 2023
* @author Roberto Mantovani
* @copyright A&L srl 2003-2023
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <trace.h>

#include <sql.h>
#include <sqlext.h>

#include <glib.h>
#include <ep-common.h>
#include <ep-odbc.h>

//#define __ORACLE_AT_LOG21__
//#define __PGSQL_AT_HOME__
#define __INFORMIX_AT_CEVA__

static void _odbc_print_result(odbc_result_t *odbc_res)
{
    if (odbc_res && odbc_res->columns && odbc_res->data) {
        for (int j=0; j < odbc_get_nfields(odbc_res); j++){
            printf("| %-*.*s ", odbc_get_flen(odbc_res, j), odbc_get_flen(odbc_res, j), odbc_get_fname(odbc_res, j));
            if (j+1==odbc_get_nfields(odbc_res)) printf("|\n");
        }
        for (int j=0; j < odbc_get_nfields(odbc_res); j++){
            printf("%s| ", j ? " " : "");
            for(int k=0; k < odbc_get_flen(odbc_res, j); k++) printf ("-");
            if (j+1==odbc_get_nfields(odbc_res)) printf(" |\n");
        }
        for (int i=0; i < odbc_get_ntuples(odbc_res); i++){
            for (int j=0; j < odbc_get_nfields(odbc_res); j++){
                printf("| %-*.*s ", odbc_get_flen(odbc_res, j), odbc_get_flen(odbc_res, j), odbc_get_fvalue(odbc_res, i, j));
                if (j+1==odbc_get_nfields(odbc_res)) printf("|\n");
            }
        }
    }
}

static void init_trace(long size)
{
#ifdef TRACE
    char szBufTrace[80];

    strcpy(szBufTrace, "../trace");
    strcat(szBufTrace,"/");
    strcat(szBufTrace, "odbc");
    strcat(szBufTrace, TRACE_FILE_SUFFIX);

    open_trace("odbc", szBufTrace, size);

    trace_debug(TRUE, TRUE, "Started");
#endif
}

#ifdef __PGSQL_AT_HOME__

int main(int argc, char **argv)
{
    char *table_name="operatori";
    char *DSN="PSQL";
    char *user="roberto";
    char *pwd="3zin";
    char sql_cmd[256];


    printf("ODBC TEST : DSN : [%s] - User [%s] - pwd [%s] - table [%s]\n", DSN, user, pwd, table_name);

    odbc_connect (DSN, user, pwd);

    srand(time(NULL));
    
    odbc_result_t *odbc_res;

    sprintf(sql_cmd, "create table test (numero numeric,testo text)");
    printf("execute : %s\n", sql_cmd);
    if((odbc_res = odbc_execute_stmt (false, sql_cmd))){
        printf("cmd_tuples = %ld\n", odbc_get_cmd_ntuples(odbc_res));

        odbc_free_result(odbc_res);
    }
    sprintf(sql_cmd, "insert into test values (13,'iutryiuytr')");
    printf("execute : %s\n", sql_cmd);
    if((odbc_res = odbc_execute_stmt (false, sql_cmd))){
        printf("cmd_tuples = %ld\n", odbc_get_cmd_ntuples(odbc_res));
        odbc_free_result(odbc_res);
    }

    sprintf(sql_cmd, "update test set numero=16");
    printf("execute : %s\n", sql_cmd);
    if((odbc_res = odbc_execute_stmt (false, sql_cmd))){
        printf("cmd_tuples = %ld\n", odbc_get_cmd_ntuples(odbc_res));
        odbc_free_result(odbc_res);
    }


    sprintf(sql_cmd, "select * from test");
    printf("execute : %s\n", sql_cmd);
    if((odbc_res = odbc_execute_stmt (false, sql_cmd))){
        _odbc_print_result(odbc_res);
        odbc_free_result(odbc_res);
    }

    sprintf(sql_cmd, "select * from test where numero=67");
    printf("execute : %s\n", sql_cmd);
    if((odbc_res = odbc_execute_stmt (false, sql_cmd))){
        _odbc_print_result(odbc_res);
        odbc_free_result(odbc_res);
    }

    sprintf(sql_cmd, "select * from %s", table_name);
    printf("execute : %s\n", sql_cmd);
    if((odbc_res = odbc_execute_stmt (false, sql_cmd))){
        _odbc_print_result(odbc_res);
        odbc_free_result(odbc_res);
    }

    sprintf(sql_cmd, "select * from ric_ord limit 10", table_name);
    printf("execute : %s\n", sql_cmd);
    if((odbc_res = odbc_execute_stmt (false, sql_cmd))){
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

#ifdef __ORACLE_AT_LOG21__

int main(int argc, char **argv)
{
    char *DSN = "ORA21";
    char *user = "CKS_LINEA";
    char *pwd = "lineaAL";
    char sql_cmd[256];
    char szBarcode[128];
    char szPeso[128];
    char szDest[128];
    unsigned long id_seq;
    bool fine;

    init_trace(100*1024*1024);

    trace_debug (TRUE, TRUE, "ODBC TEST : DSN : [%s] - User [%s] - pwd [%s]\n", DSN, user, pwd);

    odbc_connect (DSN, user, pwd);

    srand(time(NULL));

    sprintf(szBarcode, "%d", rand());

    sprintf(szPeso, "%d", rand());

    odbc_result_t *odbc_res;

    sprintf(sql_cmd, "insert into  PESOCOLLO_IN (PAYLOAD_1, PAYLOAD_2) values ('%s','%s')", szBarcode, szPeso);
    trace_debug (TRUE, TRUE, "execute : %s", sql_cmd);
    if((odbc_res = odbc_execute_stmt (false, sql_cmd))){
        trace_debug (TRUE, TRUE, "cmd_tuples = %ld\n", odbc_get_cmd_ntuples(odbc_res));

        odbc_free_result(odbc_res);
    }

    usleep(500000);

    fine = false;
    while(!fine){
        sprintf(sql_cmd, "select ST_REQUEST, DF_RESULT from PESOCOLLO_IN where ST_REQUEST != 'CLOSED'");
        trace_debug (TRUE, TRUE, "execute : %s", sql_cmd);
        if((odbc_res = odbc_execute_stmt (false, sql_cmd))){
            _odbc_print_result(odbc_res);
            if(odbc_get_ntuples(odbc_res)==0) {
                fine = true;
            }
            odbc_free_result(odbc_res);
            trace_debug (TRUE, FALSE, ".");
        }
        usleep(500000);
    }
    trace_debug (TRUE, TRUE, "");

    fine = false;
    while(!fine){
        sprintf(sql_cmd, "select ID_SEQ, PAYLOAD_1, PAYLOAD_2 from DESTCOLLO_OUT where PAYLOAD_1 = '%s'", szBarcode);
        trace_debug (TRUE, TRUE, "execute : %s", sql_cmd);
        if((odbc_res = odbc_execute_stmt (false, sql_cmd))){
            _odbc_print_result(odbc_res);
            if(odbc_get_ntuples(odbc_res)==1) {
                id_seq = strtoul(odbc_get_fvalue(odbc_res, 0,0), NULL, 10);
                strcpy(szDest, odbc_get_fvalue(odbc_res, 0,2));
                fine = true;
            }
            odbc_free_result(odbc_res);
        }
        usleep(500000);
        trace_debug (TRUE, FALSE, ".");
    }
    trace_debug (TRUE, TRUE, "");

    sprintf(sql_cmd, "update DESTCOLLO_OUT set ST_REQUEST='CLOSED' where ID_SEQ=%ld", id_seq);
    trace_debug (TRUE, TRUE, "execute : %s", sql_cmd);
    if((odbc_res = odbc_execute_stmt (false, sql_cmd))){
        trace_debug (TRUE, TRUE, "cmd_tuples = %ld", odbc_get_cmd_ntuples(odbc_res));

        odbc_free_result(odbc_res);
    }

    trace_debug (TRUE, TRUE, "Destinazione collo [%ld]-[%s] : [%s]", id_seq, szBarcode, szDest);

    return 0;
}
#endif /* __ORACLE_AT_LOG21__ */

#ifdef __INFORMIX_AT_CEVA__
void test_informix(char *barcode)
{
    char *dsn = "generoaix02";
    char *uid = "sorterp6";
    char *pwd = "Incasgr0up";
    char *table_name = "AEL_COLLI";
    char sql_cmd[256];
    odbc_result_t *odbc_res;
    int tuples=-1;

    if (odbc_connect (dsn, uid, pwd)){
        sprintf(sql_cmd, "select forza, pesot, toll, cntpesatura from AEL_COLLI where barcode='%s'", barcode);
        printf("execute : %s\n", sql_cmd);
        if((odbc_res = odbc_execute_stmt (FALSE, sql_cmd))){
            tuples = odbc_get_ntuples(odbc_res);
            odbc_free_result(odbc_res);
        } else {
            printf("ODBC EXECUTE STMT ERROR\n");
        } 
        odbc_disconnect();
    } else {
        printf("ODBC CONNECT ERROR\n");
    }
}

int main (int argc, char **argv)
{
    test_informix("AME057810775");
    return 0;
}
#endif /* __INFORMIX_AT_CEVA__ */

/*
* shm_db_sync.c
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
// #include <termio.h>
#include <sys/stat.h>
#include <glib.h>
#include <ep-common.h>
#include <picking.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
    #include <trace.h>
#endif
#include <proc_list.h>

#include <shmdb.h>

#include "cmp_time.h"

CFG_STRUCT    Cfg;
PLINEA_STRUCT pDatiLinea;     /* */
PLINEA_STRUCT pOldDatiLinea;  /* */
PSETTORE pSettori;            /* */
PSETTORE pOldSettori;         /* */

extern char __module_name__[];

/*
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc()
{
    if (!shmdb_disconnect_linea(&pDatiLinea)){
#ifdef TRACE
        trace_debug(TRUE, TRUE,"Error Disconnecting From Shared Memory (%d)", Cfg.nShmKey);
#endif
    }
    if (!shmdb_disconnect_settori(&pSettori)){
#ifdef TRACE
        trace_debug(TRUE, TRUE,"Error Disconnecting From Shared Memory (%d)", Cfg.nSettoriShmKey);
#endif
    }

#ifdef TRACE
    trace_debug(TRUE, TRUE,"Stopped");
#endif

    exit(1);
} /* Fine SafeIntFunc */


/*
* main()
*/
int main(int argc,char** argv)
{
    char szBufTrace[80];

    /* Lettura della configurazione */
    shmdb_read_configuration(&Cfg);

#ifdef TRACE
    /* Apro il file trace */
    sprintf(szBufTrace,"%s/%s%s",Cfg.szPathTrace,__module_name__,TRACE_FILE_SUFFIX);
    open_trace(__module_name__, szBufTrace, TRACE_FILE_MAXSIZE);

    trace_debug(TRUE, TRUE,"Started");
#endif

    pOldDatiLinea=(PLINEA_STRUCT)malloc(sizeof(LINEA_STRUCT));
    memset(pOldDatiLinea,0,sizeof(LINEA_STRUCT));
    pOldSettori=(PSETTORE)malloc(Cfg.nNumeroSettori*sizeof(SETTORE));
    memset(pOldSettori,0,Cfg.nNumeroSettori*sizeof(SETTORE));



    /* Predispongo l'utilizzo dell'uscita di sicurezza */
    signal(SIGTERM, SafeIntFunc);
    signal(SIGINT,SafeIntFunc);
    signal(SIGILL,SafeIntFunc);
#ifdef __linux__
	signal(SIGQUIT,SafeIntFunc);
	signal(SIGKILL,SafeIntFunc);
#endif

    while(1){
        /*
        * connetto le shared memory di settori e linea
        */
        if(!shmdb_connect_linea(&pDatiLinea)){
#ifdef TRACE
            trace_debug(TRUE, TRUE,"Connessione a dati linea fallita.");
#endif
            pDatiLinea=NULL;
        }
        if(!shmdb_connect_settori(&pSettori)){
#ifdef TRACE
            trace_debug(TRUE, TRUE,"Connessione a settori fallita.");
#endif
            pSettori=NULL;
        }


        if(pDatiLinea) {
            if(memcmp(pOldDatiLinea,pDatiLinea,sizeof(LINEA_STRUCT))){
#ifdef TRACE
                shmdb_dump_dati_linea(pDatiLinea);
#endif

                shmdb_set_dati_linea(pDatiLinea);

                /* attualizzo */
                memcpy(pOldDatiLinea,pDatiLinea,sizeof(LINEA_STRUCT));
            }
        } else {
            memset(pOldDatiLinea,0,sizeof(LINEA_STRUCT));
        }

        if(pSettori) {
            int nIndex;

            for (nIndex = 0; nIndex < Cfg.nNumeroSettori; nIndex++){
                PSETTORE pSettore=&pSettori[nIndex];
                PSETTORE pOldSettore=&pOldSettori[nIndex];

                if(memcmp(pOldSettore,pSettore,sizeof(SETTORE))){
#ifdef TRACE
                    shmdb_dump_settore(pSettore);
#endif

                    shmdb_set_settore(nIndex,pSettore);

                    /* attualizzo */
                    memcpy(pOldSettore,pSettore,sizeof(SETTORE));
                }
            }
        } else {
            memset(pOldSettori,0,Cfg.nNumeroSettori*sizeof(SETTORE));
        }

        if (!shmdb_disconnect_linea(&pDatiLinea)){
#ifdef TRACE
            trace_debug(TRUE, TRUE,"Disconnessione a dati linea fallita.");
#endif
        }
        if (!shmdb_disconnect_settori(&pSettori)){
#ifdef TRACE
            trace_debug(TRUE, TRUE,"Disconnessione a dati settori fallita.");
#endif
        }

        ep_usleep(100000);

    }
    SafeIntFunc();

}    /* End of main() */



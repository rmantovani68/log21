/*
* writeshm.c : legge memcached e scrive shm linea / settori 
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/stat.h>
#include <glib.h>
#include <gio/gio.h>


#include <libmemcached/memcached.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
    #include <trace.h>
#endif

#include <ep-common.h>
#include <picking.h>
#include <proc_list.h>

#include <shm.h>

#include "cmp_time.h"

CFG_STRUCT    Cfg;
PLINEA_STRUCT pDatiLinea;     /* dati in shm */
PLINEA_STRUCT pMemDatiLinea;  /* dati in memcache */
PSETTORE pSettori;            /* dati in shm */
PSETTORE pMemSettori;         /* dati in memcache */

extern char __module_name__[];

/*
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc()
{
    if (!disconnect_linea_shm(&pDatiLinea)){
#ifdef TRACE
        trace_debug(TRUE, TRUE,"Error Disconnecting From Shared Memory (%d)", Cfg.nShmKey);
#endif
    }
    if (!disconnect_settori_shm(&pSettori)){
#ifdef TRACE
        trace_debug(TRUE, TRUE,"Error Disconnecting From Shared Memory (%d)", Cfg.nSettoriShmKey);
#endif
    }
    if (!remove_linea_shm(&pDatiLinea)){
#ifdef TRACE
        trace_debug(TRUE, TRUE,"Error Removing Shared Memory (%d)", Cfg.nShmKey);
#endif
    }
    if (!remove_settori_shm(&pSettori)){
#ifdef TRACE
        trace_debug(TRUE, TRUE,"Error Removing Shared Memory (%d)", Cfg.nSettoriShmKey);
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
    memcached_server_st *servers = NULL;
    memcached_st *memc;
    memcached_return rc;

    memc = memcached_create(NULL);
    servers = memcached_server_list_append(servers, Cfg.szMemCachedServerName, Cfg.nMemCachedPortNumber, &rc);
    if((rc = memcached_server_push(memc, servers)==MEMCACHED_SUCCESS)){
#ifdef TRACE
        trace_debug(TRUE, TRUE,"Added server successfully [%s:%d]",Cfg.szMemCachedServerName, Cfg.nMemCachedPortNumber);
#endif
    } else {
#ifdef TRACE
        trace_debug(TRUE, TRUE,"Couldn't add server [%s:%d]: %s", Cfg.szMemCachedServerName, Cfg.nMemCachedPortNumber, memcached_strerror(memc, rc));
#endif
    }


    /* Lettura della configurazione */
    shm_read_configuration(&Cfg);

#ifdef TRACE
    /* Apro il file trace */
    sprintf(szBufTrace,"%s/%s%s",Cfg.szPathTrace,__module_name__,TRACE_FILE_SUFFIX);
    open_trace(__module_name__, szBufTrace, TRACE_FILE_MAXSIZE);

    trace_debug(TRUE, TRUE,"Started");
#endif

    /*
    * alloco le shared memory di settori e linea
    */
    if(!setup_linea_shm(&pDatiLinea)){
#ifdef TRACE
        trace_debug(TRUE, TRUE,"Creazione shm dati linea fallita.");
#endif
        pDatiLinea=NULL;
    }
    if(!setup_settori_shm(&pSettori)){
#ifdef TRACE
        trace_debug(TRUE, TRUE,"Creazione shm settori fallita.");
#endif
        pSettori=NULL;
    }

    /* alloco dati per memcache */
    pMemDatiLinea=(PLINEA_STRUCT)malloc(sizeof(LINEA_STRUCT));
    memset(pMemDatiLinea,0,sizeof(LINEA_STRUCT));

    pMemSettori=(PSETTORE)malloc(Cfg.nNumeroSettori*sizeof(SETTORE));
    memset(pMemSettori,0,Cfg.nNumeroSettori*sizeof(SETTORE));



    /* Predispongo l'utilizzo dell'uscita di sicurezza */
    signal(SIGTERM, SafeIntFunc);
    signal(SIGINT,SafeIntFunc);
    signal(SIGQUIT,SafeIntFunc);
    signal(SIGILL,SafeIntFunc);
    signal(SIGKILL,SafeIntFunc);



    while(1){
        /*
        * connetto le shared memory di settori e linea
        */
        if(!connect_linea_shm(&pDatiLinea)){
#ifdef TRACE
            trace_debug(TRUE, TRUE,"Connessione a dati linea fallita.");
#endif
            pDatiLinea=NULL;
        }

        if(!connect_settori_shm(&pSettori)){
#ifdef TRACE
            trace_debug(TRUE, TRUE,"Connessione a settori fallita.");
#endif
            pSettori=NULL;
        }

        /* leggo i dati linea da memcached */
        memcache_get_dati_linea(pMemDatiLinea, memc);

        if(pDatiLinea) {
            if(memcmp(pMemDatiLinea,pDatiLinea,sizeof(LINEA_STRUCT))){
#ifdef TRACE
                dump_dati_linea(pDatiLinea);
#endif
                /* attualizzo */
                memcpy(pDatiLinea,pMemDatiLinea,sizeof(LINEA_STRUCT));
            }
        }

        /* leggo i dati dei settori da memcached */
        memcache_get_settori(pMemSettori, memc);

        if(pSettori) {
            int nIndex;

            for (nIndex = 0; nIndex < Cfg.nNumeroSettori; nIndex++){
                PSETTORE pSettore=&pSettori[nIndex];
                PSETTORE pMemSettore=&pMemSettori[nIndex];

                if(memcmp(pMemSettore,pSettore,sizeof(SETTORE))){
                    
#ifdef TRACE
                    dump_settore(pMemSettore);
#endif
                    /* attualizzo */
                    memcpy(pSettore,pMemSettore,sizeof(SETTORE));
                }
            }
        }

        if (!disconnect_linea_shm(&pDatiLinea)){
#ifdef TRACE
            trace_debug(TRUE, TRUE,"Disconnessione a dati linea fallita.");
#endif
        }
        if (!disconnect_settori_shm(&pSettori)){
#ifdef TRACE
            trace_debug(TRUE, TRUE,"Disconnessione a dati settori fallita.");
#endif
        }

        usleep(100000);

    }
    SafeIntFunc();

}    /* End of main() */



/*
* volum.c : Analisi Volumetrica
* 
* Progetto Easy Picking 4.0 : Wella Rebuid
*
* Autore : Roberto Mantovani - Stefano Tarroni
*
* Copyright A&L srl 2003-2021
*
* 10-10-2000 rm+st : Inizio rebuild Wella 
*
* Sostituzione delle librerie CB con PostgreSQL
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
// #include <termio.h>
#include <sys/stat.h>
#include <glib.h>
#include <gio/gio.h>


#include <libpq-fe.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#include <trace.h>
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>
#include <proc_list.h>

#include    "volum.h"
#include    "volstruct.h"
#include    "volext.h"
#include    "volfun.h"

extern char *optarg;
extern int optind;
extern char rcsid[];
extern char __version__[];
extern char __customer__[];

/*
* main()
*/
int main(int argc,char** argv)
{
    int nErr=0;
    int nOpt;


    Cfg.bAlone=FALSE;

    while ((nOpt = getopt(argc, argv, "ap:")) != -1) {
        switch(nOpt){
            case 'p':    /* Process ID */
                nPID=atoi(optarg);
            break;
            case 'a':    /* alone */
                Cfg.bAlone=TRUE;
            break;
            default:
                nErr++;
            break;
        }
    }
    if(nPID==0){
        nPID=PROC_VOLUM;
    }
 

    if(nErr){
        printf("%s : copyright 2000-2021 A&L srl\n",argv[0]);
        printf("Uso : %s [-d]\n",argv[0]);
        printf("\t-d    : Run Alone\n");

        exit(0);
    }



    /*
    * Lettura della configurazione corrente
    */
    ReadConfiguration(TRUE);


#ifdef TRACE
    pszNomeModulo = g_path_get_basename (argv[0]);
    /* Apro il file trace */
    sprintf(szBufTrace, "%s/%s%s", Cfg.szPathTrace,pszNomeModulo, TRACE_FILE_SUFFIX);
    open_trace(pszNomeModulo, szBufTrace, PICKING_TRACE_FILE_MAXSIZE);

    trace_debug(TRUE, TRUE, "Started");
    trace_debug(FALSE, TRUE, "Module Name     : %s",argv[0]);
    trace_debug(FALSE, TRUE, "Version         : %s",__version__);
    trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
    trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
    trace_debug(FALSE, TRUE, "Stand Alone Mode: %s",Cfg.bAlone?"ON":"OFF");
#endif
    
    /*
    * Connetto a DB Postgres
    */
    if(!DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword)){
        trace_debug(TRUE, TRUE, "Connection to database [%s:%s] failed.", Cfg.szDBHost, Cfg.szDBName);
        exit(1);
    }


    /*
    * Apro la coda messaggi principale 
    */
    if((OpenProcessMsgQ(PROC_MAIN))<0){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "Apertura coda messaggi principale fallita");
#endif
    }
    /* Creo la coda messaggi locale */
    if(CreateProcessMsgQ(nPID, 1)<0){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "Creazione coda messaggi locale fallita");
#endif
    }

    /*
    * Leggo la shared memory per la gestione della linea
    */
    if((pDatiLinea=(PLINEA_STRUCT)OpenSharedMemory(Cfg.nShmKey,sizeof(LINEA_STRUCT)))==NULL){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "%s : Cannot Get Shared Memory [%d] !\n",             pszNomeModulo, Cfg.nShmKey);
#endif
        pDatiLinea=(PLINEA_STRUCT)malloc(sizeof(LINEA_STRUCT));
        memset(pDatiLinea,0,sizeof(LINEA_STRUCT));
    }

    /* Predispongo l'utilizzo dell'uscita di sicurezza */
    signal(SIGTERM, SafeIntFunc);
    signal(SIGINT,  SafeIntFunc);
    signal(SIGILL,  SafeIntFunc);
#ifdef __linux__
	signal(SIGQUIT,SafeIntFunc);
	signal(SIGKILL,SafeIntFunc);
#endif

    /* 
    * modifico il livello di protezione di lettura / scrittura dei files
    */
    umask(0);


    ReadImballi(&Imballi);
    SortImballi(&Imballi);

#ifdef TRACE_ELIMINATO
    {
        int nIndex;

        for(nIndex=0;nIndex<Imballi.nImballi;nIndex++){
            trace_debug(FALSE, TRUE, "COD:%s L:%d CAT:%d",                 Imballi.Imballo[nIndex].szCodice,                 Imballi.Imballo[nIndex].nSwLinea,                 Imballi.Imballo[nIndex].nCategoria);
        }
        trace_debug(FALSE, TRUE, "Numero Imballi : %d",Imballi.nImballi);
    }
#endif

    /*
    * avverto MAIN dello start
    */
    SendMessage(PROC_MAIN, PROC_VOLUM, PROGRAM_STARTED, NULL);

    /*
    * gestione program loop
    */
    for(;;){
        /*
        * Gestione Messaggi Ricevuti
        */
        nAllMsg = 0;
        if((nCarIn = ProcessReceiveMsgNoWait(PROC_VOLUM, &nAllMsg, szInMsg))>=0){
            UnpackMessage(szInMsg, &InMsgStruct, szText, nCarIn);

            switch(InMsgStruct.code){
                case REFRESH_CONFIGURATION:
                    ReadImballi(&Imballi);
                    SortImballi(&Imballi);
                    ReadConfiguration(FALSE);
                break;
                case ANALISI_ORDINE:
                {
                    ep_bool_t bRC;
                    DBresult *DBResOrdini;
                    ep_bool_t bPallet=FALSE;
                    int nSWCol;

                    DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog,roswcol from ric_ord where ordprog='%s';",szText);
                    nSWCol=atoi(DBgetvalue(DBResOrdini,0,1));
                    if(nSWCol==ORDINE_PALLET){
                        bPallet=TRUE;
                    } else {
                        bPallet=FALSE;
                    }
                    DBclear(DBResOrdini);

                    pDatiLinea->bAnalisi=TRUE;
                    bRC=AnalisiVolumetricaOrdine(szText,TRUE,bPallet,FALSE,FALSE,InMsgStruct.srce,FALSE);
                    pDatiLinea->bAnalisi=FALSE;
                }
                break;


                case START_ANALISI:
                {
                    int nOrdini;

                    SendMessage(InMsgStruct.srce, PROC_VOLUM, VOLUM_START_ANALISI, NULL);
                    pDatiLinea->bAnalisi=TRUE;
                    nOrdini=AnalisiVolumetrica(ORDINE_SPEDITO,TRUE,InMsgStruct.srce,FALSE);
                    pDatiLinea->bAnalisi=FALSE;
                    if(nOrdini){
                        SendMessage(InMsgStruct.srce, PROC_VOLUM, VOLUM_END_ANALISI, NULL);
                    }
                }
                break;
                case START_ANALISI_TEST:
                {
                    int nOrdini;

                    SendMessage(InMsgStruct.srce, PROC_VOLUM, VOLUM_START_ANALISI_TEST, NULL);
                    pDatiLinea->bAnalisi=TRUE;
                    nOrdini=AnalisiVolumetrica(ORDINE_RICEVUTO,TRUE,InMsgStruct.srce,TRUE);
                    pDatiLinea->bAnalisi=FALSE;
                    if(nOrdini){
                        SendMessage(InMsgStruct.srce, PROC_VOLUM, VOLUM_END_ANALISI_TEST, NULL);
                    }
                }
                break;
            }
        }
        usleep(1000*Cfg.nDelay);
    }


    SafeIntFunc();
}    /* End of main() */


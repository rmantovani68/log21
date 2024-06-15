/**
* @file printlabel.c
* Stampa Etichette
* 
* Progetto Easy Picking 4.0
* @date 02/02/1999
*
* @author Roberto Mantovani
*
* Storia delle modifiche
* S. DATA         DESCRIZIONE
* -- ----------   ----------------------------------------
* rm 07-02-1999 : Inizio versione Europharco
* rm 10-02-1999 : Corretti alcuni errori di ricerca archivi ...
* rm 11-02-1999 : Aggiunte alcune gestioni (corriere, flag ritiro presso deposito ...)
* rm 23-02-1999 : Stampa Etichette Depositi
* rm 06-04-1999 : Apertura / Chiusura archivi ad ogni operazione
* rm 24-01-2000 : Inizio versione S.L.I.
* rm 09-11-2000 : Inizio versione WELLA - POSTGRESQL 
* rm 13-03-2001 : Inizio versione EMI - POSTGRESQL 
* rm 30-05-2001 : Utilizzo fork per velocizzare la stampa
* rm 30-05-2001 : si usa lpr con l'opzione -r - cancella file - dove necessita
* rm 27-06-2001 : modificato per rendere indipendente il processo dal PID (lancio multiplo)
* rm 15-08-2001 : Modifiche per Versione DeAgostini
* rm 05-02-2002 : Riscrittura della versione Mondadori - Delta
* -- ----------   ----------------------------------------
*
* @copyright A&L srl 2003-2023
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
//#include <termio.h>
#include <sys/stat.h>
#include <glib.h>
#include <gio/gio.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
    #include <trace.h>
#endif
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>
#include <proc_list.h>

#include    "printlabel.h"
#include    "printstruct.h"
#include    "printext.h"
#include    "printfun.h"

/*
* main()
*/
int main(int argc,char** argv)
{
    int nErr=0;
    ep_bool_t bAlone=FALSE;
    int nOpt;
    char szBolla[80];
    char szPrinter[80];

    strcpy(szBolla,"");
    strcpy(szPrinter,"lp");


    while ((nOpt = getopt(argc, argv, "hp:")) != -1) {
        switch(nOpt){
            case 'p':    /* Process ID */
                nPID=atoi(optarg);
            break;
            case 'h':
                printf("%s : Copyright A&L srl 2003-2021\n",argv[0]);
                printf("Uso: %s [-h] [-p <pid>]\n",argv[0]);
                printf("\t-h       : help\n");
                printf("\t-p <pid> : PID\n");
                exit(-1);
            break;
            default:
                nErr++;
            break;
        }
    }
 

    if(nPID==0){
        fprintf(stderr,"PID non definito\n");
        nPID=PROC_PRINTLABEL;
    }


    if(nErr){
        printf("%s : Copyright A&L srl 2003-2021\n",argv[0]);
        printf("Uso: %s [-h] [-p <pid>]\n",argv[0]);
        printf("\t-h       : help\n");
        printf("\t-p <pid> : PID\n");
        exit(0);
    }


    pszNomeModulo = argv[0];


    /*
    * Lettura della configurazione corrente
    */
    ReadConfiguration();

    /*
    * Leggo la shared memory per la gestione della linea
    */
    if((pDatiLinea=(PLINEA_STRUCT)GetShm(Cfg.nShmKey,sizeof(LINEA_STRUCT)))==NULL){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "%s : Cannot Get Shared Memory [%d] !\n", pszNomeModulo, Cfg.nShmKey);
#endif
        pDatiLinea=(PLINEA_STRUCT)malloc(sizeof(LINEA_STRUCT));
        memset(pDatiLinea,0,sizeof(LINEA_STRUCT));
    }

    /*
    * Gestione Database PostgreSQL
    */
    DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword);

#ifdef TRACE
    /* Apro il file trace */
    sprintf(szBufTrace, "%s/%s%s", Cfg.szPathTrace, g_path_get_basename (pszNomeModulo), TRACE_FILE_SUFFIX);

    open_trace(pszNomeModulo, szBufTrace, TRACE_FILE_MAXSIZE);

    trace_debug(FALSE, TRUE, "Stand Alone Mode: %s",bAlone?"ON":"OFF");
    trace_debug(FALSE, TRUE, "Module Name     : %s",argv[0]);
    trace_debug(FALSE, TRUE, "Version         : %s",__version__);
    trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
    trace_debug(FALSE, TRUE, "Author          : %s",__authors__);
    trace_debug(FALSE, TRUE, "Copyright       : %s",__copyright__);
    trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
    trace_debug(FALSE, TRUE, "Process ID      : %d",nPID);
    trace_debug(TRUE, TRUE, "Started");
#endif
    
    /* Apro la coda messaggi principale */
    if(OpenProcessMsgQ( Cfg.nMainID)<0){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "%s : Apertura coda messaggi principale fallita", pszNomeModulo);
#endif
    }
    /* Apro la coda messaggi locale */
    if(CreateProcessMsgQ(nPID, 1)<0){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "%s : Creazione coda messaggi locale fallita", pszNomeModulo);
#endif
    }

    /* Predispongo l'utilizzo dell'uscita di sicurezza */
    signal(SIGTERM, SafeIntFunc);
    signal(SIGINT,SafeIntFunc);
    //signal(SIGQUIT,SafeIntFunc);
    signal(SIGILL,SafeIntFunc);
    //signal(SIGKILL,SafeIntFunc);


    /*
    * avverto MAIN dello start
    */
    SendMessage(Cfg.nMainID, nPID, PROGRAM_STARTED, NULL);


    /*
    * gestione program loop
    */
    for(;;){
        /*
        * Gestione Messaggi Ricevuti
        */
        nAllMsg = 0;
        if((nCarIn = ProcessReceiveMsg(nPID, &nAllMsg, szInMsg))>=0){
            UnpackMessage(szInMsg, &InMsgStruct, szText, nCarIn);

            switch(InMsgStruct.code){
                case REFRESH_CONFIGURATION:
                break;
                case PRINT_RAC_STORICO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

                    pDatiLinea->bStampaRAC=TRUE;
                    if (StampaEtichetteOrdineStorico(NULL,szOrdKey,szPrinter,InMsgStruct.srce)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_RAC_STAMPATO,szOrdKey );
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_RAC_ERROR,szOrdKey );
                    }
                    pDatiLinea->bStampaRAC=FALSE;
                }
                break;
                case PRINT_RAC_COLLO_STORICO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];
                    int nCollo = 0;;

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Collo */   if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ nCollo=atoi(pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

                    pDatiLinea->bStampaRAC=TRUE;
                    if (StampaEtichettaColloStorico(szOrdKey,nCollo,szPrinter,InMsgStruct.srce)){
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_RAC_ERROR,szOrdKey );
                    }
                    pDatiLinea->bStampaRAC=FALSE;
                }
                break;
                /*
                * Messaggi di stampa
                */
                case PRINT_RAC:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

                    pDatiLinea->bStampaRAC=TRUE;
                    if (StampaEtichetteOrdine(NULL,szOrdKey,szPrinter,InMsgStruct.srce)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_RAC_STAMPATO,szOrdKey );
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_RAC_ERROR,szOrdKey );
                    }
                    pDatiLinea->bStampaRAC=FALSE;
                }
                break;
                case PRINT_RAC_COLLO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];
                    int nCollo = 0;

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Collo */   if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ nCollo=atoi(pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

                    pDatiLinea->bStampaRAC=TRUE;
                    if (StampaEtichettaCollo(szOrdKey,nCollo,szPrinter,InMsgStruct.srce)){
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_RAC_ERROR,szOrdKey );
                    }
                    pDatiLinea->bStampaRAC=FALSE;
                }
                break;
                case PRINT_RAC_LINEA:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szNMCED[128];
                    char szTPSPE[128];
                    char szCDLIN[128];
                    char szCDVET[128];
                    char szPrinter[128];
                    char szLinea[128];

                    if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }   /* Numero Cedola */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szTPSPE,pPtr); }     /* Tipo Spedizione */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDLIN,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDVET,pPtr); }     /* VETTORE */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }   /* Printer */ 

                    /*
                    * Stampa di RAC per linea spedizione
                    */
                    sprintf(szLinea,"%s,%s,%s,%s",szNMCED,szTPSPE,szCDLIN,szCDVET);

                    pDatiLinea->bStampaRAC=TRUE;
                    if (StampaRACLinea(szPrinter,szNMCED,szTPSPE,szCDLIN,szCDVET,InMsgStruct.srce)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_RAC_LINEA_STAMPATA, szLinea);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_RAC_LINEA_ERROR, szLinea);
                    }
                    pDatiLinea->bStampaRAC=FALSE;
                }
                break;

                case PRINT_PACKING_LIST_LINEA_PREFINCATO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szNMCED[128];
                    char szTPSPE[128];
                    char szCDLIN[128];
                    char szCDVET[128];
                    char szPrinter[128];
                    char szLinea[128];

                    if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }   /* Numero Cedola */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szTPSPE,pPtr); }     /* Tipo Spedizione */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDLIN,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDVET,pPtr); }     /* VETTORE */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }   /* Printer */ 

                    /*
                    * Stampa di RAC per linea spedizione
                    */
                    sprintf(szLinea,"%s,%s,%s,%s",szNMCED,szTPSPE,szCDLIN,szCDVET);

                    pDatiLinea->bStampa=TRUE;
                    if (StampaPackingListLinea(szPrinter,szNMCED,szTPSPE,szCDLIN,szCDVET,FALSE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_PACKING_LIST_LINEA_STAMPATA, szLinea);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_PACKING_LIST_LINEA_ERROR, szLinea);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;

                case PRINT_ALL_RAC:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szPrinter[128];

                    /* Printer */ if((pPtr=strtok(szText  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

                    pDatiLinea->bStampaRAC=TRUE;
                    if (StampaAllRAC(szPrinter,InMsgStruct.srce)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_ALL_RAC_STAMPATO, NULL);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_ALL_RAC_ERROR, NULL);
                    }
                    pDatiLinea->bStampaRAC=FALSE;
                }
                break;
                case PRINT_PACKING_LIST:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

#ifdef TRACE
                    trace_debug(TRUE, TRUE, "PRINT_PACKING_LIST [%s] on [%s]", szOrdKey,szPrinter);
#endif
                    /*
                    * Stampa packing list di un ordine
                    */
                    StampaPackingListBollaPrefincato(szPrinter,szOrdKey);
                    //StampaPackingListBolla(szOrdKey,szPrinter);
#ifdef TRACE
                    trace_debug(TRUE, TRUE, "PRINT_PACKING_LIST - FINE");
#endif
                }
                break;
                case PRINT_PACKING_LIST_COLLO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];
                    int nCollo = 0;

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Collo   */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ nCollo=atoi(pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

#ifdef TRACE
                    trace_debug(TRUE, TRUE, "PRINT_PACKING_LIST_COLLO [%s] [%d] on [%s]", szOrdKey,nCollo,szPrinter);
#endif
                    /*
                    * Stampa packing list di un collo all'interno di un ordine
                    */
                    //StampaPackingListCollo(szOrdKey,nCollo,szPrinter,NULL,FALSE);
                    StampaPackingListColloPrefincato(szPrinter,szOrdKey,nCollo,FALSE);
#ifdef TRACE
                    trace_debug(TRUE, TRUE, "PRINT_PACKING_LIST_COLLO - FINE");
#endif
                }
                break;
                case PRINT_PACKING_LIST_COLLO_PREFINCATO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];
                    int nCollo = 0;

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Collo   */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ nCollo=atoi(pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

#ifdef TRACE
                    trace_debug(TRUE, TRUE, "PRINT_PACKING_LIST_COLLO [%s] [%d] on [%s]", szOrdKey,nCollo,szPrinter);
#endif
                    /*
                    * Stampa packing list di un collo all'interno di un ordine
                    */
                    StampaPackingListColloPrefincato(szPrinter,szOrdKey,nCollo,FALSE);
#ifdef TRACE
                    trace_debug(TRUE, TRUE, "PRINT_PACKING_LIST_COLLO - FINE");
#endif
                }
                break;

                case CREATE_PACKING_LIST_COLLO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];
                    char szFile[128];
                    int nCollo = 0;
                    int nRC;

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Collo   */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ nCollo=atoi(pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }
                    /* File */    if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szFile,pPtr); }

#ifdef TRACE
                    trace_debug(TRUE, TRUE, "CREATE_PACKING_LIST_COLLO [%s] [%s] [%d] on [%s]", szFile,szOrdKey,nCollo,szPrinter);
#endif
                    pDatiLinea->bStampa=TRUE;
                    /*
                    * Stampa packing list di un collo all'interno di un ordine
                    */
                    nRC=CreatePackingListColloPrefincato(szPrinter,szOrdKey,nCollo,szFile,FALSE);
#ifdef TRACE
                    trace_debug(TRUE, TRUE, "CREATE_PACKING_LIST_COLLO - FINE [%d]",nRC);
#endif
                    pDatiLinea->bStampa=FALSE;
                    if(!nRC){
                        /* tutto OK */
                        SendMessage(InMsgStruct.srce, nPID,PRINTLABEL_PACKING_LIST_CREATED, szFile);
                    } else {
                        /* Errore */
                        SendMessage(InMsgStruct.srce, nPID,PRINTLABEL_PACKING_LIST_NOT_CREATED, NULL);
                    }
                }
                break;

                case CREATE_XAB_A4:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];
                    char szFile[128];
                    char szBuffer[128];
                    int nCollo = 0;
                    int nRC;

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Collo   */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ nCollo=atoi(pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }
                    /* File */    if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szFile,pPtr); }

#ifdef TRACE
                    trace_debug(TRUE, TRUE, "CREATE_XAB_A4 [%s] [%s] [%d] on [%s]", szFile,szOrdKey,nCollo,szPrinter);
#endif
                    pDatiLinea->bStampa=TRUE;
                    /*
                    * Stampa packing list di un collo all'interno di un ordine
                    */
                    nRC=CreateXAB_A4(szPrinter,szOrdKey,szFile,FALSE);
#ifdef TRACE
                    trace_debug(TRUE, TRUE, "CREATE_XAB_A4 - FINE [%d]",nRC);
#endif
                    pDatiLinea->bStampa=FALSE;

                    sprintf(szBuffer,"%s,%d,%s",szOrdKey,nCollo,szFile);
                    if(nRC){
                        /* tutto OK */
                        SendMessage(InMsgStruct.srce, nPID,PRINTLABEL_XAB_A4_CREATED, szBuffer);
                    } else {
                        /* Errore */
                        SendMessage(InMsgStruct.srce, nPID,PRINTLABEL_XAB_A4_NOT_CREATED, szBuffer);
                    }
                }
                break;

                case PRINT_XAB:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

                    pDatiLinea->bStampa=TRUE;
                    /*
                    * Stampa di XAB per chiave ordine di spedizione
                    */
                    /* Prova */
                    //if(StampaDCSpedizione(szPrinter,szOrdKey,FALSE))
                    /* Prova */
                    if(StampaXABSpedizione(szPrinter,szOrdKey,FALSE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_STAMPATA, szOrdKey);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_ERROR, szOrdKey);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;
                case PRINT_XAB_STORICO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }
                    pDatiLinea->bStampa=TRUE;

                    /*
                    * Stampa di XAB per chiave ordine di spedizione 
                    */
                    if(StampaXABSpedizione(szPrinter,szOrdKey,TRUE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_STAMPATA, szOrdKey);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_ERROR, szOrdKey);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;

                case PRINT_XAB_A4:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

                    pDatiLinea->bStampa=TRUE;
                    /*
                    * Stampa di XAB per chiave ordine di spedizione
                    */
                    if(StampaXABSpedizione_A4(szPrinter,szOrdKey,FALSE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_STAMPATA, szOrdKey);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_ERROR, szOrdKey);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;

                case PRINT_XAB_A4_LINEA:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szNMCED[128];
                    char szTPSPE[128];
                    char szCDLIN[128];
                    char szCDVET[128];
                    char szLinea[128];
                    char szPrinter[128];

                    if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }   /* Numero Cedola */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szTPSPE,pPtr); }     /* Tipo Spedizione */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDLIN,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDVET,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }   /* Printer */ 

                    /*
                    * Stampa di XAB per linea spedizione
                    */
                    sprintf(szLinea,"%s,%s,%s,%s",szNMCED,szTPSPE,szCDLIN,szCDVET);

                    pDatiLinea->bStampa=TRUE;
                    //if(StampaXABLinea(szPrinter,szNMCED,szTPSPE,szCDLIN,FALSE)){
                    if(StampaPackingListLinea(szPrinter,szNMCED,szTPSPE,szCDLIN,szCDVET,FALSE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_STAMPATA, szLinea);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_ERROR, szLinea);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;
                case PRINT_XAB_A4_LINEA_STORICO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szNMCED[128];
                    char szTPSPE[128];
                    char szCDLIN[128];
                    char szCDVET[128];
                    char szLinea[128];
                    char szPrinter[128];

                    if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }   /* Numero Cedola */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szTPSPE,pPtr); }     /* Tipo Spedizione */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDLIN,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDVET,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }   /* Printer */ 

                    /*
                    * Stampa di XAB per linea spedizione
                    */
                    sprintf(szLinea,"%s,%s,%s,%s",szNMCED,szTPSPE,szCDLIN,szCDVET);

                    pDatiLinea->bStampa=TRUE;
                    //if(StampaXABLinea(szPrinter,szNMCED,szTPSPE,szCDLIN,FALSE)){
                    if(StampaPackingListLinea(szPrinter,szNMCED,szTPSPE,szCDLIN,szCDVET,TRUE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_STAMPATA, szLinea);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_ERROR, szLinea);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;


                case PRINT_XAB_STORICO_A4:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

                    pDatiLinea->bStampa=TRUE;
                    /*
                    * Stampa di XAB per chiave ordine di spedizione 
                    */
                    if(StampaXABSpedizione_A4(szPrinter,szOrdKey,TRUE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_STAMPATA, szOrdKey);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_ERROR, szOrdKey);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;

                case PRINT_XAB_LINEA:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szNMCED[128];
                    char szTPSPE[128];
                    char szCDLIN[128];
                    char szLinea[128];
                    char szPrinter[128];

                    if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }   /* Numero Cedola */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szTPSPE,pPtr); }     /* Tipo Spedizione */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDLIN,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }   /* Printer */ 

                    /*
                    * Stampa di XAB per linea spedizione
                    */
                    sprintf(szLinea,"%s,%s,%s",szNMCED,szTPSPE,szCDLIN);

                    pDatiLinea->bStampa=TRUE;
                    if(StampaXABLinea(szPrinter,szNMCED,szTPSPE,szCDLIN,FALSE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_STAMPATA, szLinea);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_ERROR, szLinea);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;

                case PRINT_XAB_LINEA_STORICO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szNMCED[128];
                    char szTPSPE[128];
                    char szCDLIN[128];
                    char szLinea[128];
                    char szPrinter[128];

                    if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }   /* Numero Cedola */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szTPSPE,pPtr); }     /* Tipo Spedizione */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDLIN,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }   /* Printer */ 

                    /*
                    * Stampa di XAB per linea spedizione
                    */
                    sprintf(szLinea,"%s,%s,%s",szNMCED,szTPSPE,szCDLIN);

                    pDatiLinea->bStampa=TRUE;
                    if(StampaXABLinea(szPrinter,szNMCED,szTPSPE,szCDLIN,TRUE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_STAMPATA, szLinea);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_ERROR, szLinea);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;

                case PRINT_DC:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }

                    /*
                    * Stampa di XAB per chiave ordine di spedizione
                    */
                    if(StampaDCSpedizione(szPrinter,szOrdKey,FALSE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_STAMPATA, szOrdKey);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_ERROR, szOrdKey);
                    }
                }
                break;

                case PRINT_DC_STORICO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szOrdKey[128];
                    char szPrinter[128];

                    /* OrdKey  */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdKey,pPtr); }
                    /* Printer */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }
                    pDatiLinea->bStampa=TRUE;

                    if(StampaDCSpedizione(szPrinter,szOrdKey,TRUE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_STAMPATA, szOrdKey);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_ERROR, szOrdKey);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;

                case PRINT_DC_LINEA:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szNMCED[128];
                    char szTPSPE[128];
                    char szCDLIN[128];
                    char szCDVET[128];
                    char szLinea[128];
                    char szPrinter[128];

                    if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }   /* Numero Cedola */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szTPSPE,pPtr); }     /* Tipo Spedizione */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDLIN,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDVET,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }   /* Printer */ 

                    /*
                    * Stampa di XAB per linea spedizione
                    */
                    sprintf(szLinea,"%s,%s,%s,%s",szNMCED,szTPSPE,szCDLIN,szCDVET);

                    pDatiLinea->bStampa=TRUE;
                    if(StampaDCLinea(szPrinter,szNMCED,szTPSPE,szCDLIN,szCDVET,FALSE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_STAMPATA, szLinea);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_ERROR, szLinea);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;

                case PRINT_DC_LINEA_STORICO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szNMCED[128];
                    char szTPSPE[128];
                    char szCDLIN[128];
                    char szCDVET[128];
                    char szLinea[128];
                    char szPrinter[128];

                    if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }   /* Numero Cedola */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szTPSPE,pPtr); }     /* Tipo Spedizione */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDLIN,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDVET,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }   /* Printer */ 

                    /*
                    * Stampa di XAB per linea spedizione
                    */
                    sprintf(szLinea,"%s,%s,%s,%s",szNMCED,szTPSPE,szCDLIN,szCDVET);

                    pDatiLinea->bStampa=TRUE;
                    if(StampaDCLinea(szPrinter,szNMCED,szTPSPE,szCDLIN,szCDVET,TRUE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_STAMPATA, szLinea);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_XAB_LINEA_ERROR, szLinea);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;

                case PRINT_DISTINTA_STORICO:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szNMCED[128];
                    char szTPSPE[128];
                    char szCDLIN[128];
                    char szCDVET[128];
                    char szLinea[128];
                    char szPrinter[128];
                    char szANNO[128];

                    if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }   /* Numero Cedola */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szTPSPE,pPtr); }     /* Tipo Spedizione */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDLIN,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDVET,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szANNO,pPtr); }      /* ANNO */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }   /* Printer */ 
                    sprintf(szLinea,"%s,%s,%s,%s,%s",szNMCED,szTPSPE,szCDLIN,szCDVET,szANNO);

                    pDatiLinea->bStampa=TRUE;
                    if (StampaDistinta(szPrinter,szNMCED,szTPSPE,szCDLIN,szCDVET,szANNO,TRUE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_DISTINTA_STAMPATA, szLinea);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_DISTINTA_ERROR, szLinea);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;

                case PRINT_DISTINTA:
                {
                    char  *pPtr;
                    char  *szSeparator=",\t\n";
                    char szNMCED[128];
                    char szTPSPE[128];
                    char szCDLIN[128];
                    char szCDVET[128];
                    char szLinea[128];
                    char szPrinter[128];
                    char szANNO[128];

                    if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }   /* Numero Cedola */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szTPSPE,pPtr); }     /* Tipo Spedizione */
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDLIN,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDVET,pPtr); }     /* Codice Linea */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szANNO,pPtr); }      /* ANNO */ 
                    if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szPrinter,pPtr); }   /* Printer */ 
                    sprintf(szLinea,"%s,%s,%s,%s,%s",szNMCED,szTPSPE,szCDLIN,szCDVET,szANNO);

                    pDatiLinea->bStampa=TRUE;
                    if (StampaDistinta(szPrinter,szNMCED,szTPSPE,szCDLIN,szCDVET,szANNO,FALSE)){
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_DISTINTA_STAMPATA, szLinea);
                    } else {
                        SendMessage(InMsgStruct.srce, nPID, PRINTLABEL_DISTINTA_ERROR, szLinea);
                    }
                    pDatiLinea->bStampa=FALSE;
                }
                break;

            }
        }
        usleep(Cfg.nDelay);
    }

    SafeIntFunc(0);
}    /* End of main() */



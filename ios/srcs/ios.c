/*
* ios-plc.c : gestione luci
* 
* Progetto Easy Picking 4.0 : IMS-VIDEO
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*
*/
#include <glib.h>
#include <gio/gio.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
//#include <sys/mman.h>


#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#include <trace.h>

#include <ep-common.h>
#include <proc_list.h>
#include <picking.h>

#include "cmp_time.h"

#include "ios.h"

int nAllMsg;
int nCarIn;
ep_msg_header_t InMsgStruct;
char szInMsg[INFO_LEN];
char szText[INFO_LEN];
char szBufTrace[80];

char szTimeBuffer[80];
char szDateBuffer[80];


int nPID=0;                   /* Process ID */
CFGSTRUCT    Cfg;                /* struttura della configurazione */

int nAMATRIX ;
int nARIGA   ;
int nACOLON  ;
int nACCEN   ;
int nRACCEN  ;

int nNMATRIX ;
int nNRIGA   ;
int nNCOLON  ;
int nANNUL   ;
int nRANNUL  ;

int nSMATRIX ;
int nSRIGA   ;
int nSCOLON  ;
int nSPEGNI  ;
int nRSPEGNI ;
int nRESETL  ;
int nRRESETL ;
int nCONGELA ;


/*
* ReadConfiguration()
* lettura della configurazione dal file cni.cfg
*/
void ReadConfiguration(char *szCfgFileName)
{
    char szParagraph[128];
    char szCurrentDirectory[128];
    char szCfg[128];
    char szProcIndex[128];
    char szProcBuffer[128];
    int nIndex;

    /*
    * leggo il file di configurazione dalla directory corrente
    */
    getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
    
    strcpy(szCfg,szCurrentDirectory);
    strcat(szCfg,"/");
    strcat(szCfg,szCfgFileName);
    /*
    * lettura del file di configurazione
    */
    strcpy(szParagraph,"General Settings");
    Cfg.nDebugLevel=GetFileInt(szParagraph,"DebugLevel",0,szCfg,NULL);

    GetFileString(szParagraph,"PathTrace",       "../trace",    Cfg.szPathTrace,       80, szCfg, NULL);
    /*
    * Delay in millisecondi per la simulazione pressione tasto
    */
    Cfg.nDelaySimulazione=GetFileInt(szParagraph, "DelaySimulazione",100,szCfg,NULL);

    /*
    * Simulazione IOS
    */
    Cfg.nSimulazione=GetFileInt(szParagraph, "Simulazione",0,szCfg,NULL);

    ReadProcInfo(szCfgFileName);

    /*
    * parametri specifici dell'applicazione
    */
    /*
    * PLC Presente
    */
    Cfg.nPLC=GetFileInt("PLC","PLC",1,szCfg,NULL);
    GetFileString("PLC","PLCHost","localhost",Cfg.szPLCHost,80,szCfg,NULL);
    Cfg.nServicePort=GetFileInt("PLC","ServicePort",8050,szCfg,NULL);
    /*
    * Delay in millisecondi 
    */
    Cfg.nDelay=GetFileInt(ProcessGetName(PROC_IOS),"Delay",100,szCfg,NULL);
    /*
    * Numero delle schede IOS connesse (da aprire)
    */
    Cfg.nIOS=GetFileInt(ProcessGetName(PROC_IOS),"IOSNumber",1,szCfg,NULL);


    /*
    * Lettura Lista variabili PLC
    */
    plc_read_vars_list(szCfg);

}

/*
* WaitIOS()
* attendo che il codice comando in TX del modulo 
* e della IOS passate in ingresso sia libero (0)
* rm 06-09-2001 : aggiunta gestione timeout
*/
void WaitIOS(int nIOS,int nCPU,int nModulo)
{
#ifdef HAS_IOS_CNI
    struct timeval tvActualTime;
    struct timeval tvStartTime;
    int nTimePassed=0;    /* msecs */

    gettimeofday(&tvStartTime,NULL);

    while(nTimePassed < IOS_TIMEOUT && IOS_TX_BYTE(nIOS,nCPU,nModulo,0)){
        gettimeofday(&tvActualTime,NULL);

        nTimePassed=(tvActualTime.tv_sec-tvStartTime.tv_sec)*1000;     /* secondi */
        nTimePassed+=(tvActualTime.tv_usec-tvStartTime.tv_usec)/1000;   /* milli secondi */
#ifdef TRACE_TIME
        trace_debug(FALSE, TRUE, "TimePassed = %6d",nTimePassed);
#endif
        usleep(1000*Cfg.nDelay);
    }
    if(nTimePassed > IOS_TIMEOUT){
        char szMsg[128];

#ifdef TRACE
        trace_debug(TRUE, TRUE, "Errore di TIMEOUT su IOS [%d.%d.%d]",nIOS,nCPU,nModulo);
#endif
        sprintf(szMsg, "Errore di TIMEOUT su IOS [%d.%d.%d]\n",nIOS,nCPU,nModulo);
        SendMessage(PROC_MAIN, PROC_IOS, DISPLAY_MSG, szMsg);
    }
#endif
}

void WaitPLCVar(int nPLCVar,int nValue)
{
    struct timeval tvActualTime;
    struct timeval tvStartTime;
    int nTimePassed=0;    /* msecs */

    gettimeofday(&tvStartTime,NULL);

    /* attualizzo le var plc */
    if(Cfg.nPLC){
        plc_refresh_vars();
    }

    /* attendo la risposta al comando plcvar */
    while(nTimePassed < PLC_TIMEOUT && plc_var_value(nPLCVar)!=nValue){
        gettimeofday(&tvActualTime,NULL);

        nTimePassed+=(tvActualTime.tv_sec-tvStartTime.tv_sec)*1000;     /* secondi */
        nTimePassed+=(tvActualTime.tv_usec-tvStartTime.tv_usec)/1000;   /* milli secondi */
#ifdef TRACE_TIME
        trace_debug(FALSE, FALSE, ".");
#endif
        usleep(1000*Cfg.nDelay);
        /* attualizzo le var plc */
        if(Cfg.nPLC){
            plc_refresh_vars();
        }
    }
    if(nTimePassed > PLC_TIMEOUT){
        char szMsg[128];

#ifdef TRACE
        trace_debug(TRUE, TRUE, "Errore di TIMEOUT su fronte salita PLC [%s]",plc_get_var_name(nPLCVar));
#endif
        sprintf(szMsg, "Errore di TIMEOUT su fronte salita PLC [%s]",plc_get_var_name(nPLCVar));
        SendMessage(PROC_MAIN, PROC_IOS, DISPLAY_MSG, szMsg);
    }
}


void GetVarsIndex(void)
{
    char szBuffer[128];

    sprintf(szBuffer,"AMATRIX"); if((nAMATRIX  = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"ARIGA");   if((nARIGA    = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"ACOLON");  if((nACOLON   = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"NMATRIX"); if((nNMATRIX  = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"NRIGA");   if((nNRIGA    = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"NCOLON");  if((nNCOLON   = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"ACCEN");   if((nACCEN    = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"RACCEN");  if((nRACCEN   = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"ANNUL");   if((nANNUL    = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"RANNUL");  if((nRANNUL   = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"SMATRIX"); if((nSMATRIX  = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"SRIGA");   if((nSRIGA    = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"SCOLON");  if((nSCOLON   = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"SPEGNI");  if((nSPEGNI   = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"RSPEGNI"); if((nRSPEGNI  = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"RESETL");  if((nRESETL   = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"RRESETL"); if((nRRESETL  = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
    sprintf(szBuffer,"CONGELA"); if((nCONGELA  = plc_get_var_index(szBuffer))==-1){ trace_debug(TRUE, TRUE, "Variabile %s assente !\n",szBuffer); }
}

/* 
* main() 
*/
int main(int argc, char **argv)
{
    int nRC;
    int nIndex;
    int nCPUIndex;
    int nIOS,nCPU,nModule,nRowIndex,nColIndex;
    int nBlinkState;
    char szSendBuffer[128];
    char szMsg[128];
    int nOpt;
    int nPLCError;

    while ((nOpt = getopt(argc, argv, "p:")) != -1) {
        switch(nOpt){
            case 'p':    /* Process ID */
                nPID=atoi(optarg);
            break;
        }
    }
    if(nPID==0){
        nPID=PROC_IOS;
    }

    /*
    * Lettura della configurazione corrente
    */
    ReadConfiguration(__configuration_file__);

#ifdef TRACE
    /* Apro il file trace */
    sprintf(szBufTrace,"%s/%s%s",Cfg.szPathTrace,__module_name__,TRACE_FILE_SUFFIX);
    open_trace(__module_name__, szBufTrace, PICKING_TRACE_FILE_MAXSIZE);

    trace_debug(FALSE, TRUE, "Module Name     : %s",__module_name__);
    trace_debug(FALSE, TRUE, "Version         : %s",__version__);
    trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
    trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
    trace_debug(FALSE, TRUE, "Simulazione     : %d",Cfg.nSimulazione);

    trace_debug(TRUE, TRUE, "Started");
#endif

    /* Apro la coda messaggi principale */
    if((OpenProcessMsgQ(PROC_MAIN))<0){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "%s : Apertura coda messaggi principale fallita", __module_name__);
#endif
    }
    /* Apro la coda messaggi processo Settori */
    if((OpenProcessMsgQ(PROC_SETTORI))<0){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "%s : Apertura coda messaggi settori fallita", __module_name__);
#endif
    }
    /* Apro la coda messaggi locale */
    if((CreateProcessMsgQ( nPID, 1))<0){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "%s : Creazione coda messaggi locale fallita", __module_name__);
#endif
    }


    /* Predispongo l'utilizzo dell'uscita di sicurezza */
    signal(SIGTERM, SafeIntFunc);


#ifdef HAS_IOS_CNI
    for(nIndex=0;nIndex<Cfg.nIOS;nIndex++){
        Cfg.nFdIOS[nIndex]=0xff;    /* NON NULL VALUE */
        nRC=iosOpenGeneral(nIndex,&(Cfg.IOSInfo[nIndex]),&Cfg.nFdIOS[nIndex]);
        /*
        * gestire il valore di ritorno ....
        */
        if(nRC==-1){
#ifdef TRACE
            trace_debug(FALSE, TRUE, "Errore in apertura Scheda IOS Numero [%2d]",nIndex);
#endif
            sprintf(szMsg,"Errore in apertura Scheda IOS Numero [%2d]\n",nIndex);
            SendMessage(PROC_MAIN, PROC_IOS, DISPLAY_MSG, szMsg);
            exit(1);
        } else {
#ifdef TRACE
            trace_debug(TRUE, TRUE, "Scheda IOS Numero %2d - Versione : %c%c%c%c",
                nIndex,
                Cfg.IOSInfo[nIndex].base[0x225],
                Cfg.IOSInfo[nIndex].base[0x226],
                Cfg.IOSInfo[nIndex].base[0x227],
                Cfg.IOSInfo[nIndex].base[0x228]);
#endif
            for (nCPUIndex = 0; nCPUIndex < MAX_IOS_CPU; nCPUIndex++) {
                if (Cfg.IOSInfo[nIndex].cpu[nCPUIndex]) {
                    /* abilita il riaggancio */
                    Cfg.IOSInfo[nIndex].cpu[nCPUIndex][IOS_HANDSHAKE]=(BYTE)0;
                    sleep(1);
                    /* disabilita il riaggancio */
                    Cfg.IOSInfo[nIndex].cpu[nCPUIndex][IOS_HANDSHAKE]=(BYTE)0x55;
                }
            }
        }
    }
#endif



    /*
    * Apertura del canale di comunicazione con il PLC server (QPLC)
    */
    if(Cfg.nPLC){
        if((nPLCError=plc_connect(Cfg.szPLCHost,Cfg.nServicePort))==FALSE){
            Cfg.nPLC=0;
#ifdef TRACE
            trace_debug(FALSE, TRUE, "%s : Connessione al PLC Fallita : errore [%d]", __module_name__,nPLCError);
#endif
            sprintf(szMsg, "Connessione al PLC Fallita : errore [%d]", nPLCError);
            SendMessage(PROC_MAIN, PROC_IOS, DISPLAY_MSG, szMsg);
        }
    }

    /*
    * Memorizzo gli indici nel vettore interno
    */

    if(Cfg.nPLC){
        GetVarsIndex();

        plc_set_var_value(nAMATRIX,FALSE);
        plc_set_var_value(nARIGA,FALSE);
        plc_set_var_value(nACOLON,FALSE);
        plc_set_var_value(nACCEN,FALSE);
        plc_set_var_value(nANNUL,FALSE);
        plc_set_var_value(nRSPEGNI,FALSE);
        plc_set_var_value(nRESETL,FALSE);
    }


    /*
    * avverto MAIN dello start
    */
    SendMessage(PROC_MAIN, PROC_IOS, PROGRAM_STARTED, NULL);


    /*
    * gestione program loop
    */
    for(;;){
        /*
        * Richiesta informazioni sulle variabili registrate per il DEBUG
        */
        if(Cfg.nPLC){
            plc_refresh_vars();
        }

        /*
        * Gestione Messaggi Ricevuti
        */
        nAllMsg = 0;
        if((nCarIn = ProcessReceiveMsgNoWait(PROC_IOS, &nAllMsg, szInMsg))>=0){ 
            UnpackMessage(szInMsg, &InMsgStruct, szText, nCarIn);

#ifdef TRACE
            if(Cfg.nDebugLevel){
                trace_debug(TRUE, TRUE, "Msg [code=%d, srce=%d, dest=%d] %s%s%s from [%s]",
                    InMsgStruct.code,
                    InMsgStruct.srce,
                    InMsgStruct.dest,
                    strlen(szText)?"[":"",strlen(szText)? szText : "",strlen(szText)?"]":"",
                    ProcessGetName(InMsgStruct.srce));
            }
#endif

            switch (InMsgStruct.srce) {
                /* 
                * gestisco i messaggi provenienti da MAIN e SETTORI 
                */
                case PROC_SETTORI:
                case PROC_MAIN:

                    switch(InMsgStruct.code){
                        case IOS_PKL_SET_RC:
                            /* 
                            * Parametri : Scheda IOS,CPU,Modulo,Riga,Colonna
                            */
                            sscanf(szText,"%d,%d,%d,%d,%d",&nIOS,&nCPU,&nModule,&nRowIndex,&nColIndex);

#ifdef TRACE
                            trace_debug(TRUE, TRUE, "IOS_PKL_SET_RC [IOS:%d,CPU:%d,M:%2d,R:%2d,C:%2d]", nIOS,nCPU,nModule,nRowIndex,nColIndex);
#endif
                            if(Cfg.nPLC){
                                plc_set_var_value(nAMATRIX,nModule);
                                plc_set_var_value(nARIGA,nRowIndex);
                                plc_set_var_value(nACOLON,nColIndex);

                                plc_set_var_value(nACCEN,TRUE);
                                WaitPLCVar(nRACCEN,TRUE);
                                plc_set_var_value(nACCEN,FALSE);
                                
                                /* attendo la discesa di RACCEN */
                                WaitPLCVar(nRACCEN,FALSE);
                            }
                            /*
                            * se sono in simulazione restituisco la pressione tasto
                            */
                            if(Cfg.nSimulazione){
                                usleep(1000*Cfg.nDelaySimulazione);

#ifdef TRACE
                                trace_debug(TRUE, TRUE, "SIM: PKL_BUTTON_PRESSED [IOS:%d,CPU:%d,M:%2d,R:%2d,C:%2d]", nIOS,nCPU,nModule,nRowIndex,nColIndex);
#endif
                                sprintf(szSendBuffer,"%d,%d,%d,%d,%d",nIOS,nCPU,nModule,nRowIndex,nColIndex);

                                SendMessage(PROC_SETTORI, PROC_IOS, IOS_PKL_BUTTON_PRESSED, szSendBuffer);
                            }
                        break;
                        case IOS_PKL_RESET_RC:
                            /* 
                            * Parametri : Scheda IOS,CPU,Modulo,Riga,Colonna
                            */
                            sscanf(szText,"%d,%d,%d,%d,%d",&nIOS,&nCPU,&nModule,&nRowIndex,&nColIndex);
#ifdef TRACE
                            trace_debug(TRUE, TRUE, "IOS_PKL_RESET_RC [IOS:%d,CPU:%d,M:%2d R:%2d C:%2d]", nIOS, nCPU, nModule,nRowIndex,nColIndex);
#endif
                            if(Cfg.nPLC){
                                plc_set_var_value(nNMATRIX,nModule);
                                plc_set_var_value(nNRIGA,nRowIndex);
                                plc_set_var_value(nNCOLON,nColIndex);

                                plc_set_var_value(nANNUL,TRUE);
                                WaitPLCVar(nRANNUL,TRUE);
                                plc_set_var_value(nANNUL,FALSE);
                                /* attendo la discesa di RANNUL */
                                WaitPLCVar(nRANNUL,FALSE);
                            }

                        break;
                        case IOS_PKL_CONGELA:
                            /* 
                            * Parametri : Scheda IOS,CPU,Modulo
                            */
                            sscanf(szText,"%d,%d,%d",&nIOS,&nCPU,&nModule);
#ifdef TRACE
                            trace_debug(TRUE, TRUE, "IOS_PKL_CONGELA [IOS:%d,CPU:%d,M:%2d]", nIOS, nCPU, nModule);
#endif
                            Cfg.nCongela=TRUE;
                            if(Cfg.nPLC){
                                plc_set_var_value(nCONGELA,TRUE);
                            }
                        break;
                        case IOS_PKL_SCONGELA:
                            /* 
                            * Parametri : Scheda IOS,CPU,Modulo
                            */
                            sscanf(szText,"%d,%d,%d",&nIOS,&nCPU,&nModule);
#ifdef TRACE
                            trace_debug(TRUE, TRUE, "IOS_PKL_SCONGELA [IOS:%d,CPU:%d,M:%2d]", nIOS, nCPU, nModule);
#endif
                            Cfg.nCongela=FALSE;
                            if(Cfg.nPLC){
                                plc_set_var_value(nCONGELA,FALSE);
                            }

                        break;
                        case IOS_PKL_RESET_ALL:
                            /* 
                            * Parametri : Scheda IOS,CPU,Modulo
                            */
                            sscanf(szText,"%d,%d,%d",&nIOS,&nCPU,&nModule);
#ifdef TRACE
                            trace_debug(TRUE, TRUE, "IOS_PKL_RESET_ALL [IOS:%d,CPU:%d,M:%2d]", nIOS, nCPU, nModule);
#endif
                            if(Cfg.nPLC){
                                plc_set_var_value(nRESETL,TRUE);
                                WaitPLCVar(nRRESETL,TRUE);
                                plc_set_var_value(nRESETL,FALSE);
                                /* attendo la discesa di RRESETL */
                                WaitPLCVar(nRRESETL,FALSE);
                            }
                        break;
                        case IOS_PKL_TEST:
                            /* 
                            * Parametri : Scheda IOS,CPU,Modulo
                            */
                            sscanf(szText,"%d,%d,%d",&nIOS,&nCPU,&nModule);
                        break;

                        case IOS_DPY_CHAR:
                        {
                            int nDigit;
                            int nComando;
                            int nDPYByte[4];

                            /* ricavo i dati da visualizzare */
                            sscanf(szText,"%d,%d,%d,%d,%d,%c",
                                &nIOS,
                                &nCPU,
                                &nModule,
                                &nDigit,
                                &nBlinkState,
                                (char *)&(nDPYByte[0]));


                            switch(nBlinkState){
                                case 0: default: nComando = DPY_ASCII_DGT; break;
                                case 1: nComando = DPY_ASCII_DGT_BLINK; break;
                            }

                            WaitIOS(nIOS,nCPU,nModule);
#ifdef TRACE
                            trace_debug(TRUE, TRUE, "IOS_DPY_CHAR [IOS:%d,CPU:%d,M:%2d D%d:%c]", nIOS, nCPU, nModule,nDigit, nDPYByte[0]);
#endif

#ifdef HAS_IOS_CNI
                            if(nIOS<Cfg.nIOS){
                                IOS_TX_BYTE(nIOS,nCPU,nModule,1)=(BYTE)nDigit;           /* digit */
                                IOS_TX_BYTE(nIOS,nCPU,nModule,2)=(BYTE)nDPYByte[0];      /* ascii */
                                IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)nComando;         /* comando */
                            }
#endif
                            WaitIOS(nIOS,nCPU,nModule);
                        }
                        break;

                        case IOS_DPY_STRING:
                        {
                            int nIndex;
                            int nComando;
                            int nDPYByte[4];

                            /* ricavo i dati da visualizzare */
                            sscanf(szText,"%d,%d,%d,%d,%d,%x,%x,%x,%x",
                                &nIOS,
                                &nCPU,
                                &nModule,
                                &nRowIndex,
                                &nBlinkState,
                                &nDPYByte[0],
                                &nDPYByte[1],
                                &nDPYByte[2],
                                &nDPYByte[3]);

                            switch(nBlinkState){
                                case 0: default: nComando = DPY_ASCII_DGT; break;
                                case 1: nComando = DPY_ASCII_DGT_BLINK; break;
                            }
#ifdef TRACE
                            trace_debug(TRUE, TRUE, "IOS_DPY_STRING [IOS:%d,CPU:%d,M:%2d R%d:%c%c%c%c]", nIOS, nCPU, nModule, nRowIndex, nDPYByte[0], nDPYByte[1], nDPYByte[2], nDPYByte[3]);
#endif

                            for(nIndex=0;nIndex<2;nIndex++){
                                
                                WaitIOS(nIOS,nCPU,nModule);

#ifdef HAS_IOS_CNI
                                if(nIOS<Cfg.nIOS){
                                    IOS_TX_BYTE(nIOS,nCPU,nModule,1)=(BYTE)((nRowIndex*4)+nIndex*2); /* digit */
                                    IOS_TX_BYTE(nIOS,nCPU,nModule,2)=(BYTE)nDPYByte[nIndex*2];       /* ascii */
                                    IOS_TX_BYTE(nIOS,nCPU,nModule,3)=(BYTE)nDPYByte[nIndex*2+1];     /* ascii */
                                    IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)nComando;                 /* comando */
                                }
#endif
                                WaitIOS(nIOS,nCPU,nModule);

                            }
                        }
                        break;

                        case IOS_DPY_BCD:
                        {
                            int nComando=-1;
                            int nDPYByte[4];

                            /* ricavo i dati da visualizzare */
                            sscanf(szText,"%d,%d,%d,%d,%x,%x,%x",
                                &nIOS,
                                &nCPU,
                                &nModule,
                                &nRowIndex,
                                &nDPYByte[0],
                                &nDPYByte[1],
                                &nDPYByte[2]);

                            switch(nRowIndex){
                                case 0: nComando=DPY_BCD_03_FLAG; break;
                                case 1: nComando=DPY_BCD_47_FLAG; break;
                            }
#ifdef TRACE
                            trace_debug(TRUE, TRUE, "IOS_DPY_BCD (%x) [IOS:%d,CPU:%d,M:%2d R%d:%x:%x:%x]", nComando, nIOS, nCPU, nModule, nRowIndex, nDPYByte[0], nDPYByte[1], nDPYByte[2]);
#endif
                            WaitIOS(nIOS,nCPU,nModule);

#ifdef HAS_IOS_CNI
                            if(nIOS<Cfg.nIOS){
                                IOS_TX_BYTE(nIOS,nCPU,nModule,1)=(BYTE)nDPYByte[0];    /* Digit 0-1 o 4-5 */
                                IOS_TX_BYTE(nIOS,nCPU,nModule,2)=(BYTE)nDPYByte[1];    /* Digit 2-3 o 6-7 */
                                IOS_TX_BYTE(nIOS,nCPU,nModule,3)=(BYTE)nDPYByte[2];    /* Flags per blink e '.' */
                                IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)nComando;                 /* comando */
                            }
#endif
                            
                            WaitIOS(nIOS,nCPU,nModule);

                        }
                        break;

                    }
                break;
            }
        }

        /* controllo tasti premuti */
        if(plc_var_value(nSPEGNI)){
            nIOS=0;
            nCPU=0;
            nModule=plc_var_value(nSMATRIX);
            nRowIndex=plc_var_value(nSRIGA);
            nColIndex=plc_var_value(nSCOLON);

#ifdef TRACE
            trace_debug(TRUE, TRUE, "PKL_BUTTON_PRESSED [IOS:%d,CPU:%d,M:%2d,R:%2d,C:%2d]", nIOS,nCPU,nModule,nRowIndex,nColIndex);
#endif

            sprintf(szSendBuffer,"%d,%d,%d,%d,%d",nIOS,nCPU,nModule,nRowIndex,nColIndex);

            SendMessage(PROC_SETTORI,  PROC_IOS, IOS_PKL_BUTTON_PRESSED, szSendBuffer);

            /* rispondo a PLC  e attendo */
            plc_set_var_value(nRSPEGNI,TRUE);
            WaitPLCVar(nSPEGNI,FALSE);
            plc_set_var_value(nRSPEGNI,FALSE);
        }



        usleep(1000*Cfg.nDelay);
    }     /* fine ciclo */
    SafeIntFunc();
    return (0);
}




/*
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc()
{
int nIndex;

#ifdef TRACE
    trace_debug(TRUE, TRUE, "Stopped");
#endif
#ifdef HAS_IOS_CNI
    /*
    * chiusura IOS
    */
    for(nIndex=0;nIndex<Cfg.nIOS;nIndex++){
        close(Cfg.nFdIOS[nIndex]);
    }
#endif

    /*
    * avverto MAIN dello stop
    */
    SendMessage(PROC_MAIN, PROC_IOS, PROGRAM_STOPPED, NULL);

    exit(1);
} /* Fine SafeIntFunc */

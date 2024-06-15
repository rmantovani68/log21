/*
* bilancia.c : gestione Bilancia modello Alfa Bilici
* 
* Progetto Easy Picking 4.0
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <glib.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#ifdef TRACE
    #include <trace.h>
#endif

#include <proc_list.h>
#include <ep-common.h>

#include "bilancia.h"

int nAllMsg;
int nCarIn;
ep_msg_header_t InMsgStruct;
char szInMsg[INFO_LEN];
char szText[INFO_LEN];
char szBufTrace[80];
char *pszNomeModulo;
char szRcvBuf[256];

char szTimeBuffer[80];
char szDateBuffer[80];


int nPID=0;                   /* Process ID */
int    nRxPId=0;            
CFGSTRUCT    Cfg;                /* struttura della configurazione */

extern char rcsid[];
extern char __version__[];
extern char __customer__[];
extern char __configuration_file__[];



/*
* ReadConfiguration()
* lettura della configurazione dal file cni.cfg
*/
void ReadConfiguration(void)
{
    char szParagraph[128];
    char szCurrentDirectory[128];
    char szProcIndex[128];
    char szProcBuffer[128];
    char szBuffer[128];
    char  *pPtr;
    char  *szSeparator=",\t\n";
    int nIndex;
    char szCfgFileName[128];

    /*
    * leggo il file di configurazione dalla directory corrente
    */
    getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
    
    strcpy(szCfgFileName, szCurrentDirectory);
    strcat(szCfgFileName, "/");
    strcat(szCfgFileName, __configuration_file__);

    /*
    * lettura del file di configurazione
    */
    strcpy(szParagraph,"General Settings");
    GetFileString(szParagraph,"PathTrace",   "../trace", Cfg.szPathTrace,         80,szCfgFileName,NULL);

    /*
    * Estraggo i parametri relativi ai processi 
    */
    ReadProcInfo(szCfgFileName);
    

    /*
    * parametri specifici dell'applicazione
    */
    sprintf(szParagraph,"%s %d",ProcessGetName(nPID),nPID);

    /*
    * Codice PID main process di riferimento (a cui mandare i barcode letti)
    */
    Cfg.nMainID = GetFileInt(szParagraph,"MainID",   0, szCfgFileName,NULL);

    /*
    * Modalita' di comunicazione con dispositivo di lettura peso bilancia
    */
    GetFileString(szParagraph,"TransmitMode","CONTINUE", Cfg.szTransmitMode, 80,szCfgFileName,NULL);
    if(!strcmp(Cfg.szTransmitMode,"CONTINUE")){
        Cfg.nTransmitMode=TX_CONTINUE;
    } else if(!strcmp(Cfg.szTransmitMode,"REQUEST")){
        Cfg.nTransmitMode=TX_REQUEST;
    } else {
#ifdef TRACE
        trace_debug(FALSE, TRUE, "Modo di trasmissione non settato [%s], valori : [CONTINUE] / REQUEST",Cfg.szTransmitMode);
#endif
        Cfg.nTransmitMode=TX_CONTINUE;
    }

    /*
    * nome della porta seriale utilizzata per il colloquio con la Bilancia
    */
    GetFileString(szParagraph,"CommDevice","/dev/com2", Cfg.szCommDevice, 80,szCfgFileName,NULL);

    /*
    * Parametri di comunicazione utilizzati per il colloquio con la Bilancia
    */
    GetFileString(szParagraph,"CommParams","9600,n,8,1", szBuffer, 80,szCfgFileName,NULL);

    /* 
    * Baudrate 
    */
    if((pPtr=strtok(szBuffer,szSeparator))!=NULL){
        Cfg.nBaudrate=atoi(pPtr);
    }
    /* Parity */
    if((pPtr=strtok(NULL,szSeparator))!=NULL){
        switch(*pPtr){
            default:
            case 'N': 
            case 'n':
                Cfg.nParity=0; /* NONE */
            break;
            case 'O': 
            case 'o':
                Cfg.nParity=1; /* ODD */
            break;
            case 'E': 
            case 'e':
                Cfg.nParity=2; /* EVEN */
            break;
        }
    };
    /* Data Bits */
    if((pPtr=strtok(NULL,szSeparator))!=NULL){
        Cfg.nDataBits=atoi(pPtr);
    };
    /* Stop Bits */
    if((pPtr=strtok(NULL,szSeparator))!=NULL){
        Cfg.nStopBits=atoi(pPtr);
    };
}



/* 
* main() 
*/
int main(int argc, char **argv)
{
    int nOpt;
    int nNumCar;

    while ((nOpt = getopt(argc, argv, "p:")) != -1) {
        switch(nOpt){
            case 'p':    /* Process ID */
                nPID=atoi(optarg);
            break;
        }
    }
    if(nPID==0){
        nPID=PROC_BILANCIA;
    }

    pszNomeModulo = argv[0];

    /*
    * Lettura della configurazione corrente
    */
    ReadConfiguration();

#ifdef TRACE
    /* Apro il file trace */
    sprintf(szBufTrace, "%s/%s_%d%s", Cfg.szPathTrace, g_path_get_basename (pszNomeModulo), nPID,TRACE_FILE_SUFFIX);
    open_trace(pszNomeModulo, szBufTrace, TRACE_FILE_MAXSIZE);

    trace_debug(FALSE, TRUE, "Module Name     : %s",pszNomeModulo);
    trace_debug(FALSE, TRUE, "Version         : %s",__version__);
    trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
    trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
    trace_debug(FALSE, TRUE, "Process ID      : %d",nPID);

    trace_debug(FALSE, TRUE, "Started         : %s %s", GetDate(szDateBuffer), GetTime(szTimeBuffer));
#endif

    /* Apro la coda messaggi principale */
    if(OpenProcessMsgQ(Cfg.nMainID)<0){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "%s : Apertura coda messaggi principale fallita", pszNomeModulo);
#endif
    }
    /* Apro la coda messaggi locale */
    if(CreateProcessMsgQ( nPID, 1)<0){
#ifdef TRACE
        trace_debug(FALSE, TRUE, "%s : Apertura coda messaggi locale fallita", pszNomeModulo);
#endif
    }


    /* Predispongo l'utilizzo dell'uscita di sicurezza */
    signal(SIGTERM, SafeIntFunc);


#ifdef TRACE
    trace_debug(FALSE, TRUE, "%s : Parametri di comunicazione : %s : %d,%d,%d,%d",  pszNomeModulo, Cfg.szCommDevice,Cfg.nBaudrate,Cfg.nParity,Cfg.nDataBits,Cfg.nStopBits);
#endif
    /* Apertura porta seriale */
    if (InitSer(Cfg.szCommDevice,&Cfg.nPortFD,Cfg.nBaudrate,Cfg.nDataBits,Cfg.nParity,Cfg.nStopBits,O_RDWR)) {
#ifdef TRACE
        trace_debug(FALSE, TRUE, "%s : Apertura porta '%s' fallita", pszNomeModulo,Cfg.szCommDevice);
#endif
        exit(0);
    }
    /*
    * avverto MAIN dello start
    */
    SendMessage(Cfg.nMainID, nPID,  PROGRAM_STARTED, NULL);

    /* Lancio il processo RX */
#ifdef __linux__
    nRxPId = fork();
    if (nRxPId < 0) {
#ifdef TRACE
        trace_debug(FALSE, TRUE, "%s : Lancio processo RX fallito", pszNomeModulo);
#endif
        CloseSer();
        exit(FALSE);
    }
#endif
    if (nRxPId == 0) {
        RxLoop();
    }

    /*
    * loop di ricezione messaggi
    */
    for(;;){
        nAllMsg = 0;
        if((nCarIn = ProcessReceiveMsg(nPID, &nAllMsg, szInMsg))>=0){
            UnpackMessage(szInMsg, &InMsgStruct, szText, nCarIn);

        	switch(InMsgStruct.code){
				case MAIN_REQ_WEIGHT:
				{
#ifdef TRACE
					trace_debug(FALSE, TRUE, "%s : arrivato MAIN_REQ_WEIGHT",pszNomeModulo);
#endif
					if(SendSerMsg(Cfg.nPortFD,NULL,msgSnd+ALFA_SND_REQ,&nNumCar)){
						/* Trasmissione fallita */
						SendMessage(InMsgStruct.srce, nPID, BILANCIA_TX_ERROR, szRcvBuf);
					}
				}
				break;
			}
		}
		usleep(Cfg.nDelay);
	}	 /* fine while */

	return 0;
}


/*
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc()
{
#ifdef TRACE
	trace_debug(FALSE, TRUE, "%s %s : %s Stopped", pszNomeModulo, GetDate(szDateBuffer), GetTime(szTimeBuffer));
#endif

	if(nRxPId){
#ifdef __linux__
		kill((pid_t)nRxPId,SIGTERM);
#endif
	}
	CloseSer();
	/*
	* avverto MAIN dello stop
	*/
	SendMessage(Cfg.nMainID, nPID,  PROGRAM_STOPPED, NULL);

	exit(1);
} /* Fine SafeIntFunc */


/*
* funzione di ricezione
* del barcode e spedizione messaggio di barcode ricevuto al main process
*/
void RxLoop(void)
{
	int	nMsgLung;
	int	nRitorno;
	int	nMsgList[MAX_NUM_MESSAGES];
	int nNumMsg;
	int nIndex;

	for (;;) {
		nMsgList[0] = ALFA_RCV_DATA;
		nNumMsg = 1;
		nRitorno = RecSerMsg(Cfg.nPortFD, szRcvBuf, msgRcv, nMsgList, &nNumMsg, &nMsgLung, 0, TIMEOUT_WITHIN);
	
		/* Controllo il messaggio ricevuto */
		switch(nRitorno){
			case ALFA_RCV_DATA:
				nIndex=0;
				while(szRcvBuf[nIndex]){
					szRcvBuf[nIndex] &= 0x7f;
					nIndex++;
				}
#ifdef NO_TRACE
				trace_debug(FALSE, TRUE, "%s : Arrivati dati bilancia: '%s'", pszNomeModulo,szRcvBuf);
#endif
				SendMessage(Cfg.nMainID, PROC_BILANCIA,  BILANCIA_RX_DATA, szRcvBuf);
			break;
			default:
#ifdef TRACE
				trace_debug(FALSE, TRUE, "%s : Errore numero %d", pszNomeModulo,nRitorno);
#endif
				SendMessage(Cfg.nMainID, PROC_BILANCIA,  BILANCIA_RX_ERROR, szRcvBuf);
			break;

		}
	}
} /* Fine RxLoop*/

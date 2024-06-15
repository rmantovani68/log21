/*
* ev2002.c : gestione Bilancia modello Bilanciai EV2002
* 
* Progetto Mondadori Picking - Gestione pesatura bancali
*
* Autore : Roberto Mantovani - Stefano Tarroni
*
* Copyright A&L srl 2003-2021
*/
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <glib.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#ifdef TRACE
	#include <trace.h>
#endif

#include <proc_list.h>
#include <ep-common.h>

#include "ev2002.h"

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


int	nRxPId;			
CFGSTRUCT	Cfg;                /* struttura della configurazione */

/*
* ReadConfiguration()
* lettura della configurazione dal file cni.cfg
*/
void ReadConfiguration(void)
{
	char szParagraph[128];
	char szCurrentDirectory[128];
	char szCfgFileName[128];
	char szProcIndex[128];
	char szProcBuffer[128];
	char szBuffer[128];
	char  *pPtr;
	char  *szSeparator=",\t\n";
	int nIndex;

	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));

    extern char __configuration_file__[];
	
	strcpy(szCfgFileName, szCurrentDirectory);
	strcat(szCfgFileName, "/");
	strcat(szCfgFileName, __configuration_file__);
	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");
	GetFileString(szParagraph,"PathTrace",   "../trace", Cfg.szPathTrace,         80,szCfgFileName,NULL);

    ReadProcInfo(szCfgFileName);

	/*
	* parametri specifici dell'applicazione
	*/
	strcpy(szParagraph,ProcessGetName(PROC_EV2002));
	/*
	* nome della porta seriale utilizzata per il colloquio con la Bilancia
	*/
	GetFileString(szParagraph,"CommDevice","/dev/ttyS0", Cfg.szCommDevice, 80,szCfgFileName,NULL);

	/*
	* Parametri di comunicazione utilizzati per il colloquio con la Bilancia
	*/
	GetFileString(szParagraph,"CommParams","9600,n,8,1", szBuffer, 80,szCfgFileName,NULL);

	/*
	* Delay in millisecondi (lo trasformo in microsecondi)
	*/
	Cfg.nDelay=1000*GetFileInt(ProcessGetName(PROC_EV2002),"Delay",100,szCfgFileName,NULL);

	/* Baudrate */
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
	/*
	* simulazione
	*/
	Cfg.nSimulazione=GetFileInt(ProcessGetName(PROC_EV2002),"simulazione",0,szCfgFileName,NULL);
}



/* 
* main() 
*/
int main(int argc, char **argv)
{

	pszNomeModulo = argv[0];

	/*
	* Lettura della configurazione corrente
	*/
	ReadConfiguration();

#ifdef TRACE
	/* Apro il file trace */
	sprintf(szBufTrace, "%s/%s%s", Cfg.szPathTrace, g_path_get_basename (pszNomeModulo), TRACE_FILE_SUFFIX);
	open_trace(pszNomeModulo, szBufTrace, TRACE_FILE_MAXSIZE);

	trace_debug(FALSE, TRUE, "Module Name     : %s",pszNomeModulo);
	trace_debug(FALSE, TRUE, "Started         : %s %s", GetDate(time((long *)0), szDateBuffer), GetTime(time((long *)0), szTimeBuffer));
#endif

	/* Apro la coda messaggi principale */
	if(OpenProcessMsgQ(PROC_BANCALI)<0){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "%s : Apertura coda messaggi principale fallita", pszNomeModulo);
#endif
	}


	/* Predispongo l'utilizzo dell'uscita di sicurezza */
	signal(SIGTERM, SafeIntFunc);


#ifdef TRACE
    trace_debug(FALSE, TRUE, "%s : Parametri di comunicazione : %s : %d,%d,%d,%d", pszNomeModulo, Cfg.szCommDevice,Cfg.nBaudrate,Cfg.nParity,Cfg.nDataBits,Cfg.nStopBits);
#endif
	/* Apertura porta seriale */
	if (InitSer(Cfg.szCommDevice,&Cfg.nPortFD,Cfg.nBaudrate,Cfg.nDataBits,Cfg.nParity,Cfg.nStopBits,O_RDWR)) {
#ifdef TRACE
		trace_debug(FALSE, TRUE, "%s : Apertura porta '%s' fallita", pszNomeModulo,Cfg.szCommDevice);
#endif
		SafeIntFunc();
	}
	/*
	* avverto BANCALI dello start
	*/
	SendMessage(PROC_BANCALI, PROC_EV2002, PROGRAM_STARTED, NULL);

    RxLoop();


	return 0;
}


/*
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc()
{
#ifdef TRACE
	trace_debug(FALSE, TRUE, "%s %s : %s Stopped", pszNomeModulo, GetDate(time((long *)0), szDateBuffer), GetTime(time((long *)0), szTimeBuffer));
#endif
	if(nRxPId){
		kill((pid_t)nRxPId,SIGTERM);
	}

	CloseSer();
	/*
	* avverto MAIN dello stop
	*/
	SendMessage(PROC_BANCALI, PROC_EV2002, PROGRAM_STOPPED, NULL);

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

	for (;;) {
        if(Cfg.nSimulazione){
            nRitorno = EV2002_RCV_DATA;
            sprintf(szRcvBuf,"%07d %07d AAA\r\n",12345,67890);
        } else {
            nMsgList[0] = EV2002_RCV_DATA;
            nNumMsg = 1;
            nRitorno = RecSerMsg(Cfg.nPortFD, szRcvBuf, msgRcv, nMsgList, &nNumMsg, &nMsgLung, 0, TIMEOUT_WITHIN);
        }

		/* Controllo il messaggio ricevuto */
		switch(nRitorno){
			case EV2002_RCV_DATA:
#ifdef TRACE
				trace_debug(FALSE, TRUE, "%s : Arrivati dati bilancia: '%s'", pszNomeModulo,szRcvBuf);
#endif
				SendMessage(PROC_BANCALI, PROC_EV2002, EV2002_RX_DATA, szRcvBuf);
			break;
			default:
#ifdef TRACE
				trace_debug(FALSE, TRUE, "%s : Errore numero %d", pszNomeModulo,nRitorno);
#endif
				SendMessage(PROC_BANCALI, PROC_EV2002, EV2002_RX_ERROR, szRcvBuf);
			break;

		}
	}
} /* Fine RxLoop*/

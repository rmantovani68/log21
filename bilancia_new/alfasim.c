/*
* alfasim.c : simulatore Bilancia modello Alfa Bilici SE357
* 
* Progetto Mondadori Picking
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
#include <sys/mman.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#ifdef TRACE
	#include <trace.h>
#endif

#include <proc_list.h>

#include "bilancia.h"

int nAllMsg;
int nCarIn;
MSGHDRINT InMsgStruct;
char szInMsg[INFO_LEN];
char szText[INFO_LEN];
char szBufTrace[80];
char *pszNomeModulo;
char szRcvBuffer[256];

char szTimeBuffer[80];
char szDateBuffer[80];


int	nRxPId;			
CFGSTRUCT	Cfg;                /* struttura della configurazione */
PROCESSO ProcList[NUM_PROC];	/* processi componenti il progetto */

/*
* ReadConfiguration()
* lettura della configurazione dal file cni.cfg
*/
void ReadConfiguration(void)
{
	char szParagraph[128];
	char szCurrentDirectory[128];
	char szCniCfg[128];
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
	
	strcpy(szCniCfg,szCurrentDirectory);
	strcat(szCniCfg,"/cni.cfg");
	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");

	/*
	* Estraggo i parametri relativi ai processi 
	*/
	for (nIndex=0; nIndex<NUM_PROC; nIndex++) {
		sprintf(szProcIndex,"Proc_%02d",nIndex);
		GetFileString("procinfo",szProcIndex, "", szProcBuffer, 80,szCniCfg,NULL);
		sscanf(szProcBuffer, "%[^,],%d,%d",
				ProcList[nIndex].szProcName,
				&ProcList[nIndex].nQKey,
				&ProcList[nIndex].bExecute);

		ProcList[nIndex].nGId=nIndex+1;

		/*
		* resetto il numero coda messaggi del processo
		*/
		ProcList[nIndex].nQNumber=0;
	}
	/*
	* parametri specifici dell'applicazione
	*/
	strcpy(szParagraph,ProcList[PROC_BILSIM].szProcName);
	/*
	* nome della porta seriale utilizzata per il colloquio con la Bilancia
	*/
	GetFileString(szParagraph,"CommDevice","/dev/com2", Cfg.szCommDevice, 80,szCniCfg,NULL);

	/*
	* Parametri di comunicazione utilizzati per il colloquio con la Bilancia
	*/
	GetFileString(szParagraph,"CommParams","9600,n,8,1", szBuffer, 80,szCniCfg,NULL);

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
	sprintf(szBufTrace, "%s%s", pszNomeModulo, TRACE_FILE_SUFFIX);
	open_trace(pszNomeModulo, szBufTrace, TRACE_FILE_MAXSIZE);

	trace_out_vstr(1,"Module Name     : %s",pszNomeModulo);
	trace_out_vstr(1,"Started         : %s %s",
			GetDate(time((long *)0), szDateBuffer),
			GetTime(time((long *)0), szTimeBuffer));
#endif

	/* Apro la coda messaggi principale */
	if((ProcList[PROC_MAIN].nQNumber = OpenMsgQ(ProcList[PROC_MAIN].nQKey))<0){
#ifdef TRACE
		trace_out_vstr(1, "%s : Apertura coda messaggi principale fallita", pszNomeModulo);
#endif
	}


	/* Predispongo l'utilizzo dell'uscita di sicurezza */
	signal(SIGTERM, SafeIntFunc);


#ifdef TRACE
		trace_out_vstr(1, "%s : Parametri di comunicazione : %s : %d,%d,%d,%d", pszNomeModulo,
			Cfg.szCommDevice,Cfg.nBaudrate,Cfg.nParity,Cfg.nDataBits,Cfg.nStopBits);
#endif
	/* Apertura porta seriale */
	if (InitSer(Cfg.szCommDevice,&Cfg.nPortFD,Cfg.nBaudrate,Cfg.nDataBits,Cfg.nParity,Cfg.nStopBits,O_RDWR)) {
#ifdef TRACE
		trace_out_vstr(1, "%s : Apertura porta '%s' fallita", pszNomeModulo,Cfg.szCommDevice);
#endif
	}
	/*
	* avverto MAIN dello start
	*/
	SendSpecificMsg(ProcList[PROC_MAIN].nQNumber, ProcList[PROC_MAIN].nGId, ProcList[PROC_BILSIM].nGId, PROGRAM_STARTED, NULL);

	/*
	* sempre in ricezione sulla seriale
	*/
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
	trace_out_vstr(1,"%s %s : %s Stopped",
		pszNomeModulo,
		GetDate(time((long *)0), szDateBuffer),
		GetTime(time((long *)0), szTimeBuffer));
#endif

	CloseSer();
	/*
	* avverto MAIN dello stop
	*/
	SendSpecificMsg(ProcList[PROC_MAIN].nQNumber, ProcList[PROC_MAIN].nGId, ProcList[PROC_BILSIM].nGId, PROGRAM_STOPPED, NULL);

	exit(1);
} /* Fine SafeIntFunc */

/*
* GetTime()
* get time of day as a string
*/
char *GetTime(long time,char *szTimeString)
{
	struct tm *ltime;
	long curr_time;

	curr_time = time;
	ltime = localtime(&curr_time);
	sprintf(szTimeString, "%.2d:%.2d:%.2d", 
		ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

	return szTimeString;
}

/*
* GetDate()
* get date a string
*/
char *GetDate(long time,char *szDateString)
{
	struct tm *ltime;
	long curr_time;

	curr_time = time;
	ltime = localtime(&curr_time);
	sprintf(szDateString, "%.2d-%.2d-%.2d", 
		ltime->tm_mday, ltime->tm_mon+1, ltime->tm_year%100);

	return szDateString;
}


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
	char szSndBuffer[80];
	int nNumCar;

	nMsgList[0] = ALFA_SIM_RCV_TX_REQ;
	nNumMsg = 1;
	for (;;) {
		nRitorno = RecSerMsg(Cfg.nPortFD, szRcvBuffer, msgRcv, nMsgList, &nNumMsg, &nMsgLung, 0, TIMEOUT_WITHIN);
	
		/* Controllo il messaggio ricevuto */
		switch(nRitorno){
			case ALFA_SIM_RCV_TX_REQ:
				/* spedisco il peso */
				sprintf(szSndBuffer,"%07d %07d AAA\r\n",0,0);
				/*
				* (msgSnd+ALFA_SIM_SND_DATA)->end_len=strlen(szSndBuffer);
				*/
				if (SendSerMsg(Cfg.nPortFD,szSndBuffer,msgSnd+ALFA_SIM_SND_DATA,&nNumCar)){
					/* Trasmissione fallita */
#ifdef TRACE
					trace_out_vstr(1,"%s : Errore in trasmissione",pszNomeModulo);
#endif
				}
			break;
			default:
#ifdef TRACE
				trace_out_vstr(1,"%s : Errore numero %d",pszNomeModulo,nRitorno);
#endif
			break;

		}
	}
} /* Fine RxLoop*/

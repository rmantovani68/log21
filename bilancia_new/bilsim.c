/*
* bilsim.c : simulatore Bilancia 
* 
* Progetto Easy Picking 3.0
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
void main(int argc, char **argv)
{
	int nNumCar;

	pszNomeModulo = argv[0];

	/*
	* Lettura della configurazione corrente
	*/
	ReadConfiguration();

#ifdef TRACE
	/* Apro il file trace */
	sprintf(szBufTrace, "%s%s", pszNomeModulo, TRACE_FILE_SUFFIX);
	open_trace(pszNomeModulo, szBufTrace, TRACE_FILE_MAXSIZE);

	trace_out_vstr_date(1,"Started");
	trace_out_vstr(1,"Module Name     : %s",pszNomeModulo);
#endif

	/* Apro la coda messaggi principale */
	if((ProcList[PROC_PESO].nQNumber = OpenMsgQ(ProcList[PROC_PESO].nQKey))<0){
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
	SendSpecificMsg(ProcList[PROC_PESO].nQNumber, ProcList[PROC_PESO].nGId, ProcList[PROC_BILSIM].nGId, PROGRAM_STARTED, NULL);

	/*
	* sempre in ricezione sulla seriale
	*/
	RxLoop();
}


/*
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc()
{
#ifdef TRACE
	trace_out_vstr_date(1,"Stopped");
#endif

	CloseSer();
	/*
	* avverto MAIN dello stop
	*/
	SendSpecificMsg(ProcList[PROC_PESO].nQNumber, ProcList[PROC_PESO].nGId, ProcList[PROC_BILSIM].nGId, PROGRAM_STOPPED, NULL);

	exit(1);
} /* Fine SafeIntFunc */

int WriteString(int nFD, char *szBuffer)
{
	int nRC;
	char c;
	int nIndex=0;

	while(szBuffer[nIndex]){
		c=szBuffer[nIndex];
		nRC=WriteChar(nFD,c);
		nIndex++;
	}
	return nIndex;
}
int ReadString(int nFD, char *szBuffer,char cEndChar)
{
	BOOL bFineLettura=FALSE;
	int nRC;
	BOOL bStart=TRUE;
	char c;
	int nIndex=0;

	while(!bFineLettura){
		nRC=ReadChar(nFD,&c,bStart);
		bStart=FALSE;
		szBuffer[nIndex++]=c;
		if(c==cEndChar){
			bFineLettura=TRUE;
		}
	}
	szBuffer[nIndex++]='\0';
	return nIndex;
}


/*
* funzione di ricezione
* del barcode e spedizione messaggio di barcode ricevuto al main process
*/
void RxLoop(void)
{
	int	nRitorno;
	char szSndBuffer[80];

	for (;;) {
		nRitorno = ReadString(Cfg.nPortFD,szRcvBuffer,CH_ACK);
	
		/* spedisco il peso */
		sprintf(szSndBuffer,"\002%07d%07d1XX0\003",0,0);
		WriteString(Cfg.nPortFD,szSndBuffer);
	}
} /* Fine RxLoop*/


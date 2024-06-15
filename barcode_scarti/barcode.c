/*
* barcode.c : gestione Barcode Rader
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
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#ifdef TRACE
	#include <trace.h>
#endif

#include <proc_list.h>

#include "barcode.h"

CFGSTRUCT	Cfg;                /* struttura della configurazione */
PROCESSO ProcList[NUM_PROC];	/* processi componenti il progetto */
int nPID=0;                   /* Process ID */


char szBufTrace[80];
char szBarcode[80];
char szParams[80];
char szRcvBuffer[256];
char *pszNomeModulo;
char szTimeBuffer[80];
char szDateBuffer[80];

extern char *optarg;
extern int optind;
extern char rcsid[];
extern char __version__[];
extern char __customer__[];


/*
* LeftStr()
* ritorna i primi nLen caratteri della stringa
*/
char *LeftStr(char *szString,int nLen)
{
	static char szBuffer[1024];

	strncpy(szBuffer,szString,nLen);
	szBuffer[nLen]='\0';

	return(szBuffer);
}

/*
* RightStr()
* ritorna gli ultimi nLen caratteri della stringa
*/
char *RightStr(char *szString,int nLen)
{
	static char szBuffer[1024];

	strcpy(szBuffer,szString+((strlen(szString)-nLen)));

	return(szBuffer);
}

/*
* ritorna una porzione della stringa passata in ingresso
* a partire da nPos per nLen caratteri
*/
char *SubStr(char *szString,int nPos,int nLen)
{
	static char szBuffer[1024];

	/*
	* normalizzo nLen per evitare crash di applicazione
	*/
	nLen=min(1023,nLen);
	nPos=min(nPos,strlen(szString)-nLen);

	memset(szBuffer,(int)' ',nLen);
	strncpy(szBuffer,szString+nPos,nLen);
	szBuffer[nLen]='\0';

	return(szBuffer);
}


/*
* StrTrimLeft()
*/
char *StrTrimLeft(char *str)
{
	char *dst,*src;

	dst=src=str;
	while(*src==' ') src++;
	while((*dst++=*src++));
	return(str);
}


/*
* StrTrimRight()
*/
char *StrTrimRight(char *str)
{
	char *dst,*src;

	src=str+strlen(str)-1;
	while(*src==' ') src--;
	*(src+1)='\0';
	dst=src=str;
	while((*dst++=*src++));
	return(str);
}


/*
* StrTrimAll()
*/
char *StrTrimAll(char *str)
{
	StrTrimLeft(str);
	StrTrimRight(str);
	return(str);
} 


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

		/* 
		* rm 16-01-2002 : Elimino gli spazi in testa e in coda per 
		*                 problemi con la RunSimpleProcess()  e la lettura della condifgurazione
		*/
		StrTrimAll(ProcList[nIndex].szProcName);

		ProcList[nIndex].nGId=nIndex+1;

		/*
		* resetto il numero coda messaggi del processo
		*/
		ProcList[nIndex].nQNumber=0;
	}
	/*
	* parametri specifici dell'applicazione
	*/
	sprintf(szParagraph,"%s %d",ProcList[nPID].szProcName,nPID);

	/*
	* Codice PID main process di riferimento (a cui mandare i barcode letti)
	*/
	Cfg.nMainID = GetFileInt(szParagraph,"MainID",   0, szCniCfg,NULL);

	/*
	* nome della porta seriale utilizzata per il colloquio con lo scanner
	*/
	GetFileString(szParagraph,"CommDevice","/dev/com2", Cfg.szCommDevice, 80,szCniCfg,NULL);

	/*
	* Parametri di comunicazione utilizzati per il colloquio con lo scanner
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
	}
	/* Data Bits */
	if((pPtr=strtok(NULL,szSeparator))!=NULL){
		Cfg.nDataBits=atoi(pPtr);
	}
	/* Stop Bits */
	if((pPtr=strtok(NULL,szSeparator))!=NULL){
		Cfg.nStopBits=atoi(pPtr);
	}
}


/* main() */
int main(int argc, char **argv)
{
	int nOpt;

	while ((nOpt = getopt(argc, argv, "p:")) != -1) {
		switch(nOpt){
			case 'p':	/* Process ID */
				nPID=atoi(optarg);
			break;
		}
	}
	if(nPID==0){
		fprintf(stderr,"PID non definito\n");
		exit(0);
	}

	pszNomeModulo = argv[0];

#ifdef TRACE
	/* Apro il file trace */
	sprintf(szBufTrace, "%s_%d%s", pszNomeModulo, nPID,TRACE_FILE_SUFFIX);

	open_trace(pszNomeModulo, szBufTrace, TRACE_FILE_MAXSIZE);

	trace_debug(FALSE, TRUE, "Module Name     : %s",pszNomeModulo);
	trace_debug(FALSE, TRUE, "Version         : %s",__version__);
	trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
	trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
	trace_debug(FALSE, TRUE, "Process ID      : %d",nPID);

	trace_debug(TRUE, TRUE, "Started");
#endif


	ReadConfiguration();

	/* Apro la coda messaggi principale */
	if((ProcList[Cfg.nMainID].nQNumber = OpenMsgQ(ProcList[Cfg.nMainID].nQKey))<0){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "%s : Apertura coda messaggi principale fallita", pszNomeModulo);
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

	RxLoop();
	return 0;
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
	int nIndex;

	for (;;) {
		nIndex=0;
		nMsgList[nIndex] = BCRD_RCV_BARCODE; nIndex++;

		nNumMsg = nIndex;

		nRitorno = RecSerMsg(Cfg.nPortFD, szRcvBuffer, msgRcv, nMsgList, &nNumMsg, &nMsgLung, 0, TIMEOUT_WITHIN);
	
		/* Controllo il messaggio ricevuto */
		switch(nRitorno){
			case BCRD_RCV_BARCODE:
				/*
				* barcode : dato + <CR> + <LF>
				* dato : barcode letto o <CAN>
				*/
				strncpy(szBarcode,szRcvBuffer,nMsgLung-2);
				szBarcode[nMsgLung-2]='\0';
				if(szBarcode[0]==CAN_CHAR){
#ifdef TRACE
					trace_debug(TRUE, TRUE, "BCRD : Ricevuto NOREAD");
#endif
					SendMessage(Cfg.nMainID, nPID,  BCRD_RX_NOREAD, NULL);
				} else {
#ifdef TRACE
					trace_debug(TRUE, TRUE, "BCRD : Ricevuto Barcode :[%s]", szBarcode);
#endif
					SendMessage(Cfg.nMainID, nPID,  BCRD_RX_BARCODE, szBarcode);
				}
			break;

			default:
#ifdef TRACE
				trace_debug(FALSE, TRUE, "BCRD: Errore numero %d", nRitorno);
#endif
			break;

		}
	}
} /* Fine RxLoop*/



/*
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc()
{
	CloseSer();
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stopped");
#endif

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
* SendMessage()
* Spedizione messaggio con timeout e tentativi (BLOCCANTE)
*/
ep_bool_t SendMessage(int nDest,int nSrce,int nMsgCode,char *szText)
{
	int nRC;

	if((nRC=SendSpecificMsg(ProcList[nDest].nQNumber, ProcList[nDest].nGId, ProcList[nSrce].nGId, nMsgCode, szText))!=0){
		trace_debug(TRUE, TRUE, "Errore [%d] in spedizione Messaggio a [%s]", nRC,ProcList[nDest].szProcName);
		return FALSE;
	}
	return TRUE;
}

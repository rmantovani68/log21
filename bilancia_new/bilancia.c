/*
* bilancia.c : gestione Bilancia modello Alfa Bilici
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


int nPID=0;                   /* Process ID */
int	nRxPId=0;			
CFGSTRUCT	Cfg;                /* struttura della configurazione */
PROCESSO ProcList[NUM_PROC];	/* processi componenti il progetto */

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
	* Modalita' di comunicazione con dispositivo di lettura peso bilancia
	*/
	GetFileString(szParagraph,"TransmitMode","CONTINUE", Cfg.szTransmitMode, 80,szCniCfg,NULL);
	if(!strcmp(Cfg.szTransmitMode,"CONTINUE")){
		Cfg.nTransmitMode=TX_CONTINUE;
	} else if(!strcmp(Cfg.szTransmitMode,"REQUEST")){
		Cfg.nTransmitMode=TX_REQUEST;
	} else {
#ifdef TRACE
		trace_out_vstr(1, "Modo di trasmissione non settato [%s], valori : [CONTINUE] / REQUEST",Cfg.szTransmitMode);
#endif
		Cfg.nTransmitMode=TX_CONTINUE;
	}

	/*
	* nome della porta seriale utilizzata per il colloquio con la Bilancia
	*/
	GetFileString(szParagraph,"CommDevice","/dev/com2", Cfg.szCommDevice, 80,szCniCfg,NULL);

	/*
	* Parametri di comunicazione utilizzati per il colloquio con la Bilancia
	*/
	GetFileString(szParagraph,"CommParams","9600,n,8,1", szBuffer, 80,szCniCfg,NULL);

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

	while ((nOpt = getopt(argc, argv, "p:")) != -1) {
		switch(nOpt){
			case 'p':	/* Process ID */
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
	sprintf(szBufTrace, "%s%s", pszNomeModulo, TRACE_FILE_SUFFIX);
	open_trace(pszNomeModulo, szBufTrace, TRACE_FILE_MAXSIZE);

	trace_out_vstr(1,"Module Name     : %s",pszNomeModulo);
	trace_out_vstr(1,"Version         : %s",__version__);
	trace_out_vstr(1,"Customer Name   : %s",__customer__);
	trace_out_vstr(1,"RCSID String    : %s",rcsid);
	trace_out_vstr(1,"Process ID      : %d",nPID);

	trace_out_vstr(1,"Started         : %s %s", GetDate(time((long *)0), szDateBuffer), GetTime(time((long *)0), szTimeBuffer));
#endif

	/* Apro la coda messaggi principale */
	if((ProcList[Cfg.nMainID].nQNumber = OpenMsgQ(ProcList[Cfg.nMainID].nQKey))<0){
#ifdef TRACE
		trace_out_vstr(1, "%s : Apertura coda messaggi principale fallita", pszNomeModulo);
#endif
	}
	/* Apro la coda messaggi locale */
	if((ProcList[PROC_BILANCIA].nQNumber = OpenMsgQ(ProcList[PROC_BILANCIA].nQKey))<0){
#ifdef TRACE
		trace_out_vstr(1, "%s : Apertura coda messaggi locale fallita", pszNomeModulo);
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
		exit(0);
	}
	/*
	* avverto MAIN dello start
	*/
	SendMessage(Cfg.nMainID, PROC_BILANCIA,  PROGRAM_STARTED, NULL);

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

	if(nRxPId){
		kill((pid_t)nRxPId,SIGTERM);
	}
	CloseSer();
	/*
	* avverto MAIN dello stop
	*/
	SendMessage(Cfg.nMainID, PROC_BILANCIA,  PROGRAM_STOPPED, NULL);

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
BOOL SendMessage(int nDest,int nSrce,int nMsgCode,char *szText)
{
	int nRC;

	if((nRC=SendSpecificMsg(ProcList[nDest].nQNumber, ProcList[nDest].nGId, ProcList[nSrce].nGId, nMsgCode, szText))!=0){
		trace_out_vstr_date(1,"Errore [%d] in spedizione Messaggio a [%s]", nRC,ProcList[nDest].szProcName);
		return FALSE;
	}
	return TRUE;
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
		nMsgList[0] = ALFA_RCV_DATA;
		nNumMsg = 1;
		nRitorno = RecSerMsg(Cfg.nPortFD, szRcvBuffer, msgRcv, nMsgList, &nNumMsg, &nMsgLung, 0, TIMEOUT_WITHIN);
	
		/* Controllo il messaggio ricevuto */
		switch(nRitorno){
			case ALFA_RCV_DATA:
				nIndex=0;
				while(szRcvBuffer[nIndex]){
					szRcvBuffer[nIndex] &= 0x7f;
					nIndex++;
				}
#ifdef NO_TRACE
				trace_out_vstr(1,"%s : Arrivati dati bilancia: '%s'", pszNomeModulo,szRcvBuffer);
#endif
				SendMessage(Cfg.nMainID, PROC_BILANCIA,  BILANCIA_RX_DATA, szRcvBuffer);
			break;
			default:
#ifdef TRACE
				trace_out_vstr(1,"%s : Errore numero %d", pszNomeModulo,nRitorno);
#endif
				SendMessage(Cfg.nMainID, PROC_BILANCIA,  BILANCIA_RX_ERROR, szRcvBuffer);
			break;

		}
	}
} /* Fine RxLoop*/

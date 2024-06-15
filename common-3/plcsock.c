/*
* plcsock.c
* Funzioni di gestione della comunicazione con il PLC Server
*
* Data di Creazione : 05-12-1997
* Autore : Roberto Mantovani
* 
* Copyright CNI 1997
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>
#include <signal.h>

extern int errno;

#if defined(TRACE) || defined(TRACE_PLC)
	#include <trace.h>
#endif

#include "plcsock.h"

/*
* lista dei comandi utilizzati
*/
char *szCmds[]={
	"errs",     /* richiesta o sospensione trasmissione errori */
	"info",     /* informazioni sul funzionamento del PLC */
	"go",       /* GO PLC */
	"halt",     /* HALT PLC */
	"list",     /* lista dispositivi connessi */
	"memchk",   /* convalida e registra variabili di debug */
	"memcopy",  /* dati relativi alle variabili registrate con memchk */
	"set",      /* scrittura dei valori delle variabili */
	"stat",     /* gestione della statistica  */
	"status",   /* stato del PLC (HALT/GO) */
	"trace",    /* Gestione del trace A */
	"trace",    /* Gestione del trace C */
	"trace",    /* Gestione del trace D */
	"trace",    /* Gestione del trace E */
	"trace",    /* Gestione del trace M */
	"trace",    /* Gestione del trace T */
	"trace",    /* Gestione del trace V */
	"trigger",  /* Gestione del trigger A */
	"trigger",  /* Gestione del trigger C */
	"trigger",  /* Gestione del trace D */
	"var",      /* risoluzione di un simbolo */
	"ver",      /* versione di QPLC */
	"wadd",     /* Aggiunta di un breakpoint */
	"wcreate",  /* Creazione di una lista di breakpoint */
	"wdel",     /* Cancella una lista di watch */
	"wenable",  /* Abilita una lista di watch */
	"wreset",   /* Cancella tutte le liste di watch */
	"wshow",    /* Visualizza i dati di debug relativi all'ultimo ciclo */
	"Nothing",  /* niente ... */
	(char *)NULL
};

/*
* lista delle risposte ammesse
*/
char *szAnswrs[]={
	"OK",     /* Conferma ultimo comando eseguito      : OK */
	"A",      /* Messaggio asincrono                   : A <codice> [dati] */
	"E",      /* Errore                                : E <codice> */
	"D",      /* Messaggio sincrono                    : D <dati> */
	"L",      /* Lunghezza prossimo messaggio sincrono : L <lunghezza> */
	(char *)NULL
};

char *szErrs[]={
	NULL,
	"Command unknown",
	"Command too long",
	"Invalid arguments number",
	"invalid parameter",
	"PLC bad state",
};

PLC_TYPES PLCTypes[]={
	{"BOOL",1},
	{"BYTE",1},
	{"INT",2},
	{"SINT",1},
	{"DINT",4},
	{"USINT",1},
	{"UINT",2},
	{"UDINT",4},
	{"REAL",4},
	{"TIME",4},
	{"STRING",0},
	{"WORD",2},
	{"DWORD",4},
	{"UNKNOWN",0},
};

int nLastCommand;
char szRcvBuffer[MAX_RCV_SIZE];

char *GetPLCVersion(char *szData)
{
	int nIndex;
	static char szPLCVersion[128];

	/* 
	* mi aspetto la risposta nella forma
	* D <versione>
	*/

	strcpy(szPLCVersion,szData+2);

	/* sostituisco i tab con lo spazio */
	nIndex=0;
	while(szPLCVersion[nIndex]){
		if(szPLCVersion[nIndex]=='\t'){
			szPLCVersion[nIndex]=' ';
		}
		nIndex++;
	}
	return szPLCVersion;
}

BOOL GetPLCStatus(char *szData)
{
	/* 
	* mi aspetto la risposta nella forma
	* D <status>
	*/

	switch(*(szData+2)){
		case '0':	/* PLC in HALT */
			return FALSE;
		break;
		case '1':	/* PLC in GO */
			return TRUE;
		break;
	}
	return FALSE;
}



unsigned int CheckPLCResponse(int fd,unsigned int bReceive)
{
	unsigned int bExit=FALSE;

	while(!bExit){
		if(ReceiveData(fd,szRcvBuffer)){
			bExit=TRUE;
		}
	}
	return bExit;
}

		
int OpenSocket(int *fdPort,char *name,int nPortNumber)
{
	struct sockaddr_in sin;
	struct hostent *h;
	long lAddr;


	*fdPort = socket(AF_INET, SOCK_STREAM, 0);
	if (*fdPort < 0) {
		perror("socket");
		return 0;
	}
	/*
	* rendo non bloccante il file descriptor
	fcntl(*fdPort,F_SETFL,O_NONBLOCK);
	*/

	memset((char *)&sin,0,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(nPortNumber);
	lAddr = inet_addr(name);
	if (lAddr == -1) {
		h = gethostbyname(name);
		if (! h) {
			fprintf(stderr,"Unknown host %s\n",name);
			return(0);
		}
		sin.sin_addr.s_addr = *(long *)(h -> h_addr);
	}
	else {
		sin.sin_addr.s_addr = lAddr;
	}
	if (connect(*fdPort,(struct sockaddr *)&sin,sizeof(sin)) < 0) {
		perror("connect");
		return 0;
	}

	return *fdPort;
}

/*
* ReceiveData()
* ricezione dati in coda di attesa dal PLC Server
*/
int ReceiveData(int fd,char *szRcvBuf)
{
	int rv;

	rv = recv(fd,szRcvBuf,MAX_RCV_SIZE,0);
	if(rv<0){
#ifdef TRACE
		trace_out_vstr_date(1,"RECV ERROR : [%d] [%s]",rv, strerror(errno));
#endif
	} else if(rv==0){
		/*
		* connection lost (PLC IN CRASH ?!)
		*/
	} else {
		szRcvBuf[rv]='\0';
	}

#ifdef TRACE_PLC
	if(rv){
		trace_out_vstr_date(1,"PLC REC : [%s] [%d]",szRcvBuf, rv);
	}
#endif

	return(rv);
}

unsigned int SendCmd(int fd,int nCmd,char *szParams,unsigned int bRecData)
{
	char szSendData[1024];
	int nRC;
	char *pCmdPtr[256];
	char *pAnswer;
	char *pRcvPtr;
	char *szCmdSep="\n";
	int nCmdPtrIndex=0;
	unsigned int bExit=FALSE;
	unsigned int bError=FALSE;

	nLastCommand=nCmd;

	strcpy(szSendData,szCmds[nCmd]);
	
	if(szParams!=(char *)NULL){
		strcat(szSendData," ");
		strcat(szSendData,szParams);
	}
	
#ifdef TRACE_PLC
	trace_out_vstr_date(1,"PLC CMD: [%s]\n",szSendData);
#endif

	strcat(szSendData,"\n");

	/* spedizione messaggio al PLC */
	nRC=send(fd,szSendData,strlen(szSendData),0);

	/* arrivato qualcosa .. controllo  */
	while(!bError && !bExit){
		/* ricezione dati ... */
		if((nRC=ReceiveData(fd,szRcvBuffer))<=0){
			/*
			* Errore di connessione (PLC CRASH ?!)
			*/
			bExit=TRUE;
			continue;
		}
			
		/* costruisco un array di puntatory alle risposte arrivate */
		pRcvPtr=szRcvBuffer;
		while((pAnswer=strtok(pRcvPtr,szCmdSep))!=NULL){
			pCmdPtr[nCmdPtrIndex]=pAnswer;
			
			nCmdPtrIndex++;
			pRcvPtr=NULL;
		}
		pCmdPtr[nCmdPtrIndex]=NULL;

		nCmdPtrIndex=0;
		while(pCmdPtr[nCmdPtrIndex]){
			pRcvPtr=pCmdPtr[nCmdPtrIndex];

			if(pRcvPtr[0]=='E'){
				/* 
				* messaggio di errore  ...
				*/
#ifdef TRACE
				szSendData[strlen(szSendData)-1]='\0';
				trace_out_vstr_date(1,"PLC ERROR : CMD:[%s] [%s]",szSendData,pRcvPtr);
#endif
				/* esco dal ciclo while di attesa risposta ... */
				bError=TRUE;
			} else if(pRcvPtr[0]=='A'){
				/* 
				* messaggio asincrono ...
				* lo gestisco dopo ....
				*/
				
#ifdef TRACE
				trace_out_vstr_date(1,"PLC ASYNC : [%s]",pRcvPtr);
#endif
			} else {
				/* esco dal ciclo while di attesa risposta ... */
				bExit=TRUE;
			}
			nCmdPtrIndex++;
		}
	}
	if(nRC==0){
#ifdef TRACE
		trace_out_vstr_date(1,"PLC CONNECTION ERROR");
#endif
	}
	return(bError?0:nRC);
}

int GetVarType(unsigned long ulFlags)
{
	ulFlags&=SHV_MODE_TYPE;

	switch(ulFlags){
		case SHV_MODE_TYPE_BOOL:   return TYPE_BOOL; break;
		case SHV_MODE_TYPE_CHAR:   return TYPE_BYTE; break;
		case SHV_MODE_TYPE_UCHAR:  return TYPE_BYTE; break;
		case SHV_MODE_TYPE_LONG:   return TYPE_DINT; break;
		case SHV_MODE_TYPE_ULONG:  return TYPE_UDINT; break;
		case SHV_MODE_TYPE_FLOAT:  return TYPE_REAL; break;
		case SHV_MODE_TYPE_DOUBLE: return TYPE_REAL; break; // LREAL ?
		default:	return TYPE_UNKNOWN; break;
	}
}

/*
* GetVariableInfo
* Lettura dei dati reltivi ad una variabile PLC
*/
unsigned int GetVariableInfo(char *szData,PLCVAR *pPLCVar)
{
	int nPLCType;
	char *szSep=" ";
	char *pPtr;

	
	/* 
	* Risposta nella forma 
	*
	*	D I <address> <file> : simbolo ordinario
	*		indirizzo : indirizzo della variabile
	*       file      : file contenente la definizione del nome
	*
	*	D S <address> <dim1> <dim2> <flags> <size> <key> : segnale di scambio
	*		indirizzo : indirizzo della variabile
	*		dim1,dim2 : dimensioni del segnale di scambio (1 se scalare)
	*		flags     : caratteristiche del segnale di scambio (bitmask)
	*       size      : dimensioni (in byte) di un elemento del segnale di scambio
	*		key       : chiave dell'elemento selezionato
	*/ 

	
	if((pPtr=strtok(szData,szSep))!=(char *)NULL){ /* Risposta (D o E) */
		if(*pPtr=='E'){
			return FALSE;
		}
	}

	if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ /* Tipo della variabile */
		switch(*pPtr){
			case 'I':	/* simbolo ordinario */
				pPLCVar->usSymbolType=VAR_NORMAL;
			break;
			case 'S':	/* segnale di scambio */
				pPLCVar->usSymbolType=VAR_SIGNAL;
			break;
		}
	}
	if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ /* Indirizzo (in esadecimale) */
		sscanf(pPtr,"%lx",&(pPLCVar->ulAddress));
	}

	switch(pPLCVar->usSymbolType){
		case VAR_NORMAL:
			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ /* file */
				strcpy(pPLCVar->szFile,pPtr);
			}
			/*
			* VARIABILE NORMALE
			* assegno il tipo del PLC e il modo di visualizzazione appropriato
			*/
			pPLCVar->usType=TYPE_BYTE;
		break;
		case VAR_SIGNAL:
			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // dim1
				sscanf(pPtr,"%lx",&(pPLCVar->ulDim1));
			}
			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // dim2
				sscanf(pPtr,"%lx",&(pPLCVar->ulDim2));
			}
			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // flags
				sscanf(pPtr,"%lx",&(pPLCVar->ulFlags));
				/* 
				* leggo il tipo variabile dal PLC 
				*/
				nPLCType=GetVarType(pPLCVar->ulFlags);
			}	

			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // size
				sscanf(pPtr,"%lx",&(pPLCVar->ulSize));
			}
			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // key
				sscanf(pPtr,"%lx",&(pPLCVar->ulKey));
			}
			/*
			* VARIABILE SEGNALE DI SCAMBIO
			* assegno il tipo del PLC e il modo di visualizzazione appropriato
			*/
			pPLCVar->usType=nPLCType;
		break;
	}
	return TRUE;
}

void GetMemChkInfo(PLCVAR *PLCVars,char *szData)
{
	int nItemIndex=0;
	char *szSep=" ";
	char *pPtr;
	
	/* 
	* Risposta nella forma 
	*
	*	D <flag> [<flag> ...]
	*		flag : validitá (1) / non validità (0) dell'area richiesta dalla memchk
	*/ 
	if((pPtr=strtok(szData,szSep))!=(char *)NULL){ // Risposta (D)
	}
	while((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // Flag 0,1,...,n esimo
		PLCVars[nItemIndex].bEnabled=(atoi(pPtr)==0?FALSE:TRUE);
		nItemIndex++;
	}
}

void SendVarListMemChk(int fd,PLCVAR *PLCVars)
{
	int nIndex=0;
	char szParams[512];
	char szVarData[128];
	int nDataLength;
	/* 
	* Inizializzo i parametri del comando memchk 
	*/
	szParams[0]='\0';

	while(strlen(PLCVars[nIndex].szName)){
		if(PLCVars[nIndex].bEnabled){
			nDataLength=PLCTypes[PLCVars[nIndex].usType].nLen;

			sprintf(szVarData,"%lx %x ",PLCVars[nIndex].ulAddress,nDataLength);
			strcat(szParams,szVarData);
		}

		nIndex++;		
	}
	if(strlen(szParams)){
		if(SendCmd(fd,CMD_MEMCHK,szParams,TRUE)){
			GetMemChkInfo(PLCVars,szRcvBuffer);
		}
	}
}

/*
* Leggo i dati di tutti isegnali di scambio interessati
*/
void GetVarsInfo(int fd,PLCVAR *PLCVars)
{
	char szParams[128];
	int nIndex=0;
	PLCVAR *pPLCVar;

	while(strlen(PLCVars[nIndex].szName)){
		pPLCVar=&PLCVars[nIndex];
		sprintf(szParams,"%s %d %d",pPLCVar->szName,pPLCVar->nIndex1,pPLCVar->nIndex2);

		/* 
		* richiesta informazioni al PLC
		*/
		if(SendCmd(fd,CMD_VAR,(char *)szParams,TRUE)){
			GetVariableInfo(szRcvBuffer,pPLCVar);
			pPLCVar->bEnabled=1;
		} else {
			pPLCVar->bEnabled=0;
		}
		nIndex++;
	}
}

/*
* GetDebugValue()
*/
int GetDebugValue(PLCVAR *pPV,char *pszBuffer,int nOffset)
{
	int nIndex;
	unsigned int tmp;
	char szByte[3];

	szByte[2]='\0';
	
	for(nIndex=0;nIndex<PLCTypes[pPV->usType].nLen;nIndex++){
		szByte[0]=pszBuffer[nOffset];
		szByte[1]=pszBuffer[nOffset+1];
		sscanf(szByte,"%x",&tmp);
		pPV->Value.Bytes[nIndex]=(unsigned char)tmp;
		nOffset+=2;
	}
	return PLCTypes[pPV->usType].nLen;
}

/*
* GetMemCopyInfo(char *szData)
* lettura dei valori delle variabili registrate
*/
void GetMemCopyInfo(PLCVAR *PLCVars,char *szData)
{
	int nItemIndex=0;
	int nByteOffset=0;
	int nBytes;
	
	/* 
	* Risposta nella forma 
	*
	*	D <flag> [<flag> ...]
	*		flag : validitá (1) / non validità (0) dell'area richiesta dalla memchk
	*/ 
	while(strlen(PLCVars[nItemIndex].szName)){
		if(PLCVars[nItemIndex].bEnabled){
			nBytes=GetDebugValue(&PLCVars[nItemIndex],szData+2,nByteOffset);
			
			nByteOffset+=nBytes*2;
		}
		nItemIndex++;
	}
}


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
//#include <sys/mman.h>
#include <glib.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#ifdef TRACE
	#include <trace.h>
#endif

#include <proc_list.h>
#include <ep-common.h>

#include "barcode.h"

CFGSTRUCT	Cfg;                /* struttura della configurazione */
int nPID=0;                   /* Process ID */


char szBufTrace[80];
char szBarcode[80];
char szParams[80];
char szRcvBuf[256];
char *pszNomeModulo;
char szTimeBuffer[80];
char szDateBuffer[80];

extern char *optarg;
extern int optind;
extern char rcsid[];
extern char __version__[];
extern char __customer__[];


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
	
	strcpy(szCfgFileName, szCurrentDirectory);
	strcat(szCfgFileName, "/");
	strcat(szCfgFileName, __configuration_file__);

	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");
	GetFileString(szParagraph,"PathTrace",   "../trace",     Cfg.szPathTrace,         80,szCfgFileName, NULL);

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
	* nome della porta seriale utilizzata per il colloquio con lo scanner
	*/
	GetFileString(szParagraph,"CommDevice","/dev/com2", Cfg.szCommDevice, 80,szCfgFileName,NULL);

	/*
	* Parametri di comunicazione utilizzati per il colloquio con lo scanner
	*/
	GetFileString(szParagraph,"CommParams","9600,n,8,1", szBuffer, 80,szCfgFileName,NULL);

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

	trace_debug(TRUE, TRUE, "Started");
#endif


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
	static int nBarcodeReader=-1;
	int nIndex;

	for (;;) {
		nIndex=0;
		nMsgList[nIndex] = BCRD_RCV_BARCODE; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_00; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_01; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_02; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_03; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_04; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_05; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_06; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_07; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_08; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_09; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_10; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_001; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_002; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_003; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_004; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_005; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_006; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_007; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_008; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_009; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_010; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_011; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_012; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_013; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_014; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_015; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_016; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_017; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_018; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_019; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_020; nIndex++;
		nMsgList[nIndex] = BCRD_RCV_BARCODE_GRIPHON; nIndex++;

		nNumMsg = nIndex;

		nRitorno = RecSerMsg(Cfg.nPortFD, szRcvBuf, msgRcv, nMsgList, &nNumMsg, &nMsgLung, 0, TIMEOUT_WITHIN);
	
		/* Controllo il messaggio ricevuto */
		switch(nRitorno){
			case BCRD_RCV_00:
			case BCRD_RCV_01:
			case BCRD_RCV_02:
			case BCRD_RCV_03:
			case BCRD_RCV_04:
			case BCRD_RCV_05:
			case BCRD_RCV_06:
			case BCRD_RCV_07:
			case BCRD_RCV_08:
			case BCRD_RCV_09:
			case BCRD_RCV_10:
				nBarcodeReader=nRitorno-BCRD_RCV_00;
#ifdef TRACE
				trace_debug(TRUE, TRUE, "BCRD: [%2d]", nRitorno-BCRD_RCV_00);
#endif
			break;

			case BCRD_RCV_001:
			case BCRD_RCV_002:
			case BCRD_RCV_003:
			case BCRD_RCV_004:
			case BCRD_RCV_005:
			case BCRD_RCV_006:
			case BCRD_RCV_007:
			case BCRD_RCV_008:
			case BCRD_RCV_009:
			case BCRD_RCV_010:
			case BCRD_RCV_011:
			case BCRD_RCV_012:
			case BCRD_RCV_013:
			case BCRD_RCV_014:
			case BCRD_RCV_015:
			case BCRD_RCV_016:
			case BCRD_RCV_017:
			case BCRD_RCV_018:
			case BCRD_RCV_019:
			case BCRD_RCV_020:
				strncpy(szBarcode,szRcvBuf,3);
				szBarcode[3]='\0';
#ifdef TRACE
				trace_debug(TRUE, TRUE, "BCRD [%d]: Ricevuto Barcode :[%s]", nBarcodeReader,szBarcode);
#endif
				if(nBarcodeReader!=-1){
					sprintf(szParams,"%d,%s",nBarcodeReader,szBarcode);
					SendMessage(Cfg.nMainID, nPID,  BCRD_RX_BARCODE, szParams);
				} else {
					sprintf(szParams,"%d,%s",0,szBarcode);
					SendMessage(Cfg.nMainID, nPID,  BCRD_RX_BARCODE, szParams);
				}
			break;

			case BCRD_RCV_BARCODE:
				/*
				* barcode : <STX> + dato + <CR> + <LF>
				* dato : barcode letto o <CAN>
				*/
				strncpy(szBarcode,szRcvBuf+1,nMsgLung-2);
				szBarcode[nMsgLung-3]='\0';
				if(szBarcode[0]==CAN_CHAR){
#ifdef TRACE
					trace_debug(TRUE, TRUE, "BCRD [%d]: Ricevuto NOREAD",nBarcodeReader);
#endif
					if(nBarcodeReader!=-1){
						sprintf(szParams,"%d",nBarcodeReader);
						SendMessage(Cfg.nMainID, nPID,  BCRD_RX_NOREAD, szParams);
					} else {
						sprintf(szParams,"%d",0);
						SendMessage(Cfg.nMainID, nPID,  BCRD_RX_NOREAD, szParams);
					}
				} else {
#ifdef TRACE
					trace_debug(TRUE, TRUE, "BCRD [%d]: Ricevuto Barcode :[%s]", nBarcodeReader,szBarcode);
#endif
					if(nBarcodeReader!=-1){
						sprintf(szParams,"%d,%s",nBarcodeReader,szBarcode);
						SendMessage(Cfg.nMainID, nPID,  BCRD_RX_BARCODE, szParams);
					} else {
						sprintf(szParams,"%d,%s",0,szBarcode);
						SendMessage(Cfg.nMainID, nPID,  BCRD_RX_BARCODE, szParams);
					}
					/* nBarcodeReader=-1; */
				}
			break;

			case BCRD_RCV_BARCODE_GRIPHON:
				/*
				* barcode : dato + <CR> + <LF>
				* dato : barcode letto o <CAN>
				*/
				strncpy(szBarcode,szRcvBuf,nMsgLung-2);
				szBarcode[nMsgLung-2]='\0';
				if(szBarcode[0]==CAN_CHAR){
#ifdef TRACE
					trace_debug(TRUE, TRUE, "BCRD [%d]: Ricevuto NOREAD",nBarcodeReader);
#endif
					if(nBarcodeReader!=-1){
						sprintf(szParams,"%d",nBarcodeReader);
						SendMessage(Cfg.nMainID, nPID,  BCRD_RX_NOREAD, szParams);
					} else {
						sprintf(szParams,"%d",0);
						SendMessage(Cfg.nMainID, nPID,  BCRD_RX_NOREAD, szParams);
					}
				} else {
#ifdef TRACE
					trace_debug(TRUE, TRUE, "BCRD [%d]: Ricevuto Barcode :[%s]", nBarcodeReader,szBarcode);
#endif
					if(nBarcodeReader!=-1){
						sprintf(szParams,"%d,%s",nBarcodeReader,szBarcode);
						SendMessage(Cfg.nMainID, nPID,  BCRD_RX_BARCODE, szParams);
					} else {
						sprintf(szParams,"%d,%s",0,szBarcode);
						SendMessage(Cfg.nMainID, nPID,  BCRD_RX_BARCODE, szParams);
					}
					/* nBarcodeReader=-1; */
				}
			break;

			case BCRD_RCV_NOREAD:
#ifdef TRACE
				trace_debug(TRUE, TRUE, "BCRD: Ricevuto NOREAD");
#endif
				if(nBarcodeReader!=-1){
					sprintf(szParams,"%d",nBarcodeReader);
					SendMessage(Cfg.nMainID, nPID,  BCRD_RX_NOREAD, szParams);
				} else {
					sprintf(szParams,"%d",0);
					SendMessage(Cfg.nMainID, nPID,  BCRD_RX_NOREAD, szParams);
				}
				nBarcodeReader=-1;
			break;

			default:
#ifdef TRACE
				trace_debug(FALSE, TRUE, "BCRD: Errore numero %d", nRitorno);
#endif
				nBarcodeReader=-1;
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

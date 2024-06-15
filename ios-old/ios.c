/*
* ios.c : gestione IOS
* 
* Progetto Easy Picking 4.0 : Wella
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*
* Modifiche per impanto Wella:
* - Gestione di n schede IOS per tastiere su piu' isole
* - Gestione di n displays per singolo settore
*/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <glib.h>

#ifdef HAS_IOS_CNI
	#include <hwcni.h>
#endif

#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif

#include <picking.h>
#include <proc_list.h>

#include <ep-common.h>

#include "ios.h"

#define SARTI

int nAllMsg;
int nCarIn;
ep_msg_header_t InMsgStruct;
char szInMsg[INFO_LEN];
char szText[INFO_LEN];
char szBufTrace[80];
char *pszNomeModulo;

char szTimeBuffer[80];
char szDateBuffer[80];


int nPID=0;                   /* Process ID */
CFGSTRUCT	Cfg;                /* struttura della configurazione */

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
	char szCfgFileName[128];
	int nIndex;

	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	
	strcpy(szCfgFileName,szCurrentDirectory);
	strcat(szCfgFileName,"/");
	strcat(szCfgFileName, __configuration_file__);

	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");
	Cfg.nDebugLevel=GetFileInt(szParagraph,"DebugLevel",0,szCfgFileName,NULL);
	GetFileString(szParagraph,"PathExe",    "./",          Cfg.szPathExe,      80, szCfgFileName, NULL);
	GetFileString(szParagraph,"PathTrace",  "./",          Cfg.szPathTrace,    80, szCfgFileName, NULL);

	ReadProcInfo(szCfgFileName);

	/*
	* parametri specifici dell'applicazione
	*/
	/*
	* Delay in millisecondi 
	*/
	Cfg.nDelay=GetFileInt(ProcessGetName(PROC_IOS),"Delay",100,szCfgFileName,NULL);
	/*
	* Delay in millisecondi per la simulazione pressione tasto
	*/
	Cfg.nDelaySimulazione=GetFileInt(ProcessGetName(PROC_IOS),"DelaySimulazione",100,szCfgFileName,NULL);
	/*
	* Numero delle schede IOS connesse (da aprire)
	*/
	Cfg.nIOS=GetFileInt(ProcessGetName(PROC_IOS),"IOSNumber",1,szCfgFileName,NULL);
	/*
	* Numero dei moduli PKL 
	*/
	Cfg.nPKLNumber=GetFileInt(ProcessGetName(PROC_IOS),"PKLNumber",1,szCfgFileName,NULL);

	/*
	* dati relativi ai moduli PKL
	*/
	for(nIndex=0;nIndex<Cfg.nPKLNumber;nIndex++){
		sprintf(szProcIndex,"PKL_%02d",nIndex);
		GetFileString(ProcessGetName(PROC_IOS),szProcIndex, "", szProcBuffer, 80,szCfgFileName,NULL);

		sscanf(szProcBuffer, "%d,%d,%d",
			&(Cfg.PKL[nIndex].nIOS),
			&(Cfg.PKL[nIndex].nCPU),
			&(Cfg.PKL[nIndex].nModulo));
	}

	/*
	* Simulazione IOS
	*/
	Cfg.nSimulazione=GetFileInt(ProcessGetName(PROC_IOS),"Simulazione",0,szCfgFileName,NULL);

}


/*
* WaitIOS()
* attendo che il codice comando in TX del modulo 
* e della IOS passate in ingresso sia libero (0)
* rm 06-09-2001 : aggiunta gestione timeout
*/
void WaitIOS(int nIOS,int nCPU,int nModulo)
{
	struct timeval tvActualTime;
	struct timeval tvStartTime;
	int nTimePassed=0;    /* msecs */

	gettimeofday(&tvStartTime,NULL);

#ifdef HAS_IOS_CNI
	if(nIOS<Cfg.nIOS){
		while(nTimePassed < IOS_TIMEOUT && IOS_TX_BYTE(nIOS,nCPU,nModulo,0)){

			gettimeofday(&tvActualTime,NULL);

			nTimePassed+=(tvActualTime.tv_sec-tvStartTime.tv_sec)*1000;     /* secondi */
			nTimePassed+=(tvActualTime.tv_usec-tvStartTime.tv_usec)/1000;   /* milli secondi */
#ifdef TRACE
			trace_debug(FALSE, TRUE, "TimePassed = %6d",nTimePassed);
#endif

			usleep(1000*Cfg.nDelay);
		}
		if(nTimePassed > IOS_TIMEOUT){
			char szMsg[128];

#ifdef TRACE
			trace_debug(FALSE, TRUE, "Errore di TIMEOUT su IOS [%d.%d.%d]",nIOS,nCPU,nModulo);
#endif
			sprintf(szMsg, "Errore di TIMEOUT su IOS [%d.%d.%d]\n",nIOS,nCPU,nModulo);
			SendMessage(PROC_MAIN, PROC_IOS, DISPLAY_MSG, szMsg);
		}
	}
	/*
	* rm 27-02-2001 : inserito ritardo per mancata accensione luce
	*                 rif. (Sarti)
	*/
#ifdef SARTI
	usleep(20000);
#endif
#endif
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
	int nOpt;

	while ((nOpt = getopt(argc, argv, "p:")) != -1) {
		switch(nOpt){
			case 'p':	/* Process ID */
				nPID=atoi(optarg);
			break;
		}
	}
	if(nPID==0){
		nPID=PROC_IOS;
	}

	pszNomeModulo = argv[0];

	/*
	* Lettura della configurazione corrente
	*/
	ReadConfiguration();

#ifdef TRACE
	/* Apro il file trace */
    char szBufTrace[80];
    char *pszNomeModulo;

	pszNomeModulo = g_path_get_basename (argv[0]);

	/* Apro il file trace */
    strcpy(szBufTrace, Cfg.szPathTrace);
    strcat(szBufTrace,"/");
    strcat(szBufTrace, pszNomeModulo);
    strcat(szBufTrace, TRACE_FILE_SUFFIX);

	open_trace(pszNomeModulo, szBufTrace, PICKING_TRACE_FILE_MAXSIZE);

	trace_debug(FALSE, TRUE, "Module Name     : %s",pszNomeModulo);
	trace_debug(FALSE, TRUE, "Version         : %s",__version__);
	trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
	trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
	trace_debug(FALSE, TRUE, "Simulazione     : %d",Cfg.nSimulazione);

	trace_debug(TRUE, TRUE, "Started");
#endif

	/* Apro la coda messaggi principale */
	if((OpenProcessMsgQ(PROC_MAIN))<0){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "%s : Apertura coda messaggi principale fallita", pszNomeModulo);
#endif
	}
	/* Apro la coda messaggi processo Settori */
	if((OpenProcessMsgQ(PROC_SETTORI))<0){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "%s : Apertura coda messaggi settori fallita", pszNomeModulo);
#endif
	}
	/* Apro la coda messaggi locale */
	if((CreateProcessMsgQ( PROC_IOS, 1))<0){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "%s : Apertura coda messaggi locale fallita", pszNomeModulo);
#endif
	}


	/* Predispongo l'utilizzo dell'uscita di sicurezza */
	signal(SIGTERM, SafeIntFunc);

#ifdef HAS_IOS_CNI
	for(nIndex=0;nIndex<Cfg.nIOS;nIndex++){
		Cfg.nFdIOS[nIndex]=0xff;	/* NON NULL VALUE */
		nRC=iosOpenGeneral(nIndex,&(Cfg.IOSInfo[nIndex]),&Cfg.nFdIOS[nIndex]);
		/*
		* gestire il valore di ritorno ....
		*/
		if(nRC==-1){
#ifdef TRACE
			trace_debug(FALSE, TRUE, "Errore in apertura Scheda IOS Numero [%2d]",nIndex);
#endif
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
	* avverto MAIN dello start
	*/
	SendMessage(PROC_MAIN, PROC_IOS, PROGRAM_STARTED, NULL);


	/*
	* gestione program loop
	*/
	for(;;){

		/*
		* Gestione Messaggi Ricevuti
		*/
		nAllMsg = 0;
        if((nCarIn = ProcessReceiveMsgNoWait(PROC_IOS, &nAllMsg, szInMsg))>=0){
			UnpackStructMsg(szInMsg, &InMsgStruct, szText, nCarIn);

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
							if(szText==NULL){
								break;
							}
							sscanf(szText,"%d,%d,%d,%d,%d",&nIOS,&nCPU,&nModule,&nRowIndex,&nColIndex);

							WaitIOS(nIOS,nCPU,nModule);

#ifdef TRACE
							trace_debug(TRUE, TRUE, "IOS_PKL_SET_RC [IOS:%d,CPU:%d,M:%2d,R:%2d,C:%2d]", nIOS,nCPU,nModule,nRowIndex,nColIndex);
#endif
							if(nIOS<Cfg.nIOS){
								IOS_TX_BYTE(nIOS,nCPU,nModule,1)=(BYTE)nColIndex;      /* riga */
								IOS_TX_BYTE(nIOS,nCPU,nModule,2)=(BYTE)nRowIndex;      /* colonna */
								IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)PKL_LIGHT_ON;   /* comando */
							}

							WaitIOS(nIOS,nCPU,nModule);
							/*
							* se sono in simulazione restituisco la pressione tasto
							*/
							if(Cfg.nSimulazione){
								usleep(1000*Cfg.nDelaySimulazione);

								sprintf(szSendBuffer,"%d,%d,%d,%d,%d",nIOS,nCPU,nModule,nRowIndex,nColIndex);
#ifdef TRACE
								trace_debug(TRUE, TRUE, "SIM: PKL_BUTTON_PRESSED [IOS:%d,CPU:%d,M:%2d,R:%2d,C:%2d]", nIOS,nCPU,nModule,nRowIndex,nColIndex);
#endif

                                SendMessage(PROC_SETTORI, PROC_IOS, IOS_PKL_BUTTON_PRESSED, szSendBuffer);

								if(nIOS<Cfg.nIOS){
									IOS_TX_BYTE(nIOS,nCPU,nModule,1)=(BYTE)nColIndex;      /* riga */
									IOS_TX_BYTE(nIOS,nCPU,nModule,2)=(BYTE)nRowIndex;      /* colonna */
									IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)PKL_LIGHT_OFF;  /* comando */
								}
							}
						break;
						case IOS_PKL_RESET_RC:
							/* 
							* Parametri : Scheda IOS,CPU,Modulo,Riga,Colonna
							*/
							if(szText==NULL){
								break;
							}
							sscanf(szText,"%d,%d,%d,%d,%d",&nIOS,&nCPU,&nModule,&nRowIndex,&nColIndex);
#ifdef TRACE
							trace_debug(TRUE, TRUE, "IOS_PKL_RESET_RC [IOS:%d,CPU:%d,M:%2d R:%2d C:%2d]", nIOS, nCPU, nModule,nRowIndex,nColIndex);
#endif
							WaitIOS(nIOS,nCPU,nModule);

							if(nIOS<Cfg.nIOS){
								IOS_TX_BYTE(nIOS,nCPU,nModule,1)=(BYTE)nColIndex;      /* riga */
								IOS_TX_BYTE(nIOS,nCPU,nModule,2)=(BYTE)nRowIndex;      /* colonna */
								IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)PKL_LIGHT_OFF;  /* comando */
							}

							WaitIOS(nIOS,nCPU,nModule);
						break;
						case IOS_PKL_CONGELA:
							/* 
							* Parametri : Scheda IOS,CPU,Modulo
							*/
							sscanf(szText,"%d,%d,%d",&nIOS,&nCPU,&nModule);
#ifdef TRACE
							trace_debug(TRUE, TRUE, "IOS_PKL_CONGELA [IOS:%d,CPU:%d,M:%2d]", nIOS, nCPU, nModule);
#endif
							WaitIOS(nIOS,nCPU,nModule);

							if(nIOS<Cfg.nIOS){
								IOS_TX_BYTE(nIOS,nCPU,nModule,1)=(BYTE)1;              /* azione */
								IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)PKL_FREEZE;     /* comando */
							}

							WaitIOS(nIOS,nCPU,nModule);
						break;
						case IOS_PKL_SCONGELA:
							/* 
							* Parametri : Scheda IOS,CPU,Modulo
							*/
							sscanf(szText,"%d,%d,%d",&nIOS,&nCPU,&nModule);
#ifdef TRACE
							trace_debug(TRUE, TRUE, "IOS_PKL_SCONGELA [IOS:%d,CPU:%d,M:%2d]", nIOS, nCPU, nModule);
#endif
							WaitIOS(nIOS,nCPU,nModule);

							if(nIOS<Cfg.nIOS){
								IOS_TX_BYTE(nIOS,nCPU,nModule,1)=(BYTE)0;              /* azione */
								IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)PKL_FREEZE;     /* comando */
							}

							WaitIOS(nIOS,nCPU,nModule);
						break;
						case IOS_PKL_RESET_ALL:
							/* 
							* Parametri : Scheda IOS,CPU,Modulo
							*/
							sscanf(szText,"%d,%d,%d",&nIOS,&nCPU,&nModule);
#ifdef TRACE
							trace_debug(TRUE, TRUE, "IOS_PKL_RESET_ALL [IOS:%d,CPU:%d,M:%2d]", nIOS, nCPU, nModule);
#endif
							WaitIOS(nIOS,nCPU,nModule);

							if(nIOS<Cfg.nIOS){
								IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)PKL_RESET;      /* comando */
							}
							WaitIOS(nIOS,nCPU,nModule);
						break;
						case IOS_PKL_TEST:
							/* 
							* Parametri : Scheda IOS,CPU,Modulo
							*/
							sscanf(szText,"%d,%d,%d",&nIOS,&nCPU,&nModule);
							WaitIOS(nIOS,nCPU,nModule);

							if(nIOS<Cfg.nIOS){
								IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)PKL_TEST;       /* comando */
							}
							WaitIOS(nIOS,nCPU,nModule);
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

							if(nIOS<Cfg.nIOS){
								IOS_TX_BYTE(nIOS,nCPU,nModule,1)=(BYTE)nDigit;           /* digit */
								IOS_TX_BYTE(nIOS,nCPU,nModule,2)=(BYTE)nDPYByte[0];      /* ascii */
								IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)nComando;         /* comando */
							}
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

								if(nIOS<Cfg.nIOS){
									IOS_TX_BYTE(nIOS,nCPU,nModule,1)=(BYTE)((nRowIndex*4)+nIndex*2); /* digit */
									IOS_TX_BYTE(nIOS,nCPU,nModule,2)=(BYTE)nDPYByte[nIndex*2];       /* ascii */
									IOS_TX_BYTE(nIOS,nCPU,nModule,3)=(BYTE)nDPYByte[nIndex*2+1];     /* ascii */
									IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)nComando;                 /* comando */
								}
								WaitIOS(nIOS,nCPU,nModule);

							}
						}
						break;

						case IOS_DPY_BCD:
						{
							int nComando;
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

							if(nIOS<Cfg.nIOS){
								IOS_TX_BYTE(nIOS,nCPU,nModule,1)=(BYTE)nDPYByte[0];	/* Digit 0-1 o 4-5 */
								IOS_TX_BYTE(nIOS,nCPU,nModule,2)=(BYTE)nDPYByte[1];	/* Digit 2-3 o 6-7 */
								IOS_TX_BYTE(nIOS,nCPU,nModule,3)=(BYTE)nDPYByte[2];	/* Flags per blink e '.' */
								IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)nComando;    /* comando */
							}
							
							WaitIOS(nIOS,nCPU,nModule);

						}
						break;
						case IOS_TEST_CMD:
						{
							int nIndex;
							int nComando;
							int nByte[4];
							int nNumBytes;

							/* ricavo i dati da spedire */
							sscanf(szText,"%d,%d,%d,%x,%x,%x,%x,%x,%x",
								&nIOS,
								&nCPU,
								&nModule,
								&nComando,
								&nNumBytes,
								&nByte[0],
								&nByte[1],
								&nByte[2],
								&nByte[3]);

#ifdef TRACE
							trace_debug(TRUE, TRUE, "IOS_TEST_CMD (%x) [IOS:%d,CPU:%d,M:%2d %x:%x:%x:%x]", nComando, nIOS, nCPU, nModule, nByte[0], nByte[1], nByte[2], nByte[3]);
#endif
							WaitIOS(nIOS,nCPU,nModule);

							if(nIOS<Cfg.nIOS){
								for(nIndex=0;nIndex<nNumBytes;nIndex++){
									IOS_TX_BYTE(nIOS,nCPU,nModule,nIndex+1)=(BYTE)nByte[nIndex];
								}
								IOS_TX_BYTE(nIOS,nCPU,nModule,0)=(BYTE)nComando;                 /* comando */
							}

							WaitIOS(nIOS,nCPU,nModule);

						}
						break;
					}
				break;
			}
		}

		/*
		* controllo gli eventi della IOS (tastiera o errore su lampada)
		* Byte 0 in ricezione
		*/
		for(nIndex=0;nIndex<Cfg.nPKLNumber;nIndex++){
			nIOS=Cfg.PKL[nIndex].nIOS;
			nCPU=Cfg.PKL[nIndex].nCPU;
			nModule=Cfg.PKL[nIndex].nModulo;

			switch(IOS_RX_BYTE(nIOS,nCPU,nModule,0)){
				case PKL_BUTTON_PRESSED:
					nColIndex=IOS_RX_BYTE(nIOS,nCPU,nModule,1);
					nRowIndex=IOS_RX_BYTE(nIOS,nCPU,nModule,2);

					/* reset comando */
					IOS_RX_BYTE(nIOS,nCPU,nModule,0)=0x0;

#ifdef TRACE
					trace_debug(TRUE, TRUE, "PKL_BUTTON_PRESSED [IOS:%d,CPU:%d,M:%2d,R:%2d,C:%2d]", nIOS,nCPU,nModule,nRowIndex,nColIndex);
#endif
					sprintf(szSendBuffer,"%d,%d,%d,%d,%d",nIOS,nCPU,nModule,nRowIndex,nColIndex);

                    SendMessage(PROC_SETTORI, PROC_IOS, IOS_PKL_BUTTON_PRESSED, szSendBuffer);

					/*
					* attendo che il codice comando in TX sia libero (0)
					*/
					WaitIOS(nIOS,nCPU,nModule);

					/*
					* Attesa forzata per evitare la perdita di comandi da parte della IOS
					*/
#ifdef SARTI
					usleep(20000);
#endif

					/* 
					* replica del comando in TX 
					*/
					IOS_TX_BYTE(nIOS,nCPU,nModule,0)=PKL_BUTTON_PRESSED;	

					WaitIOS(nIOS,nCPU,nModule);
					
				break;

				case PKL_LIGHT_DAMAGED:
					nColIndex=IOS_RX_BYTE(nIOS,nCPU,nModule,1);
					nRowIndex=IOS_RX_BYTE(nIOS,nCPU,nModule,2);

					/* reset comando */
					IOS_RX_BYTE(nIOS,nCPU,nModule,0)=0x0;


#ifdef TRACE
					trace_debug(TRUE, TRUE, "PKL_LIGHT_DAMAGED [IOS:%d,CPU:%d,M:%2d,R:%2d,C:%2d]", nIOS,nCPU,nModule,nRowIndex,nColIndex);
#endif
					sprintf(szSendBuffer,"%d,%d,%d,%d,%d",nIOS,nCPU,nModule,nRowIndex,nColIndex);

                    SendMessage(PROC_SETTORI, PROC_IOS, IOS_PKL_LIGHT_DAMAGED, szSendBuffer);


					/*
					* attendo che il codice comando in TX sia libero (0)
					*/
					WaitIOS(nIOS,nCPU,nModule);

					/*
					* Attesa forzata per evitare la perdita di comandi da parte della IOS
					*/
#ifdef SARTI
					usleep(20000);
#endif
					/* 
					* replica del comando in TX 
					*/
					IOS_TX_BYTE(nIOS,nCPU,nModule,0)=PKL_LIGHT_DAMAGED;	

					WaitIOS(nIOS,nCPU,nModule);

				break;
				case PKL_NULL_CMD:
					/*
					* non fare niente ...
					*/
				break;
				default:
				{
					BYTE bComando;
					
					/* comando non gestito */
					bComando=IOS_RX_BYTE(nIOS,nCPU,nModule,0);
					/* reset comando */
					IOS_RX_BYTE(nIOS,nCPU,nModule,0)=0x0;
#ifdef TRACE
					trace_debug(TRUE, TRUE, "Comando [%d] [%d,%d,%d,%2d,%2d]", bComando, nIOS,nCPU,nModule,nRowIndex,nColIndex);
#endif

					/*
					* attendo che il codice comando in TX sia libero (0)
					*/
					WaitIOS(nIOS,nCPU,nModule);

					/*
					* Attesa forzata per evitare la perdita di comandi da parte della IOS
					*/
#ifdef SARTI
					usleep(20000);
#endif
					/* 
					* replica del comando in TX 
					*/
					IOS_TX_BYTE(nIOS,nCPU,nModule,0)=bComando;	

					WaitIOS(nIOS,nCPU,nModule);
				}
				break;
			}
		}
		usleep(1000*Cfg.nDelay);
	}	 /* fine ciclo */
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


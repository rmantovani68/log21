/*
* settori.c : gestione settori di magazzino
* 
* Progetto Easy Picking 3.0 : Wella
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/stat.h>
#include <glib.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <picking.h>
#include <proc_list.h>
#include <ep-common.h>
#include <ep-db.h>

#include	"linklist.h"
#include	"settstruct.h"
#include	"settext.h"
#include	"settfun.h"

/*
* main()
*/
int main(int argc,char** argv)
{
	int nIndex;
	int nErr=0;
	int nOpt;
	BOOL bTest=FALSE;
	BOOL bDisplay=FALSE;
	BOOL bLoop=FALSE;
	BOOL bCorrezione=FALSE;
	int nSettore=0;
	char szFila[20];

	szFila[0]='\0';

	while ((nOpt = getopt(argc, argv, "actdls:p:f:")) != -1) {
		switch(nOpt){
			case 'p':	/* Process ID */
				nPID=atoi(optarg);
			break;
			case 'a':	/* alone */
				Cfg.bAlone=TRUE;
			break;
			case 't':	/* test */
				bTest=TRUE;
			break;
			case 'd':	/* test display*/
				bDisplay=TRUE;
			break;
			case 's':	/* test luci accese */
				nSettore=atoi(optarg);
			break;
			case 'f':	/* test luci accese */
				strcpy(szFila,optarg);
			break;
			case 'l':	/* test loop */
				bLoop=TRUE;
			break;
			case 'c':	/* Correzione ubicazioni (scambio colonne) */
				bCorrezione=TRUE;
			break;
			default:
				nErr++;
			break;
		}
	}
 
	if(nPID==0){
		nPID=PROC_SETTORI;
		fprintf(stderr,"PID non definito : utilizzo %d\n",nPID);
	}

	if(nErr){
		printf("%s : copyright 1995-2002 HSD - A&L\n",argv[0]);
		printf("Uso : %s [-d]\n",argv[0]);
		printf("\t-a    : Run Alone\n");
		printf("\t-t    : Run Test\n");

		exit(0);
	}

	/* nome dell'eseguibile */
	pszNomeModulo = argv[0];

	/* Lettura della configurazione */
	ReadConfiguration(TRUE);

#ifdef TRACE
	/* Apro il file trace */
	sprintf(szBufTrace, "%s/%s%s", Cfg.szPathTrace, g_path_get_basename (pszNomeModulo), TRACE_FILE_SUFFIX);
	open_trace(pszNomeModulo, szBufTrace, TRACE_FILE_MAXSIZE);

	trace_out_vstr(1,"Module Name     : %s",argv[0]);
	trace_out_vstr(1,"Version         : %s",__version__);
	trace_out_vstr(1,"Customer Name   : %s",__customer__);
	trace_out_vstr(1,"Authors         : %s",__authors__);
	trace_out_vstr(1,"Copyright       : %s",__copyright__);
	trace_out_vstr(1,"RCSID String    : %s",rcsid);
	trace_out_vstr(1,"Stand Alone Mode: %s",Cfg.bAlone?"ON":"OFF");
	trace_out_vstr_date(1,"Started");
#endif


	/* Connessione al DataBase Postgres */
	if(!DBConnect(Cfg.szPGHost, Cfg.szPGPort, Cfg.szPGDataBase)){
		trace_out_vstr_date(1, "Connection to database %s [Host %s] failed.",Cfg.szPGDataBase,Cfg.szPGHost);
		exit(1);
	}

	/* Lettura dei parametri relativi ai displays */
	Cfg.nNumeroDisplays=ReadDisplaysInfo(Cfg.Displays);

	/*
	* alloco la shared memory dei settori
	*/
	if((pSettori=(PSETTORE)SetupShm(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE)))==NULL){
#ifdef TRACE
		trace_out_vstr_date(1,"Allocazione Shared Memory FALLITA [%d] (Settori)", Cfg.nSettoriShmKey);
#endif
	}
	memset(pSettori,0,Cfg.nNumeroSettori*sizeof(SETTORE));

	ReadSettoriInfo(Cfg.nNumeroSettori);

	if(Cfg.bAlone){
		/* Creo la coda messaggi locale */
        if(CreateProcessMsgQ(PROC_SETTORI, 1)<0){
#ifdef TRACE
			trace_out_vstr_date(1, "Creazione coda messaggi locale fallita");
#endif
		}
		/*
		* Leggo la shared memory per la gestione della linea
		*/
		if((pDatiLinea=(PLINEA_STRUCT)SetupShm(Cfg.nShmKey,sizeof(LINEA_STRUCT)))==NULL){
#ifdef TRACE
			trace_out_vstr_date(1,"Cannot Create Shared Memory [%d] !\n", Cfg.nShmKey);
#endif
			pDatiLinea=(PLINEA_STRUCT)NULL;
		}
		RunSimpleProcess(PROC_IOS, Cfg.szPathExe);       /* gestione IOS */

	} else {
		/* Apro la coda messaggi principale */
        if((OpenProcessMsgQ(PROC_MAIN))<0){
#ifdef TRACE
			trace_out_vstr_date(1, "Apertura coda messaggi principale fallita");
#endif
		}
		/* Apro la coda messaggi locale */
		if((CreateProcessMsgQ( PROC_SETTORI, 1))<0){
#ifdef TRACE
			trace_out_vstr_date(1, "Apertura coda messaggi locale fallita");
#endif
		}
		/* Apro la coda messaggi di VOLUM */
        if((OpenProcessMsgQ(PROC_VOLUM))<0){
#ifdef TRACE
			trace_out_vstr_date(1, "Apertura coda messaggi VOLUM fallita");
#endif
		}
		/* Apro la coda messaggi di IOS */
        if((OpenProcessMsgQ(PROC_IOS))<0){
#ifdef TRACE
			trace_out_vstr_date(1, "Apertura coda messaggi IOS fallita");
#endif
		}
	}

	/*
	* Leggo la shared memory per la gestione della linea
	*/
	if((pDatiLinea=(PLINEA_STRUCT)OpenSharedMemory(Cfg.nShmKey,sizeof(LINEA_STRUCT)))==NULL){
#ifdef TRACE
		trace_out_vstr_date(1,"Cannot Get Shared Memory [%d] !", Cfg.nShmKey);
#endif
		pDatiLinea=(PLINEA_STRUCT)NULL;
	}

	/* Predispongo l'utilizzo dell'uscita di sicurezza */
	signal(SIGTERM, SafeIntFunc);
	signal(SIGINT,SafeIntFunc);
	signal(SIGQUIT,SafeIntFunc);
	signal(SIGILL,SafeIntFunc);
	signal(SIGKILL,SafeIntFunc);

	signal(SIGUSR1,SaveStateIntFunc);

	/* 
	* modifico il livello di protezione di lettura / scrittura dei files
	*/
	umask(0);

	ReadUbicazioni(&Ubicazioni);
	SortUbicazioni(&Ubicazioni);

	/*
	* avverto MAIN dello start
	*/
	SendMessage(PROC_MAIN, PROC_SETTORI,  PROGRAM_STARTED, NULL);

	/*
	* Test settori
	* Vengono accese le luci in sequenza di prelievo e si attende pressione tasto da operatore
	*/
	if(Cfg.bAlone && bTest){
		BOOL bTastoOK=FALSE;
		char szBuffer[80];
		char szFilaUbicazione[80];
		PGresult *PGRes = NULL;
		int nSettoreIndex;
		int nDisplayIndex;
		int nIOS;
		int nCPU;
		int nModulo;
		int nRow;
		int nCol;
		PDISPLAYSTRUCT pDisplay;
		int nIndex;
		int nTuples;

		do {
			ResetLuci();
			ResetSettori();
			ScongelaPKL();

			/*
			* Selezione di tutte le ubicazioni dell'impianto
			*/
			PGRes=DBExecQuery(TRUE,"select ubcdubi, ubcdpro, ubnmisl, ubnmset, ubnmdsp, ubplcnm, ubprior, ubstato, settore(ubcdpro) from ubicazioni where ubtpubi = '%s' and ubcdflg='%c' order by ubnmisl,ubnmset,ubprior;",Cfg.szTipoOrdini,UBICAZIONE_AUTOMATICA);

			nTuples=DBntuples(PGRes);

			for(nIndex=0;nIndex<nTuples;nIndex++){
				nSettoreIndex=atoi(DBgetvalue(PGRes,nIndex,3));
				nDisplayIndex=atoi(DBgetvalue(PGRes,nIndex,4));
				nPLCNum=atoi(DBgetvalue(PGRes,nIndex,5));
				strcpy(szFilaUbicazione,DBgetvalue(PGRes,nIndex,6));
				pDisplay=&(Cfg.Displays[nDisplayIndex-1]);

				/*
				* rm 07-08-2001
				* Gestione test su settore specifico
				*/
				if(!nSettore || nSettore==nSettoreIndex){
					if(!strlen(szFila) || szFila[0]==szFilaUbicazione[0]){
						sprintf(pDisplay->szRiga_1_Display,"%04d",nRow);     /* Riga 1 del display */
						sprintf(pDisplay->szRiga_2_Display,"%04d",nCol);     /* Riga 2 del display */
						pDisplay->nStatoRiga1=NORMAL;
						pDisplay->nStatoRiga2=NORMAL;
						UpdateDisplay(pDisplay,TUTTO);

						sprintf(szBuffer,"%d,%d,%d,%d,%d", nIOS,nCPU,nModulo,nRow,nCol);
						SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_SET_RC_SETTORE_NUMERO, szBuffer);

						bTastoOK=FALSE;

						while(!bTastoOK){
							nAllMsg = 0;
                            if((nCarIn = ProcessReceiveMsgNoWait(PROC_SETTORI, &nAllMsg, szInMsg))>=0){
								UnpackStructMsg(szInMsg, &InMsgStruct, szText, nCarIn);

								switch (InMsgStruct.srce){
									case PROC_IOS:
										switch(InMsgStruct.code){
											case IOS_PKL_BUTTON_PRESSED_SETTORE_NUMERO:
											{
												PUBICAZIONE pUbicazione;
												int nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex;

												sscanf(szText,"%d,%d,%d,%d,%d",&nMsgIOS,&nMsgCPU,&nMsgModulo,&nMsgRowIndex,&nMsgColIndex);

	#ifdef TRACE
												trace_out_vstr_date(1, "TEST : Ricevuto IOS_PKL_BUTTON_PRESSED [IOS:%d CPU:%d m:%02d,r:%02d,c:%02d]", nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex);
	#endif

												if((pUbicazione=SearchUbicazione(&Ubicazioni,nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex))!=NULL){
													if(nMsgIOS==nIOS && nMsgCPU==nCPU && nMsgModulo==nModulo && nMsgRowIndex==nRow && nMsgColIndex==nCol){
														bTastoOK=TRUE;
													}
												}
											}
											break;
											case IOS_PKL_LIGHT_DAMAGED:
											{
												PUBICAZIONE pUbicazione;
												int nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex;

												sscanf(szText,"%d,%d,%d,%d,%d",&nMsgIOS,&nMsgCPU,&nMsgModulo,&nMsgRowIndex,&nMsgColIndex);

	#ifdef TRACE
												trace_out_vstr_date(1, "TEST : Ricevuto IOS_PKL_LIGHT_DAMAGED [IOS:%d CPU:%d m:%02d,r:%02d,c:%02d]", nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex);
	#endif

												if((pUbicazione=SearchUbicazione(&Ubicazioni,nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex))!=NULL){
													if(nMsgIOS==nIOS && nMsgCPU==nCPU && nMsgModulo==nModulo && nMsgRowIndex==nRow && nMsgColIndex==nCol){
														bTastoOK=TRUE;
													}
												}
											}
											break;
										}
									break;
								}
							}
							usleep(1000*Cfg.nDelay);
						}
						usleep(1000*Cfg.nDelay);
					}
				}
			}
			/*
			* Fine del test
			*/
			DBclear(PGRes);
		} while(bLoop);
	}



	ResetLuci();
	ResetSettori();

	SetStatoLinea(LINEA_IN_STOP);

	CongelaPKL();

	Cfg.bAttesaAnalisiVolumetrica=FALSE;

	/*
	* gestione program loop
	*/
	for(;;){
		/* 
		* resetto l'evento
		*/
		Evento.nSettore=-1;
		Evento.nEvento=-1;

		/*
		* Gestione Messaggi Ricevuti
		*/
		nAllMsg = 0;
		if((nCarIn = ProcessReceiveMsgNoWait(PROC_SETTORI, &nAllMsg, szInMsg))>=0){
			UnpackStructMsg(szInMsg, &InMsgStruct, szText, nCarIn);

			switch (InMsgStruct.srce){
				case PROC_VOLUM:
					switch (InMsgStruct.code){
						case VOLUM_END_ANALISI_ORDINE:
#ifdef TRACE
							trace_out_vstr_date(1, "VOLUM_END_ANALISI_ORDINE [%s]",szText);
#endif
							Cfg.bAttesaAnalisiVolumetrica=FALSE;
							/*
							* Cambio il flag ordine 
							*/
							CambiaFlagOrdine(szText, ORDINE_ELABORATO);
						break;
						case VOLUM_ERRORE_ANALISI_ORDINE:
#ifdef TRACE
							trace_out_vstr_date(1, "VOLUM_ERRORE_ANALISI_ORDINE [%s]",szText);
#endif
							Cfg.bAttesaAnalisiVolumetrica=FALSE;
							/*
							* Cambio il flag ordine
							*/
							CambiaFlagOrdine(szText, ORDINE_IN_ANALISI);
						break;
					}
					break;
				case PROC_MAIN:
					switch(InMsgStruct.code){
						case REFRESH_CONFIGURATION:
						break;
						/*
						* gestione start/stop/pausa linea/ripresa lavoro
						*/
						case START_LINEA:
						{
							int nIndex;

#ifdef TRACE
							trace_out_vstr_date(1, "START_LINEA");
#endif
							ReadUbicazioni(&Ubicazioni);
							SortUbicazioni(&Ubicazioni);

							/*
							* ripristino stato displays e luci
							*/
							for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
								UpdateDisplay(&(Cfg.Displays[nIndex]),TUTTO);
							}
							for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
								if(pSettori[nIndex].nStatoLampada==1){
									SetLampada(pSettori[nIndex].nIOS,pSettori[nIndex].nCPU,pSettori[nIndex].nModulo,pSettori[nIndex].nRiga,pSettori[nIndex].nColonna);
								}
							}
							ScongelaPKL();

							SetStatoLinea(LINEA_IN_START);
							/* 
							* resetto lo stato di attesa analisi volumetrica 
							*/
							Cfg.bAttesaAnalisiVolumetrica=FALSE;
						}
						break;
						case STOP_LINEA:
#ifdef TRACE
							trace_out_vstr_date(1, "STOP_LINEA");
#endif
							CongelaPKL();
							SetStatoLinea(LINEA_IN_STOP);
						break;
						case RESET_SETTORE:
						{
							int nIsola,nSettore;

							sscanf(szText,"%d,%d",&nIsola,&nSettore);
							/*
							* resetto solo il settore specificato
							*/
							ResetSettore(nIsola,nSettore);
						}
						break;
						case RESET_ALL:
							/*
							* resetto tutti i settori
							*/
							ResetLuci();
							ResetSettori();
						break;
						case SAVE_STATE:
							if(!SaveState(szText)){
								SendMessage(PROC_MAIN, PROC_SETTORI,  DISPLAY_MSG, "Errore in salvataggio stato settori\n");
							}
						break;

						case RESTORE_STATE:
							if(!RestoreState(szText)){
								SendMessage(PROC_MAIN, PROC_SETTORI,  DISPLAY_MSG, "Errore in caricamento stato settori\n");
							}
						break;
					}
				break;
				case PROC_IOS:
					switch(InMsgStruct.code){
						case IOS_PKL_BUTTON_PRESSED:
						{
							PUBICAZIONE pUbicazione;
							int nIOS,nCPU,nModule,nRowIndex,nColIndex;

							sscanf(szText,"%d,%d,%d,%d,%d",&nIOS,&nCPU,&nModule,&nRowIndex,&nColIndex);

							Evento.nEvento=IOS_PKL_BUTTON_PRESSED;
							Evento.nRiga=nRowIndex;
							Evento.nColonna=nColIndex;
							Evento.nModulo=nModule;
							Evento.nIOS=nIOS;
							Evento.nCPU=nCPU;
							/*
							* rm 04-04-2000 : Azzero il contatore guasti
							*/
							nIndex=0;
							while(nIndex<Cfg.nPKLNumber){
								if(Cfg.PKL[nIndex].nIOS==nIOS && Cfg.PKL[nIndex].nCPU==nCPU && Cfg.PKL[nIndex].nModulo==nModule){
									Cfg.PKL[nIndex].DamageCounter[nRowIndex][nColIndex]=0;
									break;
								}
								nIndex++;
							}

							if((pUbicazione=SearchUbicazione(&Ubicazioni,nIOS,nCPU,nModule,nRowIndex,nColIndex))!=NULL){
								Evento.pUbicazione=pUbicazione;
								Evento.nSettore=pUbicazione->nSettore;
								Evento.nIsola=pUbicazione->nIsola;
							}
#ifdef TRACE
							trace_out_vstr_date(1, "Ricevuto IOS_PKL_BUTTON_PRESSED [IOS:%d,CPU:%d,M:%2d,R:%2d,C:%2d]", 
								nIOS,nCPU,nModule,nRowIndex,nColIndex);
#endif
						}
						break;
						case IOS_PKL_LIGHT_DAMAGED:
						{
							PUBICAZIONE pUbicazione;
							PDISPLAYSTRUCT pDisplay;
							int nIOS,nCPU,nModule,nRowIndex,nColIndex;
							char szMsg[128];
							char szBuffer[128];

							sscanf(szText,"%d,%d,%d,%d,%d",&nIOS,&nCPU,&nModule,&nRowIndex,&nColIndex);


							if((pUbicazione=SearchUbicazione(&Ubicazioni,nIOS,nCPU,nModule,nRowIndex,nColIndex))!=NULL){
#ifdef TRACE
								trace_out_vstr_date(1, "IOS_PKL_LIGHT_DAMAGED S:%d I:%d IOS:%d CPU:%d M:%d R:%2d C:%2d",
									pUbicazione->nSettore,
									pUbicazione->nIsola,
									nIOS,nCPU,nModule,nRowIndex,nColIndex);
#endif
								/*
								* rm 04-04-2000 : gestione lampada rotta con contatore per
								*                 problemi IOS
								* Ricavo indice modulo PKL
								*/
								nIndex=0;
								while(nIndex<Cfg.nPKLNumber){
									if(Cfg.PKL[nIndex].nIOS==nIOS && Cfg.PKL[nIndex].nCPU==nCPU && Cfg.PKL[nIndex].nModulo==nModule){
										Cfg.PKL[nIndex].DamageCounter[nRowIndex][nColIndex]++;
										if(Cfg.PKL[nIndex].DamageCounter[nRowIndex][nColIndex]>Cfg.nMaxDamages){
											/*
											* Avverto l'operatore 
											*/
											sprintf(szMsg,"ATTENZIONE ! Lampada Rotta : %s [M.%2d,R.%2d,C.%2d S.%2d I.%d U.%s]\n",
												pUbicazione->szLocazione,
												nModule,nRowIndex,nColIndex,
												pUbicazione->nSettore,
												pUbicazione->nIsola,
												pUbicazione->szCodUbi);

											SendMessage(PROC_MAIN, PROC_SETTORI,  DISPLAY_MSG, szMsg);

											pDisplay=&(Cfg.Displays[pUbicazione->nDisplay-1]);

											strcpy(szBuffer,"LUCE");
											/*
											* Azzero puntino (buco del firmware della pkdis (Manfredi)
											*/
											DisplayWriteStringBCD( pDisplay->nIOS, pDisplay->nCPU, pDisplay->nModulo, 1, NORMAL, "    ",0,0);
											DisplayWriteString( pDisplay->nIOS, pDisplay->nCPU, pDisplay->nModulo, 0, NORMAL, szBuffer);
											sprintf(szBuffer,"%2d%2d",nRowIndex,nColIndex);
											DisplayWriteStringBCD( pDisplay->nIOS, pDisplay->nCPU, pDisplay->nModulo, 1, NORMAL, szBuffer,2,0);

											/*
											* 19-04-2001 st: modifica disperata per problema ai cablaggi
											* visualizzo il messaggio luce per 1 sec. e successivamente bleffo
											* mandandomi un messaggio di pulsante premuto
											* usleep(1000*1000);
											* sprintf(szBuffer,"%d,%d,%d,%d,%d",nIOS,nCPU,nModule,nRowIndex,nColIndex);
											* SendMessage(PROC_SETTORI, PROC_IOS,  IOS_PKL_BUTTON_PRESSED, szBuffer);
											*/

										} else {
											/*
											* provo a riaccendere la luce dopo 2 decimi di secondo
											*/
											usleep(1000*200);
											SetLampada(nIOS,nCPU,nModule,nRowIndex,nColIndex);
										}
									}
									nIndex++;
								}
							}
						}
						break;
					}
				break;
			}
		}
		if(GetStatoLinea()==LINEA_IN_START){
			/*
			* Gestione dell'evento per Settore
			*/
			if(Evento.nSettore!=-1){
				GestioneSettore(&Evento);
			}
			/*
			* gestione dei settori non interessati da eventi di tastiera
			*/
			Evento.nEvento=-1;
			for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
				Evento.nSettore=pSettori[nIndex].nSettore;
				Evento.nIsola=pSettori[nIndex].nIsola;
				GestioneSettore(&Evento);
			}
		}

		/*
		* ritardo in microsecondi 
		*/
		usleep(1000*Cfg.nDelay);
	}

	SafeIntFunc();
}	/* End of main() */


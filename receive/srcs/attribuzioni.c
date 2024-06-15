/*
* attribuzioni.c
* ------------------
* Ri-Attribuzioni 
*
* Progetto Mondadori - Gestione Ordini
*
* Data creazione 28/01/2002
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>

#include <fcntl.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef TRACE
	#include "trace.h"
#endif

#include <msq_lib.h>
#include <shared.h>
#include <libpq-fe.h>

#include <dbfun.h>
#include <proc_list.h>
#include <picking.h>

#include "receive.h"
#include "recstruct.h"
#include "recext.h"
#include "recfun.h"

extern char *optarg;
extern int optind;
extern char rcsid[];
extern char __version__[];
extern char __customer__[];

void CalcoloInstradamenti(void);

/*
* Main function
* inizializzazione dati
* lettura dei dati di configurazione
*/
int main(int argc,char **argv)
{
	
#ifdef TRACE
	char szBufTrace[80];
#endif

#ifdef TRACE
	/*
	* Apro il file trace 
	*/
	sprintf(szBufTrace,"%s%s",argv[0],TRACE_FILE_SUFFIX);
	open_trace(argv[0], szBufTrace, TRACE_FILE_MAXSIZE);
	trace_out_vstr_date(1,"Started");
#endif

	/*
	* lettura del file di configurazione
	*/
	ReadConfiguration();

	/* 
	* Connessione al database
	*/
	ConnectDataBase(Cfg.szPGHost, Cfg.szPGPort, Cfg.szPGDataBase);

	CalcoloInstradamenti();

	return 0;
}

/*
* SafeExit()
*
* safe exit from software interrupt 
*/
void SafeExit(void)
{
	DisconnectDataBase();

	if(pLinea){
		DetachShm((char *)pLinea);
	}
#ifdef TRACE
	trace_out_vstr_date(1,"Stopped");
#endif
	exit(0);
}


/*
* SafeIntFunc()
*
* safe exit from software interrupt 
*/
void SafeIntFunc()
{
#ifdef TRACE
	trace_out_vstr_date(1,"Got Signal ! ");
#endif
	bExitRequest=TRUE;
}

/*
* ReadConfiguration()
* lettura della configurazione dal file cni.cfg
*/
void ReadConfiguration(void)
{
	char szParagraph[128];
	char szCurrentDirectory[128];

	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	
	strcpy(Cfg.szCniCfg,szCurrentDirectory);
	strcat(Cfg.szCniCfg,"/cni.cfg");
	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");

	GetFileString(szParagraph,"PathData",          "../data",        Cfg.szPathData,          80,Cfg.szCniCfg,NULL);

	Cfg.nDebugVersion    = GetFileInt(szParagraph,"DebugVersion",      0, Cfg.szCniCfg,NULL);

	/*
	* lettura configurazione DataBase System
	*/
	strcpy(szParagraph,"DataBase Settings");
	GetFileString(szParagraph,"PGHost",    "localhost", Cfg.szPGHost,     80,Cfg.szCniCfg,NULL); 
	GetFileString(szParagraph,"PGPort",    "5432",      Cfg.szPGPort,     80,Cfg.szCniCfg,NULL); 
	GetFileString(szParagraph,"PGDataBase","momo",      Cfg.szPGDataBase, 80,Cfg.szCniCfg,NULL); 

#ifdef TRACE
	trace_out_vstr_date(1,"Read Configuration");
	trace_out_vstr(1,"-------------------------------------------");
	trace_out_vstr(1,"PathData                : %s",Cfg.szPathData);
	trace_out_vstr(1,"DebugVersion            : %d",Cfg.nDebugVersion);
	trace_out_vstr(1,"PGHost                  : %s",Cfg.szPGHost);
	trace_out_vstr(1,"PGPort                  : %s",Cfg.szPGPort);
	trace_out_vstr(1,"PGDataBase              : %s",Cfg.szPGDataBase);
	trace_out_vstr(1,"-------------------------------------------");
#endif
}

/*
* ReadProcInfo()
* Lettura dei parametri relativi ai processi 
*/
void ReadProcInfo(char *szFileConfig)
{
	char szProcIndex[128];
	char szProcBuffer[128];
	int nIndex;

	for (nIndex=0; nIndex<NUM_PROC; nIndex++) {
		sprintf(szProcIndex,"Proc_%02d",nIndex);
		GetFileString("procinfo",szProcIndex, "", szProcBuffer, 80,szFileConfig,NULL);
		sscanf(szProcBuffer, "%[^,],%d,%d,%d",
				ProcList[nIndex].szProcName,
				&ProcList[nIndex].nQKey,
				&ProcList[nIndex].nPriority,
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
}





void CalcoloInstradamenti(void)
{
	int nIndex=0;
	int nTuples=0;
	int nPeso=0;
	int nColli=0;
	PGresult *PGRes;
	PGresult *PGResOrdini;
	PGresult *PGResUpdate;
	char szOrdProg[128];
	DATI_DISTRIBUZIONE DatiDistribuzione;

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"begin work;");
	DBclear(PGRes);

	/*
	* Ricalcolo numero righe, numero copie e peso dell'ordine ricevuto
	* assegno codice vettore e dati accessori
	*/
	PGResOrdini=PGExecSQL(Cfg.nDebugVersion>1,"select ordprog,ropspre,ronmcll from ric_ord where =' ';");
	nTuples=PQntuples(PGResOrdini);
	if(nTuples){
		fprintf(stdout,"\n%d ordini da attribuire\n",nTuples);
#ifdef TRACE
		trace_out_vstr_date(1,"%d ordini da attribuire",nTuples);
#endif

		for(nIndex=0;nIndex<nTuples;nIndex++){
			strcpy(szOrdProg,PQgetvalue(PGResOrdini,nIndex,0));  /* Ordine */
			nPeso=atoi(PQgetvalue(PGResOrdini,nIndex,1));   /* Peso */
			nColli=atoi(PQgetvalue(PGResOrdini,nIndex,2));  /* Colli */
			/* default a 10 */
			if(nColli==0){
				nColli=10;
			}

			if(RicavaDistribuzione(&DatiDistribuzione,szOrdProg,nPeso,nColli)){
				/*
				* Eseguo update ordine
				*/
				PGResUpdate=PGExecSQL(Cfg.nDebugVersion>1,"update ric_ord set roswcol='%s', rotpspe='%s', rocdlin='%s', rocdsca='%s', rocdssc='%s', roidvet='%s', rocdve1='%s', rocdve2='%s' where ordprog='%s';", 
					DatiDistribuzione.szSWCOL,
					DatiDistribuzione.szTPSPE,
					DatiDistribuzione.szCDLIN,
					DatiDistribuzione.szCDSCA,
					DatiDistribuzione.szCDSSC,
					DatiDistribuzione.szIDVET,
					DatiDistribuzione.szCDVE1,
					DatiDistribuzione.szCDVE2,
					szOrdProg);
				if (PQresultStatus(PGResUpdate) != PGRES_COMMAND_OK && atoi(PQcmdTuples(PGResUpdate))==0) {
					fprintf(stdout,"\nErrore in update ordine [%s]\n",szOrdProg);
#ifdef TRACE
					trace_out_vstr_date(1,"Errore in update ordine [%s]",szOrdProg);
#endif
				}
				DBclear(PGResUpdate);
				if(nIndex%10==0){
					fprintf(stdout,".");
					fflush(stdout);
				}
			} else {
				fprintf(stdout,"\nErrore in attribuzione ordine [%s]\n",szOrdProg);
#ifdef TRACE
				trace_out_vstr_date(1,"Errore in attribuzione ordine [%s]",szOrdProg);
#endif
			}

		}
	}

	DBclear(PGResOrdini);
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"commit work;");

	fprintf(stdout,"\nCalcolo instradamenti terminato\n");
#ifdef TRACE
	trace_out_vstr_date(1,"Calcolo instradamenti terminato");
#endif

	DBclear(PGRes);
}

/*
* RicavaDistribuzione()
* ---------------------
* Ricavo i seguenti dati:
*  -  Peso Lordo Spedizione
*  -  Switch Tipo Imballo
*  -  Tipo Spedizione
*  -  Linea
*  -  Scalo
*  -  Sottoscalo
* ritorna : 
*    TRUE  : tutto OK
*    FALSE : qualche problema
*/
BOOL RicavaDistribuzione(PDATI_DISTRIBUZIONE pDati,char *szOrdProg,int nPeso,int nColli)
{
	PGresult *PGRes;
	BOOL bFoundFC=FALSE;
	BOOL bFoundGC=FALSE;
	BOOL bFoundLS=FALSE;
	BOOL bFoundFP=FALSE;
	BOOL bRetValue=TRUE;
	char szCDRID[128];
	char szCDSOC[128];
	char szCDMAG[128];
	char szCPCLI[128];
	char szCDFPG[128];
	char szCPRAG[128];
	char szFPRAG[128];
	char szSWCOL[128];

	strcpy(pDati->szTPSPE,"99");
	strcpy(pDati->szCDLIN,"99");
	strcpy(pDati->szCDSCA,"99");
	strcpy(pDati->szCDSSC,"99");
	strcpy(pDati->szIDVET,"99");
	strcpy(pDati->szCDVE1,"999999");
	strcpy(pDati->szCDVE2,"999999");

	/* ricavare peso e cap spedizione */
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select rocdrid,rocpcli,rocdfpg,roswcol from ric_ord where ordprog='%s';",szOrdProg);
	if(PQntuples(PGRes)){
		strcpy(szCDRID,PQgetvalue(PGRes,0,0));
		strcpy(szCPCLI,PQgetvalue(PGRes,0,1));
		strcpy(szCDFPG,PQgetvalue(PGRes,0,2));

		strcpy(szCDSOC,SubStr(szOrdProg,0,2));
		strcpy(szCDMAG,SubStr(szOrdProg,2,7));
		
		strcpy(szCDFPG,PQgetvalue(PGRes,0,3));
	}
	DBclear(PGRes);

	/* inizializzo il tipo scatole a 1 (colli) */
	strcpy(pDati->szSWCOL,"1");
	//strcpy(pDati->szSWCOL,szSWCOL);

	/* cerco in tabella forzatura clienti */
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elcswcol,elctpspe, elccdlin, elccdsca, elccdssc from ttfc where elccdsoc='%s' and elccdmag='%s' and elccdrid='%s' and elctptpd='G' order by elccdsoc,elccdmag,elccdrid;",
		szCDSOC, szCDMAG, szCDRID);
	if(PQntuples(PGRes)){
		bFoundFC=TRUE;
		
		strcpy(pDati->szSWCOL,PQgetvalue(PGRes,0,0));
		strcpy(pDati->szTPSPE,PQgetvalue(PGRes,0,1));
		strcpy(pDati->szCDLIN,PQgetvalue(PGRes,0,2));
		strcpy(pDati->szCDSCA,PQgetvalue(PGRes,0,3));
		strcpy(pDati->szCDSSC,PQgetvalue(PGRes,0,4));

	}
	DBclear(PGRes);

	if(!bFoundFC){

		/* CAP */
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elccprag from ttgc where elccdsoc='%s' and elccdmag='%s' and elccdcap='%s' order by elccdsoc,elccdmag,elccdcap;",
			szCDSOC, szCDMAG, szCPCLI);
		if(PQntuples(PGRes)){
			/* Ricavo ELCCPRAG */
			bFoundGC=TRUE;
			strcpy(szCPRAG,PQgetvalue(PGRes,0,0));
			DBclear(PGRes);
		} else {
			DBclear(PGRes);
			/*
			* Metto a zero gli ultimi 2 caratteri del CAP
			*/
			strcpy(szCPCLI+(strlen(szCPCLI)-2),"00");
			PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elccprag from ttgc where elccdsoc='%s' and elccdmag='%s' and elccdcap='%s' order by elccdsoc,elccdmag,elccdcap;",
				szCDSOC, szCDMAG, szCPCLI);
			if(PQntuples(PGRes)){
				/* Ricavo ELCCPRAG */
				bFoundGC=TRUE;
				strcpy(szCPRAG,PQgetvalue(PGRes,0,0));
			}
			DBclear(PGRes);
		}

		if(!bFoundGC){
			bRetValue=FALSE;
#ifdef TRACE
			trace_out_vstr_date(1,"Ordine [%s] chiave ttgc non trovata [CPCLI:%s]",szOrdProg,szCPCLI);
#endif
		}

		if(bRetValue){
			/* Forma di pagamento */
			PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elcfprag from ttfp where elccdsoc='%s' and elccdmag='%s' and elccdfpg='%s' order by elccdsoc,elccdmag,elccdfpg;",
				szCDSOC, szCDMAG, szCDFPG);
			if(PQntuples(PGRes)){
				/* Ricavo ELCFPRAG */
				bFoundFP=TRUE;
				strcpy(szFPRAG,PQgetvalue(PGRes,0,0));
				DBclear(PGRes);
			} else {
				DBclear(PGRes);
				/*
				* Provo con '***'
				*/
				strcpy(szCDFPG,"***");
				PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elcfprag from ttfp where elccdsoc='%s' and elccdmag='%s' and elccdfpg='%s' order by elccdsoc,elccdmag,elccdfpg;",
					szCDSOC, szCDMAG, szCDFPG);
				if(PQntuples(PGRes)){
					/* Ricavo ELCFPRAG */
					bFoundFP=TRUE;
					strcpy(szFPRAG,PQgetvalue(PGRes,0,0));
				}
				DBclear(PGRes);
			}
		}


		if(!bFoundFP){
#ifdef TRACE
			trace_out_vstr_date(1,"Ordine [%s] chiave ttfp non trovata [CDFPG:%s]",szOrdProg,szCDFPG);
#endif
			bRetValue=FALSE;
		}

		if(bRetValue){
			PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elctpspe,elccdlin,elccdsca,elccdssc from ttls where elccdsoc='%s' and elccdmag='%s' and elccprag='%s' and elcfprag='%s' and elctptpd='G' and elcmaxps > %d and elcmaxcl > %d order by elccdsoc,elccdmag,elccprag,elcfprag,elcmaxps,elcmaxcl;",
					szCDSOC, szCDMAG, szCPRAG,szFPRAG,nPeso,nColli);
			if(PQntuples(PGRes)){
				bFoundLS=TRUE;
				strcpy(pDati->szTPSPE,PQgetvalue(PGRes,0,0));
				strcpy(pDati->szCDLIN,PQgetvalue(PGRes,0,1));
				strcpy(pDati->szCDSCA,PQgetvalue(PGRes,0,2));
				strcpy(pDati->szCDSSC,PQgetvalue(PGRes,0,3));
			}
			DBclear(PGRes);
		}

		if(!bFoundLS){
#ifdef TRACE
			trace_out_vstr_date(1,"Ordine [%s] chiave ttls non trovata [CPRAG:%s FPRAG:%s]",szOrdProg,szCPRAG,szFPRAG);
#endif
			bRetValue=FALSE;
		}
	}

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elcidvet,elccdve1,elccdve2 from ttlv where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s' and elccdlin='%s' and elccdsca='%s' and elccdssc='%s';",
		szCDSOC,szCDMAG,pDati->szTPSPE,pDati->szCDLIN,pDati->szCDSCA,pDati->szCDSSC);
	if(PQntuples(PGRes)){
		strcpy(pDati->szIDVET,PQgetvalue(PGRes,0,0));
		strcpy(pDati->szCDVE1,PQgetvalue(PGRes,0,1));
		strcpy(pDati->szCDVE2,PQgetvalue(PGRes,0,2));
	} else {
#ifdef TRACE
		trace_out_vstr_date(1,"Ordine [%s] chiave ttlv non trovata [TPSPE:%s CDLIN:%s CDSCA:%s CDSSC:%s] provo con **",szOrdProg, pDati->szTPSPE,pDati->szCDLIN,pDati->szCDSCA,pDati->szCDSSC);
#endif
		DBclear(PGRes);
		/*
		* rm 8-01-2008 - Provo con '**'
		*/
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select elcidvet,elccdve1,elccdve2 from ttlv where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s' and elccdlin='%s' and elccdsca='%s' and elccdssc='%s';",
			szCDSOC,szCDMAG,pDati->szTPSPE,pDati->szCDLIN,"**","**");
		if(PQntuples(PGRes)){
			strcpy(pDati->szIDVET,PQgetvalue(PGRes,0,0));
			strcpy(pDati->szCDVE1,PQgetvalue(PGRes,0,1));
			strcpy(pDati->szCDVE2,PQgetvalue(PGRes,0,2));
		} else {
			bRetValue=FALSE;
#ifdef TRACE
			trace_out_vstr_date(1,"Ordine [%s] chiave ttlv non trovata [TPSPE:%s CDLIN:%s CDSCA:%s CDSSC:%s]",szOrdProg, pDati->szTPSPE,pDati->szCDLIN,"**","**");
#endif
		}
	}
	DBclear(PGRes);

	return(bRetValue);
}
